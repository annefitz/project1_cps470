#pragma once

#include "common.h"
#include "urlparser.h"
#include "winsock.h"

using namespace std::chrono;

// this class is passed to all threads, acts as shared memory
class Parameters {
public:
	HANDLE print_mutex;
	CRITICAL_SECTION q_mutex;
	HANDLE unique_mutex;
	HANDLE finished;
	HANDLE eventQuit;
	queue<string> *inq;
	unordered_set<string> HOST_container;
	unordered_set<string> IP_container;
	int num_tasks;
	_Interlocked_operand_ unsigned num_HOST_unique;
	_Interlocked_operand_ unsigned num_DNS;
	_Interlocked_operand_ unsigned num_IP_unique;
	_Interlocked_operand_ unsigned num_robots;
	_Interlocked_operand_ unsigned num_URLs;
	_Interlocked_operand_ unsigned num_crawled;
	_Interlocked_operand_ long total_links_found;

	_Interlocked_operand_ long time_DNS;
	_Interlocked_operand_ long time_robots;
	_Interlocked_operand_ long time_crawled;
	_Interlocked_operand_ long time_links;

	_Interlocked_operand_ long size_crawl;

	_Interlocked_operand_ unsigned num_200;
	_Interlocked_operand_ unsigned num_300;
	_Interlocked_operand_ unsigned num_400;
	_Interlocked_operand_ unsigned num_500;
	_Interlocked_operand_ unsigned num_other;

	Parameters() {
		//q_mutex = &q_m;
		InitializeCriticalSection(&q_mutex);
	}
};

static UINT thread_fun(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);
	//InitializeCriticalSection(p->q_mutex);
	//ResetEvent(p->finished);

	// wait for mutex, then print and sleep inside the critical section
	WaitForSingleObject(p->print_mutex, INFINITE);				// lock mutex
	printf("Thread %d started\n", GetCurrentThreadId());		// always print inside critical section to avoid screen garbage
	ReleaseMutex(p->print_mutex);								// release critical section

	//HANDLE	arr[] = { p->eventQuit, p->inq };

	Winsock::initialize();	// initialize 
	Winsock ws;

	while (true)
	{
		/*if (WaitForMultipleObjects(2, arr, false, INFINITE) == WAIT_OBJECT_0) // the eventQuit has been signaled
		{
			DWORD err = GetLastError();
			cout << "ERROR CODE: " << err << endl;
			break;
		}*/
		//else // semaQ is signaled. decreased the semaphore count by 1
		//{
		//printf("Thread %d: num_tasks_left = %d\n", GetCurrentThreadId(), p->num_tasks);

		// obtain ownership of the mutex
		// WaitForSingleObject(p->q_mutex, INFINITE);
		// ------------- entered the critical section ---------------
		if (p->num_tasks == 0 || p->inq->empty()) {
			cout << "CHECK\n";
			//ReleaseMutex(p->print_mutex);
			break;
		}
		else // semaQ is signaled. decreased the semaphore count by 1
		{

			// obtain ownership of the mutex
			//WaitForSingleObject(p->q_mutex, INFINITE);
			EnterCriticalSection(&(p->q_mutex));
				// ------------- entered the critical section ---------------
				if (p->num_tasks == 0 || p->inq->empty()) {
					cout << "CHECK\n";
					SetEvent(p->eventQuit);
					LeaveCriticalSection(&(p->q_mutex));
					//ReleaseMutex(p->q_mutex);
					break;
				}
				printf("Thread %d: num_tasks_left = %d\n", GetCurrentThreadId(), p->num_tasks);
				string url = p->inq->front(); // get the item from the inputQ
				WaitForSingleObject(p->print_mutex, INFINITE);
				cout << "URL: " << url << "\n";
				ReleaseMutex(p->print_mutex);
				p->inq->pop();
				InterlockedIncrement(&(p->num_URLs));
				p->num_tasks--;
			// return mutex
			//ReleaseMutex(p->q_mutex);
			LeaveCriticalSection(&(p->q_mutex));
			// ------------- left the critical section ------------------		

			//cout << endl << "HOST: " << host << " PATH: " << path << " QUERY: " << query << endl;

			//cout << "Path: " << path << " Host : " << host << " Port: " << port << "\n";	

		// parse url
		URLParser parser(url);
		string host = parser.getHost();
		string path = parser.getPath();
		string query = parser.getQuery();
		short port = parser.getPort();

		//cout << endl << "HOST: " << host << " PATH: " << path << " QUERY: " << query << endl;

		//cout << "Path: " << path << " Host : " << host << " Port: " << port << "\n";
		WaitForSingleObject(p->print_mutex, INFINITE);
		cout << "\tParsing URL... host " << host << ", port " << port << "\n";
		ReleaseMutex(p->print_mutex);
		// delay here: contact a peer, send a request, and receive/parse the response 

		// check for host uniqueness
		WaitForSingleObject(p->unique_mutex, INFINITE);
		if (p->HOST_container.find(host) == p->HOST_container.end()) {
			// the HOST is unique, so add it to the container
			p->HOST_container.insert(host);
			cout << "\tChecking host uniqueness... passed\n";
			ReleaseMutex(p->unique_mutex);
		}
		else {
			cout << "\tChecking host uniqueness... failed\n";
			ReleaseMutex(p->unique_mutex);
			continue;
		}

		// starting connection
		// will first find IP then connect via IP
		WaitForSingleObject(p->print_mutex, INFINITE);
		cout << "\tDoing DNS... ";
		ReleaseMutex(p->print_mutex);
		// starting timer
		auto stop = high_resolution_clock::now();  // instantiate vars
		auto start = high_resolution_clock::now(); // instantiate vars
		auto duration = duration_cast<milliseconds>(stop - start);

		// get IP from hostname, check for valid host/IP
		string IP = ws.getIPfromhost(host, p->print_mutex);
		if (IP.empty()) {
			WaitForSingleObject(p->print_mutex, INFINITE);
			cout << "Invalid string: neither FQDN, nor IP address\n";
			ReleaseMutex(p->print_mutex);
			ws.closeSocket();
			continue;
		}

		ws.createTCPSocket();
		if (ws.connectToServerIP(IP, port) == 1) {
			WaitForSingleObject(p->print_mutex, INFINITE);
			cout << "IP: " << IP << "failed\n";
			ReleaseMutex(p->print_mutex);
			ws.closeSocket();
			continue;
		}
		else {
			stop = high_resolution_clock::now();
			duration = duration_cast<milliseconds>(stop - start);
			InterlockedIncrement(&(p->num_DNS));
			InterlockedAdd(&(p->time_DNS), duration.count());
			WaitForSingleObject(p->print_mutex, INFINITE);
			cout << "done in " << duration.count() << " ms, found " << IP << "\n";
		}

		WaitForSingleObject(p->unique_mutex, INFINITE);
		if (p->IP_container.find(IP) == p->IP_container.end()) {
			// the IP is unique, so add it to the container
			p->IP_container.insert(IP);
			cout << "\tChecking IP uniqueness... passed\n";
			ReleaseMutex(p->unique_mutex);
		}
		else {
			cout << "\tChecking IP uniqueness... failed\n";
			ReleaseMutex(p->unique_mutex);

			ws.closeSocket();
			continue;
		}

		// construct a GET or HEAD request (in a string), send request
		start = high_resolution_clock::now();
		WaitForSingleObject(p->print_mutex, INFINITE);
		cout << "\tConnecting on robots... ";
		ReleaseMutex(p->print_mutex);

		if (ws.sendHEADRequest(host)) {
			stop = high_resolution_clock::now();
			duration = duration_cast<milliseconds>(stop - start);
			InterlockedIncrement(&(p->num_robots));
			InterlockedAdd(&(p->time_robots), duration.count());
			WaitForSingleObject(p->print_mutex, INFINITE);
			cout << "done in " << duration.count() << " ms\n";
			ReleaseMutex(p->print_mutex);
		}
		else {
			WaitForSingleObject(p->print_mutex, INFINITE);
			cout << "failed\n";
			ReleaseMutex(p->print_mutex);
			ws.closeSocket();
			continue;
		}


		WaitForSingleObject(p->print_mutex, INFINITE);
		cout << "\tLoading... ";
		ReleaseMutex(p->print_mutex);
		start = high_resolution_clock::now(); // start timer for loading HEAD reply

		// receive HEAD reply
		string HEADreply = "";
		if (ws.receive(HEADreply)) {
			//std::cout << "reply not success\n";
			//cout << HEADreply;
			stop = high_resolution_clock::now();
			duration = duration_cast<milliseconds>(stop - start);
			InterlockedIncrement(&(p->num_crawled));
			InterlockedAdd(&(p->time_crawled), duration.count());
			WaitForSingleObject(p->print_mutex, INFINITE);
			cout << "done in " << duration.count() << " ms with " << HEADreply.size() << " bytes\n";
			InterlockedAdd(&(p->size_crawl), HEADreply.size());
			ReleaseMutex(p->print_mutex);
		}
		else {
			WaitForSingleObject(p->print_mutex, INFINITE);
			cout << "IP: " << IP << " failed\n";
			ReleaseMutex(p->print_mutex);
			ws.closeSocket();
			continue;
		}

		// find the status code in the reply
		int status_end_idx = HEADreply.find("\n");
		string status_code_string = HEADreply.substr(9, status_end_idx);
		int status_code = stoi(status_code_string.substr(0, 3));

		WaitForSingleObject(p->print_mutex, INFINITE);
		cout << "\tVerifying header... status code " << status_code << "\n";
		ReleaseMutex(p->print_mutex);

		// if the status code is 400 or higher, 
		if (status_code >= 400) {
			ws.closeSocket();
			ws.createTCPSocket();

			if (ws.connectToServerIP(IP, port) != 0) {
				//printf("Connection error: %d\n", WSAGetLastError());
			}
			start = high_resolution_clock::now();
			WaitForSingleObject(p->print_mutex, INFINITE);
			cout << "\tConnecting on page... ";
			ReleaseMutex(p->print_mutex);
			if (ws.sendGETRequest(host, path, query)) {
				//std::cout << "request success\n";
				stop = high_resolution_clock::now();
				duration = duration_cast<milliseconds>(stop - start);
				WaitForSingleObject(p->print_mutex, INFINITE);
				cout << "done in " << duration.count() << " ms\n";
				ReleaseMutex(p->print_mutex);
			}
			else {
				WaitForSingleObject(p->print_mutex, INFINITE);
				cout << "failed\n";
				ReleaseMutex(p->print_mutex);
				ws.closeSocket();
				continue;
			}

			// receive reply
			start = high_resolution_clock::now();
			WaitForSingleObject(p->print_mutex, INFINITE);
			cout << "\tLoading... ";
			ReleaseMutex(p->print_mutex);
			string GETreply = "";
			if (ws.receive(GETreply)) {
				stop = high_resolution_clock::now();
				duration = duration_cast<milliseconds>(stop - start);
				WaitForSingleObject(p->print_mutex, INFINITE);
				cout << "done in " << duration.count() << " ms with " << GETreply.size() << " bytes\n";
				ReleaseMutex(p->print_mutex);

				// find the status code in the reply
				//cout << GETreply;
				status_end_idx = GETreply.find("\n");
				status_code_string = GETreply.substr(9, status_end_idx);
				if (!status_code_string.empty()) {
					switch (stoi(status_code_string.substr(0, 1))) {
						case 2:
							InterlockedIncrement(&(p->num_200));
							break;
						case 3:
							InterlockedIncrement(&(p->num_300));
							break;
						case 4:
							InterlockedIncrement(&(p->num_400));
							break;
						case 5:
							InterlockedIncrement(&(p->num_500));
							break;
						default:
							InterlockedIncrement(&(p->num_other));
							break;
					}
					status_code = stoi(status_code_string.substr(0, 3));
				}

				WaitForSingleObject(p->print_mutex, INFINITE);
				cout << "\tVerifying header... status code " << status_code << "\n";
				ReleaseMutex(p->print_mutex);

				if (status_code == 200) {
					start = high_resolution_clock::now();
					WaitForSingleObject(p->print_mutex, INFINITE);
					cout << "\tParsing page... ";
					ReleaseMutex(p->print_mutex);
					int count = 0;
					status_end_idx = GETreply.find("http");
					status_code_string = GETreply.substr(status_end_idx);
					while (status_end_idx != NULL) {
						count++;
						status_end_idx = status_code_string.find("http") + 1;
						status_code_string = status_code_string.substr(status_end_idx);
					}
					stop = high_resolution_clock::now();
					duration = duration_cast<milliseconds>(stop - start);
					InterlockedAdd(&(p->total_links_found), count);
					InterlockedAdd(&(p->time_crawled), duration.count());
					WaitForSingleObject(p->print_mutex, INFINITE);
					cout << "done in " << duration.count() << " ms with " << count << " links\n";
					ReleaseMutex(p->print_mutex);
				}

			}
			else {
				WaitForSingleObject(p->print_mutex, INFINITE);
				cout << "failed\n";
				ReleaseMutex(p->print_mutex);
				ws.closeSocket();
				continue;
			}
		} // else further contact denied

		ws.closeSocket();

		/*WaitForSingleObject(p->q_mutex, INFINITE);
		if (p->num_tasks == 0) {
			SetEvent(p->eventQuit);
			ReleaseMutex(p->q_mutex);
		}*/

			ws.closeSocket();

			//WaitForSingleObject(p->q_mutex, INFINITE);
			EnterCriticalSection(&(p->q_mutex));
			if (p->num_tasks == 0) {
				SetEvent(p->eventQuit);
				//ReleaseMutex(p->q_mutex);
				LeaveCriticalSection(&(p->q_mutex));
				break;
			}
			LeaveCriticalSection(&(p->q_mutex));

		// obtain ownership of the mutex
		//WaitForSingleObject(p->q_mutex, INFINITE);
		// ------------- entered the critical section ------------------

		// write results into outputQ
		//p->outq->push(GETreply);

		// p->active_threads --;
		//p->num_tasks--;

		//cout << "TEST"; getchar();
		//ReleaseMutex(p->q_mutex);  // release the ownership of the mutex object to other threads
		//} ------------- left the critical section ------------------

			//cout << "TEST"; getchar();
			//ReleaseMutex(p->q_mutex);  // release the ownership of the mutex object to other threads
			} // ------------- left the critical section ------------------
		
	} // end of while loop for this thread

	Winsock::cleanUp();

	// signal that this thread is exiting
	ReleaseSemaphore(p->finished, 1, NULL);

	return 0;
}

static bool timeout(int time) {
	Sleep(time);
	return true;
}
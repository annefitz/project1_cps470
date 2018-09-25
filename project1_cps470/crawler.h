#pragma once

#include "common.h"
#include "urlparser.h"
#include "winsock.h"

using namespace std::chrono;

// this class is passed to all threads, acts as shared memory
class Parameters {
public:
	CRITICAL_SECTION print_mutex;
	CRITICAL_SECTION q_mutex;
	CRITICAL_SECTION unique_mutex;
	HANDLE finished;
	HANDLE eventQuit;
	HANDLE q_empty;
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

	_Interlocked_operand_ long size_crawl;

	_Interlocked_operand_ unsigned num_200;
	_Interlocked_operand_ unsigned num_300;
	_Interlocked_operand_ unsigned num_400;
	_Interlocked_operand_ unsigned num_500;
	_Interlocked_operand_ unsigned num_other;

	Parameters() {
		//q_mutex = &q_m;
		InitializeCriticalSection(&q_mutex);
		InitializeCriticalSection(&print_mutex);
		InitializeCriticalSection(&unique_mutex);
	}
};

static UINT thread_fun(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);
	//InitializeCriticalSection(p->q_mutex);
	//ResetEvent(p->finished);

	// wait for mutex, then print and sleep inside the critical section
	EnterCriticalSection(&(p->print_mutex));						// lock mutex
	printf("Thread %d started\n", GetCurrentThreadId());		// always print inside critical section to avoid screen garbage
	LeaveCriticalSection(&(p->print_mutex));									// release critical section

	HANDLE	arr[] = { p->eventQuit, p->inq };

	Winsock::initialize();	// initialize 
	Winsock ws;

	// url parsing vars
	string url;
	URLParser parser(url);
	string host;
	string path;
	string query;
	short port;

	string IP;
	string GETreply = "";
	string HEADreply = "";

	int status_end_idx;
	string status_code_string;
	//int status_code;

	while (true)
	{
		if (WaitForMultipleObjects(2, arr, false, INFINITE) == WAIT_OBJECT_0) // the eventQuit has been signaled
		{
			DWORD err = GetLastError();
			cout << "ERROR CODE: " << err << endl;
			break;
		}
		else // semaQ is signaled. decreased the semaphore count by 1
		{
			// obtain ownership of the mutex
			//WaitForSingleObject(p->q_mutex, INFINITE);
			EnterCriticalSection(&(p->q_mutex));
				// ------------- entered the critical section ---------------
				if (p->num_tasks == 0 || p->inq->empty()) {
					SetEvent(p->eventQuit);
					LeaveCriticalSection(&(p->q_mutex));
					//ReleaseMutex(p->q_mutex);
					break;
				}

				url = p->inq->front(); // get the item from the inputQ
				p->inq->pop();
				p->num_tasks--;

				EnterCriticalSection(&(p->print_mutex));
					printf("Thread %d: num_tasks_left = %d\n", GetCurrentThreadId(), p->num_tasks);
			LeaveCriticalSection(&(p->q_mutex));
					cout << "URL: " << url << "\n";
				LeaveCriticalSection(&(p->print_mutex));
				InterlockedIncrement(&(p->num_URLs));
			// return mutex
			//ReleaseMutex(p->q_mutex);
			
			// ------------- left the critical section ------------------		

			//cout << endl << "HOST: " << host << " PATH: " << path << " QUERY: " << query << endl;

			//cout << "Path: " << path << " Host : " << host << " Port: " << port << "\n";	

		// parse url
		URLParser parser(url);
		host = parser.getHost();
		path = parser.getPath();
		query = parser.getQuery();
		port = parser.getPort();

		//cout << endl << "HOST: " << host << " PATH: " << path << " QUERY: " << query << endl;

		EnterCriticalSection(&(p->print_mutex));
		cout << "\tParsing URL... host " << host << ", port " << port << "\n";
		LeaveCriticalSection(&(p->print_mutex));
		
		// delay here: contact a peer, send a request, and receive/parse the response 

		// check for host uniqueness
		EnterCriticalSection(&(p->unique_mutex));
		if (p->HOST_container.find(host) == p->HOST_container.end()) {
			// the HOST is unique, so add it to the container
			p->HOST_container.insert(host);
			cout << "\tChecking host uniqueness... passed\n";
			LeaveCriticalSection(&(p->unique_mutex));
		}
		else {
			cout << "\tChecking host uniqueness... failed\n";
			LeaveCriticalSection(&(p->unique_mutex));
			continue;
		}

		// starting connection
		// will first find IP then connect via IP
		//EnterCriticalSection(&(p->print_mutex));
		//cout << "\tDoing DNS... ";
		//LeaveCriticalSection(&(p->print_mutex));
		// starting timer
		auto stop = high_resolution_clock::now();  // instantiate vars
		auto start = high_resolution_clock::now(); // instantiate vars
		auto duration = duration_cast<milliseconds>(stop - start);

		// get IP from hostname, check for valid host/IP
		IP = ws.getIPfromhost(host);
		if (IP.empty()) {
			EnterCriticalSection(&(p->print_mutex));
			cout << "Invalid string: neither FQDN, nor IP address\n";
			LeaveCriticalSection(&(p->print_mutex));
			continue;
		}

		ws.createTCPSocket();
		if (ws.connectToServerIP(IP, port) == 1) {
			EnterCriticalSection(&(p->print_mutex));
			cout << "\tDoing DNS... " << "IP: " << IP << "failed\n";
			LeaveCriticalSection(&(p->print_mutex));
			ws.closeSocket();
			continue;
		}
		else {
			stop = high_resolution_clock::now();
			duration = duration_cast<milliseconds>(stop - start);
			InterlockedIncrement(&(p->num_DNS));
			EnterCriticalSection(&(p->print_mutex));
			cout << "\tDoing DNS... " << "done in " << duration.count() << " ms, found " << IP << "\n";
			LeaveCriticalSection(&(p->print_mutex));
		}

		EnterCriticalSection(&(p->unique_mutex));
		if (p->IP_container.find(IP) == p->IP_container.end()) {
			// the IP is unique, so add it to the container
			p->IP_container.insert(IP);
			cout << "\tChecking IP uniqueness... passed\n";
			LeaveCriticalSection(&(p->unique_mutex));
		}
		else {
			cout << "\tChecking IP uniqueness... failed\n";
			LeaveCriticalSection(&(p->unique_mutex));
			ws.closeSocket();
			continue;
		}

		// construct a GET or HEAD request (in a string), send request
		start = high_resolution_clock::now();
		EnterCriticalSection(&(p->print_mutex));
		cout << "\tConnecting on robots... ";
		LeaveCriticalSection(&(p->print_mutex));

		if (ws.sendHEADRequest(host)) {
			stop = high_resolution_clock::now();
			duration = duration_cast<milliseconds>(stop - start);
			InterlockedIncrement(&(p->num_robots));
			EnterCriticalSection(&(p->print_mutex));
			cout << "done in " << duration.count() << " ms\n";
			LeaveCriticalSection(&(p->print_mutex));
		}
		else {
			EnterCriticalSection(&(p->print_mutex));
			cout << "failed\n";
			LeaveCriticalSection(&(p->print_mutex));
			ws.closeSocket();
			continue;
		}


		//EnterCriticalSection(&(p->print_mutex));
		//cout << "\tLoading... ";
		//LeaveCriticalSection(&(p->print_mutex));
		start = high_resolution_clock::now(); // start timer for loading HEAD reply

		// receive HEAD reply
		if (ws.receive(HEADreply)) {
			//std::cout << "reply not success\n";
			//cout << HEADreply;
			stop = high_resolution_clock::now();
			duration = duration_cast<milliseconds>(stop - start);

			EnterCriticalSection(&(p->print_mutex));
			cout << "\tLoading... " << "done in " << duration.count() << " ms with " << HEADreply.size() << " bytes\n";
			LeaveCriticalSection(&(p->print_mutex));
		}
		else {
			EnterCriticalSection(&(p->print_mutex));
			cout << "\tLoading... " << "IP: " << IP << " failed\n";
			LeaveCriticalSection(&(p->print_mutex));
			ws.closeSocket();
			continue;
		}

		// find the status code in the reply
		status_end_idx = HEADreply.find("\n");
		/*if (status_end_idx <= 9) {
			cout << "End index is greater than the start. HEAD reply may be messed up?" << endl;
			continue;
		}*/
		if (status_end_idx == -1) {
			ws.closeSocket();
			continue;
		}
		status_code_string = HEADreply.substr(9, status_end_idx);
		//status_code = stoi(status_code_string.substr(0, 3));

		EnterCriticalSection(&(p->print_mutex));
		cout << "\tVerifying header... status code " << status_code_string.substr(0, 3) << "\n";
		LeaveCriticalSection(&(p->print_mutex));

		// if the status code is 400 or higher, 
		if (status_code_string.at(0) == '4') {
			ws.closeSocket();
			ws.createTCPSocket();

			if (ws.connectToServerIP(IP, port) == 1) {
				//printf("Connection error: %d\n", WSAGetLastError());
				ws.closeSocket();
				continue;
			}
			start = high_resolution_clock::now();
			EnterCriticalSection(&(p->print_mutex));
			cout << "\tConnecting on page... ";
			LeaveCriticalSection(&(p->print_mutex));
			if (ws.sendGETRequest(host, path, query)) {
				//std::cout << "request success\n";
				stop = high_resolution_clock::now();
				duration = duration_cast<milliseconds>(stop - start);
				EnterCriticalSection(&(p->print_mutex));
				cout << "done in " << duration.count() << " ms\n";
				LeaveCriticalSection(&(p->print_mutex));
			}
			else {
				EnterCriticalSection(&(p->print_mutex));
				cout << "failed\n";
				LeaveCriticalSection(&(p->print_mutex));
				ws.closeSocket();
				continue;
			}

			// receive reply
			start = high_resolution_clock::now();
			EnterCriticalSection(&(p->print_mutex));
			cout << "\tLoading... ";
			LeaveCriticalSection(&(p->print_mutex));
			
			if (ws.receive(GETreply)) {
				stop = high_resolution_clock::now();
				duration = duration_cast<milliseconds>(stop - start);
				EnterCriticalSection(&(p->print_mutex));
				cout << "done in " << duration.count() << " ms with " << GETreply.size() << " bytes\n";
				LeaveCriticalSection(&(p->print_mutex));

				InterlockedIncrement(&(p->num_crawled));
				InterlockedAdd(&(p->size_crawl), GETreply.size());

				// find the status code in the reply
				//cout << GETreply;
				status_end_idx = GETreply.find("\n");
				if (status_end_idx != -1) {
					status_code_string = GETreply.substr(9, status_end_idx);
					switch (status_code_string.at(0)) {
						case '2':
							InterlockedIncrement(&(p->num_200));
							break;
						case '3':
							InterlockedIncrement(&(p->num_300));
							break;
						case '4':
							InterlockedIncrement(&(p->num_400));
							break;
						case '5':
							InterlockedIncrement(&(p->num_500));
							break;
						default:
							InterlockedIncrement(&(p->num_other));
							break;
					}
					//status_code = stoi(status_code_string.substr(0, 3));
				}
				else {
					ws.closeSocket();
					continue;
				}

				EnterCriticalSection(&(p->print_mutex));
				cout << "\tVerifying header... status code " << status_code_string.substr(0, 3) << "\n";
				LeaveCriticalSection(&(p->print_mutex));

				if (status_code_string.at(0) == '2') {
					start = high_resolution_clock::now();
					EnterCriticalSection(&(p->print_mutex));
					cout << "\tParsing page... ";
					LeaveCriticalSection(&(p->print_mutex));
					int count = 0;
					status_end_idx = GETreply.find("http");

					/*if (status_end_idx >= status_code_string.size()) {
						cout << "End index is greater than the start. GET reply may be messed up?" << endl;
						continue;
					}*/
					if (status_end_idx != -1) {
						status_code_string = GETreply.substr(status_end_idx);
						while (status_end_idx != NULL) {
							count++;
							status_end_idx = status_code_string.find("http") + 1;
							status_code_string = status_code_string.substr(status_end_idx);
						}
					}
					stop = high_resolution_clock::now();
					duration = duration_cast<milliseconds>(stop - start);
					InterlockedAdd(&(p->total_links_found), count);
					EnterCriticalSection(&(p->print_mutex));
					cout << "done in " << duration.count() << " ms with " << count << " links\n";
					LeaveCriticalSection(&(p->print_mutex));
				}

			}
			else {
				EnterCriticalSection(&(p->print_mutex));
				cout << "failed\n";
				LeaveCriticalSection(&(p->print_mutex));
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

			//WaitForSingleObject(p->q_mutex, INFINITE);
			EnterCriticalSection(&(p->q_mutex));
			if (p->num_tasks == 0) {
				SetEvent(p->eventQuit); printf("Thread %d event quit.\n", GetCurrentThreadId());
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
		} //------------- left the critical section ------------------

		
	} // end of while loop for this thread
	printf("Thread %d done.\n", GetCurrentThreadId());
	Winsock::cleanUp();

	// signal that this thread is exiting
	ReleaseSemaphore(p->finished, 1, NULL);

	return 0;
}

static bool timeout(int time) {
	Sleep(time);
	return true;
}

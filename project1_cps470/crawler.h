#pragma once

#include "common.h"
#include "urlparser.h"
#include "winsock.h"

using namespace std::chrono;

// this class is passed to all threads, acts as shared memory
class Parameters {
public:
	HANDLE print_mutex;
	HANDLE q_mutex;
	HANDLE finished;
	HANDLE eventQuit;
	queue<string> *inq;
	queue<string> *outq;
	unordered_set<string> HOST_container;
	unordered_set<string> IP_container;
	int num_tasks;
};

static UINT thread_fun(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	// wait for mutex, then print and sleep inside the critical section
	WaitForSingleObject(p->print_mutex, INFINITE);				// lock mutex
	printf("Thread %d started\n", GetCurrentThreadId());		// always print inside critical section to avoid screen garbage
	ReleaseMutex(p->print_mutex);								// release critical section

	HANDLE	arr[] = { p->eventQuit, p->inq };

	Winsock::initialize();	// initialize 
	Winsock ws;

	while (true)
	{
		if (WaitForMultipleObjects(2, arr, false, INFINITE) == WAIT_OBJECT_0) // the eventQuit has been signaled 
			break;
		else // semaQ is signaled. decreased the semaphore count by 1
		{
			// obtain ownership of the mutex
			WaitForSingleObject(p->q_mutex, INFINITE);

			// ------------- entered the critical section ---------------

			string url = p->inq->front(); // get the item from the inputQ
			WaitForSingleObject(p->print_mutex, INFINITE);
			cout << "URL: " << url << "\n";
			ReleaseMutex(p->print_mutex);
			p->inq->pop();
			p->num_tasks--;

							// return mutex
			ReleaseMutex(p->q_mutex);
			// ------------- left the critical section ------------------		

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

			WaitForSingleObject(p->q_mutex, INFINITE);
			if (p->HOST_container.find(host) == p->HOST_container.end()) {
				// the IP is unique, so add it to the container
				p->HOST_container.insert(host);
				cout << "\tChecking host uniqueness... passed\n";
				ReleaseMutex(p->q_mutex);
			}
			else {
				cout << "\tChecking host uniqueness... failed\n";
				ReleaseMutex(p->q_mutex);
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

			// get IP from hostname
			string IP = ws.getIPfromhost(host, p->print_mutex);

			ws.createTCPSocket();

			if (ws.connectToServerIP(IP, port) == 1) {
				WaitForSingleObject(p->print_mutex, INFINITE);
				cout << "failed\n";
				ReleaseMutex(p->print_mutex);
				ws.closeSocket();
				continue;
			}
			else {
				stop = high_resolution_clock::now();
				duration = duration_cast<milliseconds>(stop - start);
				WaitForSingleObject(p->print_mutex, INFINITE);
				cout << "done in " << duration.count() << " ms, found " << IP << "\n";
			}

			WaitForSingleObject(p->q_mutex, INFINITE);
			if (p->IP_container.find(IP) == p->IP_container.end()) {
				// the IP is unique, so add it to the container
				p->IP_container.insert(IP);
				cout << "\tChecking IP uniqueness... passed\n";
				ReleaseMutex(p->q_mutex);
			}
			else {
				cout << "\tChecking IP uniqueness... failed\n";
				ReleaseMutex(p->q_mutex);
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

			// receive HEAD reply
			start = high_resolution_clock::now(); // start timer for loading HEAD reply
			WaitForSingleObject(p->print_mutex, INFINITE);
			cout << "\tLoading... ";
			ReleaseMutex(p->print_mutex);

			string HEADreply = "";
			if (ws.receive(HEADreply)) {
				//std::cout << "reply not success\n";
				//cout << HEADreply;
				stop = high_resolution_clock::now();
				duration = duration_cast<milliseconds>(stop - start);
				WaitForSingleObject(p->print_mutex, INFINITE);
				cout << "done in " << duration.count() << " ms with "<< HEADreply.size() << " bytes\n";
				ReleaseMutex(p->print_mutex);
			}
			else {
				WaitForSingleObject(p->print_mutex, INFINITE);
				cout << "failed\n";
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
					cout << "done in " << duration.count() << " ms with " << GETreply.size() <<" bytes\n";
					ReleaseMutex(p->print_mutex);
					cout << "GET: " << GETreply << endl;
				}
				else {
					cout << "Reply NOT received successfully.\n";
				}
			}
			else {
				// cout << "Further contact denied.\n"
			}
				ws.closeSocket();
				continue;
			}
			ws.closeSocket();

			// obtain ownership of the mutex
			WaitForSingleObject(p->q_mutex, INFINITE);
			// ------------- entered the critical section ------------------

			// write results into outputQ
			//p->outq->push(GETreply);

			// p->active_threads --;
			//p->num_tasks--;
			printf("Thread %d: num_tasks_left = %d\n", GetCurrentThreadId(), p->num_tasks);

			if (p->num_tasks == 0)
				SetEvent(p->eventQuit);

			ReleaseMutex(p->q_mutex);  // release the ownership of the mutex object to other threads
										// ------------- left the critical section ------------------
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
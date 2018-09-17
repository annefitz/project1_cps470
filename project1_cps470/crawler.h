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
			short port = parser.getPort();

			//cout << "Path: " << path << " Host : " << host << " Port: " << port << "\n";
			WaitForSingleObject(p->print_mutex, INFINITE);
			cout << "\tParsing URL... host " << host << ", port " << port << "\n";
			ReleaseMutex(p->print_mutex);
			// delay here: contact a peer, send a request, and receive/parse the response 

			ws.createTCPSocket();

<<<<<<< HEAD
			if (ws.connectToServer(host, port, p->print_mutex) == 2) {
				ws.closeSocket();
				continue;
=======
			if (ws.connectToServer(host, port, p->print_mutex, 1) != 0) {
				//printf("Connection error: %d\n", WSAGetLastError());
>>>>>>> 7c53aab45b6b04a7237b1ffec69a0a3485a7cc27
			}
			// construct a GET or HEAD request (in a string), send request
			// starting timer
			auto stop = high_resolution_clock::now();
			auto start = high_resolution_clock::now();
			auto duration = duration_cast<milliseconds>(stop - start);
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
			start = high_resolution_clock::now();
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
<<<<<<< HEAD
			cout << "STATUS CODE:" << status_code << endl;
			string reply = "";
			// if the status is a 400, download and crawl the IP
			if (status_code >= 400) {
				if (ws.sendGETRequest(host, path)) {
					cout << "\n:TEST:\n";
				}

				// receive reply
				if (ws.receive(reply)) {
					cout << "Reply received successfully.\n";
					cout << "GET REPLY: " << reply;
=======
			WaitForSingleObject(p->print_mutex, INFINITE);
			cout << "\tVerifying header... status code " << status_code << "\n";
			ReleaseMutex(p->print_mutex);

			// if the status code is 400 or higher, 
			if (status_code >= 400) {
				ws.closeSocket();
				ws.createTCPSocket();

				if (ws.connectToServer(host, port, p->print_mutex, 0) != 0) {
					//printf("Connection error: %d\n", WSAGetLastError());
				}
				start = high_resolution_clock::now();
				WaitForSingleObject(p->print_mutex, INFINITE);
				cout << "\tConnecting on page... ";
				ReleaseMutex(p->print_mutex);
				if (ws.sendGETRequest(host, path)) {
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
>>>>>>> 7c53aab45b6b04a7237b1ffec69a0a3485a7cc27
				}
				else {
					cout << "Reply NOT received successfully.\n";
				}
			}
			else {
<<<<<<< HEAD
				// cout << "Further contact denied.\n"
			}

=======
				ws.closeSocket();
				continue;
			}
>>>>>>> 7c53aab45b6b04a7237b1ffec69a0a3485a7cc27
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
				
		}
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
#pragma once

#include "common.h"
#include "urlparser.h"
#include "winsock.h"

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
			p->inq->pop();
			p->num_tasks--;

			//p->num_tasks++; // p->active_threads ++;

							// return mutex
			ReleaseMutex(p->q_mutex);
			// ------------- left the critical section ------------------		

			// parse url
			URLParser parser(url);
			string host = parser.getHost();
			string path = parser.getPath();
			short port = parser.getPort();

			cout << "Path: " << path << " Host : " << host << " Port: " << port << "\n";

			// delay here: contact a peer, send a request, and receive/parse the response 

			ws.createTCPSocket();

			if (ws.connectToServer(host, port, p->print_mutex) != 0) {

			}
			// construct a GET or HEAD request (in a string), send request
			if (ws.sendHEADRequest(host)) {

			}

			// receive HEAD reply
			string HEADreply = "";
			if (ws.receive(HEADreply)) {
				//std::cout << "reply not success\n";
				cout << HEADreply;
			}
			else {
				cout << "Error in receiving reply from the host.\n";
			}

			// find the status code in the reply
			int status_end_idx = HEADreply.find("\n");
			string status_code_string = HEADreply.substr(9, status_end_idx);
			int status_code = stoi(status_code_string.substr(0, 2));

			string reply = "";
			// if the status code isn't 200, 
			if (status_code != 200) {
				if (ws.sendGETRequest(host, path)) {
					//std::cout << "request success\n";
				}

				// receive reply
				if (ws.receive(reply)) {
					std::cout << "Reply received successfully.\n";
					// std::cout << reply;
				}
				else {
					std::cout << "Reply NOT received successfully.\n";
				}
			}

			ws.closeSocket();
			// obtain ownership of the mutex
			WaitForSingleObject(p->q_mutex, INFINITE);
			// ------------- entered the critical section ------------------

			// write results into outputQ
			p->outq->push(reply);

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
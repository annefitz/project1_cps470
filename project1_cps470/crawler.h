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
	HANDLE qq;
	int num_tasks;
};

static UINT thread_fun(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	// wait for mutex, then print and sleep inside the critical section
	WaitForSingleObject(p->print_mutex, INFINITE);				// lock mutex
	printf("Thread %d started\n", GetCurrentThreadId());		// always print inside critical section to avoid screen garbage
	ReleaseMutex(p->print_mutex);								// release critical section

	HANDLE	arr[] = { p->eventQuit, p->qq };

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
			// ------------- entered the critical section ------------------
			Sleep(1000);  // let this thread sleep for 1 second, just for code demonstration

			//string url = p->qq.pop(); // get the item from the inputQ
			p->num_tasks++; // p->active_threads ++;

							// return mutex
			ReleaseMutex(p->q_mutex);
			// ------------- left the critical section ------------------		

			// parse url
			/*URLParser parser(url);
			string host = parser.getHost();
			string path = parser.getPath();
			short port = parser.getPort();

			cout << "Path: " << path << " Host : " << host << " Port: " << port << "\n";

			// delay here: contact a peer, send a request, and receive/parse the response 

			ws.createTCPSocket();

			if (ws.connectToServer(host, port) != 0) {

			}
			// construct a GET or HEAD request (in a string), send request
			if (ws.sendRequest(host, path)) {
				//std::cout << "request success\n";
			}
			// receive reply
			string reply = "";
			if (ws.receive(reply)) {
				//std::cout << "reply success\n";
				std::cout << reply;
			}
			else {
				std::cout << "Unsuccessful reply.\n";
			}

			ws.closeSocket();

			Winsock::cleanUp();*/

							// obtain ownership of the mutex
			WaitForSingleObject(p->q_mutex, INFINITE);
			// ------------- entered the critical section ------------------

			// write results into outputQ
			//p->qq.push(reply);

			// p->active_threads --;
			p->num_tasks--;
			printf("Thread %d: num_tasks_left = %d\n", GetCurrentThreadId(), p->num_tasks);

			if (p->num_tasks == 0)
				SetEvent(p->eventQuit);

			Sleep(1000);	// let this thread sleep for 1 second, just for code demonstration		

			ReleaseMutex(p->q_mutex);  // release the ownership of the mutex object to other threads
										// ------------- left the critical section ------------------		
		}
	} // end of while loop for this thread

		// signal that this thread is exiting 
	ReleaseSemaphore(p->finished, 1, NULL);

	return 0;
}

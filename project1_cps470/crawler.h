#pragma once

#include "common.h"

// this class is passed to all threads, acts as shared memory
class Parameters {
public:
	HANDLE mutex;
	HANDLE finished;
	HANDLE eventQuit;
	HANDLE semaQ;
	int num_tasks;
};

// this function is where the thread starts
static UINT thread(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	// wait for mutex, then print and sleep inside the critical section
	WaitForSingleObject(p->mutex, INFINITE);					// lock mutex
	printf("Thread %d started\n", GetCurrentThreadId());		// always print inside critical section to avoid screen garbage
	ReleaseMutex(p->mutex);										// release critical section

	HANDLE	arr[] = { p->eventQuit, p->semaQ };
	while (true)
	{
		if (WaitForMultipleObjects(2, arr, false, INFINITE) == WAIT_OBJECT_0) // the eventQuit has been signaled 
			break;
		else // semaQ is signaled. decreased the semaphore count by 1
		{

			// obtain ownership of the mutex
			WaitForSingleObject(p->mutex, INFINITE);
			// ------------- entered the critical section ------------------
			Sleep(1000);  // let this thread sleep for 1 second, just for code demonstration

							// p->active_threads ++; 
							// get the item from the inputQ

							// return mutex
			ReleaseMutex(p->mutex);
			// ------------- left the critical section ------------------		

			// delay here: contact a peer, send a request, and receive/parse the response 

			Sleep(5000); // let this thread sleep for 5 seconds, just for code demonstration

							// obtain ownership of the mutex
			WaitForSingleObject(p->mutex, INFINITE);
			// ------------- entered the critical section ------------------

			// write results into outputQ
			// p->active_threads --;

			p->num_tasks--;
			printf("Thread %d: num_tasks_left = %d\n", GetCurrentThreadId(), p->num_tasks);

			if (p->num_tasks == 0)
				SetEvent(p->eventQuit);

			Sleep(1000);	// let this thread sleep for 1 second, just for code demonstration		

			ReleaseMutex(p->mutex);  // release the ownership of the mutex object to other threads
										// ------------- left the critical section ------------------		
		}
	} // end of while loop for this thread

		// signal that this thread is exiting 
	ReleaseSemaphore(p->finished, 1, NULL);

	return 0;
}


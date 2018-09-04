#pragma once
#include "crawler.h"
#include "winsock.h"
#include "common.h"
#include "urlparser.h"

//void update(string  & s)
//{
//	s = ""; 
//}

int main(int argc, char* argv[])
{
	//string testString = "Hello"; 
	//update(testString);
	//cout << testString << endl; 

	Winsock::initialize();	// initialize 

	Winsock ws;
	short port = 80; 

	// parse url to get host name, port, path, and so on.
	string url = "https://www.udayton.edu/apply/index.php"; 
	URLParser parser(url);
	string host0 = parser.getHost(); 

	// the following shows how to use winsock functions

	string host = "www.yahoo.com";
	ws.createTCPSocket();
	ws.connectToServer(host, port);
	// construct a GET or HEAD request (in a string), send request
	// receive reply
	ws.closeSocket(); 

	// parse url to get host name, port, path, and so on.

	string hostIP = "131.238.72.77";  // udayton.edu's IP
	ws.createTCPSocket();
	ws.connectToServerIP(hostIP, port);
	// construct a GET or HEAD request (in a string), send request
	// receive reply
	ws.closeSocket();


	printf("-----------------\n");

	// thread handles are stored here; they can be used to check status of threads, or kill them
	HANDLE *ptrs = new HANDLE[THREAD_COUNT];
	Parameters p;
	int num_peers = 20;

	// create a mutex for accessing critical sections (including printf)
	p.mutex = CreateMutex(NULL, 0, NULL);

	// create a semaphore that check if a thread finishs its task
	p.finished = CreateSemaphore(NULL, 0, THREAD_COUNT, NULL);

	//	p.active_threads = 0;
	p.num_tasks = num_peers;

	// create a manual reset event to determine the termination condition is true
	p.eventQuit = CreateEvent(NULL, true, false, NULL);

	// create a semaphore to keep track of the number of items in the inputQ. The initial size of inputQ is num_peers
	p.semaQ = CreateSemaphore(NULL, num_peers, MAX_SEM_COUNT, NULL);

	// get current system time
	DWORD t = timeGetTime();

	for (int i = 0; i < THREAD_COUNT; ++i)
	{
		// structure p is the shared space between the threads		
		ptrs[i] = CreateThread(NULL, 4096, (LPTHREAD_START_ROUTINE)thread, &p, 0, NULL);
	}
	printf("-----------created %d threads-----------\n", THREAD_COUNT);

	// make sure this main thread hangs here until the other two quit; otherwise, the program will terminate prematurely
	for (int i = 1; i <= THREAD_COUNT; ++i)
	{
		WaitForSingleObject(p.finished, INFINITE);
		printf("%d thread finished. main() function there--------------\n", i);
	}
	printf("Terminating main(), completion time %d ms\n", timeGetTime() - t); 


	

	Winsock::cleanUp(); 

	printf("Enter any key to continue ...\n"); 
	getchar(); 

	return 0;   // 0 means successful
}
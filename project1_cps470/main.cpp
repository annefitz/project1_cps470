#pragma once
#include "crawler.h"
#include "winsock.h"
#include "common.h"
#include "urlparser.h"


int main(int argc, char* argv[])
{

	// Get filename from commandline
	int num_threads = stoi(argv[1]);
	string filename = argv[2];

	// File I/O
	ifstream fin;
	fin.open(filename);

	if (fin.fail()) {
		cout << "File failed to open.\n";
		return 1;
	}
	else {
		cout << "Opened " << filename << "\n";
	}

	// push all URLs onto queue
	string turl = "";
	queue<string> inQ;
	while (!fin.eof()) {
		fin >> turl;
		cout << turl << endl;
		inQ.push(turl);
	}
	getchar();
	fin.close();

	HANDLE print_m = CreateMutex(NULL, true, NULL);
    //HANDLE q_m = CreateMutex(NULL, true, NULL);
	//LPCRITICAL_SECTION q_m;
	HANDLE unique_m = CreateMutex(NULL, true, NULL);
	HANDLE stats_m = CreateMutex(NULL, true, NULL);
	HANDLE event_quit = CreateEvent(NULL, true, false, NULL);
	HANDLE thread_finish = CreateSemaphore(NULL, 0, 1, NULL);

	// threading
	Parameters p = Parameters();

	// shared stats
	p.num_HOST_unique = 0;
	p.num_DNS = 0;
	p.num_IP_unique = 0;
	p.num_robots = 0;
	p.num_URLs = 0;
	p.total_links_found = 0;
	
	mutex print_m;
	mutex q_m;
	mutex unique_m;
	HANDLE event_quit = CreateEventA(NULL, true, false, NULL);
	HANDLE thread_finish = CreateSemaphoreA(NULL, 0, 1, NULL);

	// threading
	Parameters p;
	p.eventQuit = event_quit;
	p.finished = thread_finish;
	p.num_tasks = size(inQ);
	p.inq = &inQ;
	p.print_mutex = &print_m;
	p.q_mutex = &q_m;
	p.unique_mutex = &unique_m;
	HANDLE *t = new HANDLE[num_threads];

	//InitializeCriticalSection(p.q_mutex);
	auto start = high_resolution_clock::now(); // instantiate vars

	// spawn each thread and store them in the thread array
	for (int i = 0; i < num_threads; i++) {
		// t[i] = thread(thread_fun, i, ref(p));
		t[i] = CreateThread(NULL, 4096, (LPTHREAD_START_ROUTINE)thread_fun, &p, 0, NULL);
	}

	// wait for threads to terminate
	for (int i = 0; i < num_threads; i++) {
		WaitForSingleObject(p.finished, INFINITE);
	}
	auto stop = high_resolution_clock::now();  // instantiate vars
	auto crawl_duration = duration_cast<milliseconds>(stop - start); // total time elapsed

	// data calculatiions
	int url_ps = p.num_URLs / (crawl_duration.count() / 1000);

	// data print block
	printf("Extracted %d URLs @ %d/s\n", p.num_URLs, url_ps);

	delete[] t;

	//Winsock::cleanUp;
	//printf("Enter any key to continue ...\n"); 
	cout << "\nWAIT FOR ENTER!!!! ";
	getchar();

	return 0;   // 0 means successful
}
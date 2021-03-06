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
		//cout << turl << endl;
		inQ.push(turl);
	}

	fin.close();

	//HANDLE print_m = CreateMutexA(NULL, true, NULL);
    //HANDLE q_m = CreateMutex(NULL, true, NULL);
	//LPCRITICAL_SECTION q_m;
	//HANDLE unique_m = CreateMutexA(NULL, true, NULL);
	HANDLE stats_m = CreateMutexA(NULL, true, NULL);
	HANDLE event_quit = CreateEventA(NULL, true, false, NULL);
	HANDLE q_empty = CreateEventA(NULL, true, false, NULL);
	HANDLE thread_finish = CreateSemaphoreA(NULL, 0, 1, NULL);

	// threading
	Parameters p = Parameters();

	// shared stats
	p.num_HOST_unique = 0;
	p.num_DNS = 0;
	p.num_IP_unique = 0;
	p.num_robots = 0;
	p.num_URLs = 0;
	p.num_crawled = 0;
	p.total_links_found = 0;

	p.size_crawl = 0;
	
	p.num_200 = 0;
	p.num_300 = 0;
	p.num_400 = 0;
	p.num_500 = 0;
	p.num_other = 0;

	//mutex print_m;
	//mutex q_m;
	//mutex unique_m;

	p.eventQuit = event_quit;
	p.finished = thread_finish;
	p.num_tasks = size(inQ);
	p.inq = &inQ;
	//p.print_mutex = &print_m;
	//p.q_mutex = &q_m;
	//p.unique_mutex = &unique_m;
	HANDLE *t = new HANDLE[num_threads];

	//InitializeCriticalSection(p.q_mutex);
	auto start = high_resolution_clock::now(); // instantiate vars

	Winsock::initialize();
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

	// data print block
	printf("Extracted %d URLs @ %d/s\n", p.num_URLs, p.num_URLs/ (crawl_duration.count()/1000));
	printf("Looked up %d DNS names @ %d/s\n", p.num_DNS, p.num_DNS/ (crawl_duration.count()/1000));
	printf("Downloaded %d robots @ %d/s\n", p.num_robots, p.num_robots/ (crawl_duration.count()/1000));
	printf("Crawled %d pages @ %d/s (%.2f MB)\n", p.num_crawled, p.num_crawled/ (crawl_duration.count()/1000), (float) p.size_crawl/1000000);
	printf("Parsed %d links @ %d/s\n", p.total_links_found, p.total_links_found/ (crawl_duration.count()/1000));
	printf("HTTP codes: 2xx = %d, 3xx = %d, 4xx = %d, 5xx = %d, other = %d", p.num_200, p.num_300, p.num_400, p.num_500, p.num_other);

	delete[] t;

	// Winsock::cleanUp;

	//printf("Enter any key to continue ...\n"); 
	cout << "\nWAIT FOR KEYPRESS!!!!! ";
	getchar();

	return 0;   // 0 means successful
}
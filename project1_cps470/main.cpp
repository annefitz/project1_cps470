#pragma once
#include "crawler.h"
#include "winsock.h"
#include "common.h"
#include "urlparser.h"


int main(int argc, char* argv[])
{

	// Get filename from commandline
	int num_threads = (int) atoi(argv[1]);
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
	queue<string> outQ;
	while (!fin.eof()) {
		fin >> turl;
		//cout << turl << endl;
		inQ.push(turl);
	}

	fin.close();

	mutex print_m;
	mutex q_m;
	HANDLE event_quit = CreateEventA(NULL, true, false, NULL);
	HANDLE thread_finish = CreateSemaphoreA(NULL, 0, 1, NULL);

	// threading
	Parameters p;
	p.eventQuit = event_quit;
	p.finished = thread_finish;
	p.num_tasks = size(inQ);
	p.inq = &inQ;
	p.outq = &outQ;
	p.print_mutex = &print_m;
	p.q_mutex = &q_m;
	HANDLE t[1];

	// spawn each thread and store them in the thread array
	for (int i = 0; i < num_threads; i++) {
		// t[i] = thread(thread_fun, i, ref(p));
		t[i] = CreateThread(NULL, 4096, (LPTHREAD_START_ROUTINE)thread_fun, &p, 0, NULL);
	}
	// wait for threads to terminate
	for (int i = 0; i < num_threads; i++) {
		WaitForSingleObject(p.finished, INFINITE);
		cout << "FINISHED\n";
	}

	//printf("Enter any key to continue ...\n"); 
	cout << "\nWAIT FOR KEYPRESS!!!!! ";
	getchar();


	return 0;   // 0 means successful
}

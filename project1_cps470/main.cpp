#pragma once
#include "crawler.h"
#include "winsock.h"
#include "common.h"
#include "urlparser.h"


int main(int argc, char* argv[])
{
	Winsock::initialize();	// initialize 

	Winsock ws; 

	// Get filename from commandline
	const int num_threads = atoi(argv[1]);
	string filename = argv[2];
	cout << "Filename: " << filename << "\n";

	// File I/O
	ifstream fin;
	fin.open(filename);

	if (fin.fail()) {
		printf("File failed to open.\n");
		return 1;
	}

	// push all URLs onto queue
	string turl = "";
	queue<string> Q;
	while (!fin.eof()) {
		fin >> turl;
		cout << turl << endl;
		Q.push(turl);
	}

	fin.close();

	mutex print_m;
	mutex q_m;

	// threading
	Parameters p;
	p.num_tasks = 0;
	p.qq = &Q;
	p.print_mutex = &print_m;
	p.q_mutex = &q_m;
	thread t(thread_fun, 1, ref(p));
	//thread t[1];
	// spawn each thread and store them in the thread array
	//for (int i = 0; i < num_threads; i++) {
		//t[i] = thread(thread_fun, i, ref(p));
	//}
	// wait for threads to terminate
	//for (int i = 0; i < num_threads; i++) {
		//t[i].join();
	//}
	t.join();

	// parse url to get host name, port, path, and so on.
	URLParser parser(turl);
	string host = parser.getHost();
	string path = parser.getPath();
	short port = parser.getPort();

	cout << "Path: " << path << " Host : " << host << " Port: " << port << "\n";

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

	ws.closeSocket();
	

	Winsock::cleanUp(); 

	printf("Enter any key to continue ...\n"); 
	getchar(); 

	return 0;   // 0 means successful
}

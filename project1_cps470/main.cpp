#pragma once
#include "crawler.h"
#include "winsock.h"
#include "common.h"
#include "urlparser.h"
#include "main.h"


int main(int argc, char* argv[])
{
	Winsock::initialize();	// initialize 

	Winsock ws; 

	// Get filename from commandline
	string filename = argv[2];
	int const num_threads = (int)argv[1];
	cout << "Filename: " << filename << "\n";

	// File I/O
	ifstream fin;

	ofstream fout;
	fin.open(filename);
	fout.open("crawldata.txt");

	// push all URLs onto queue

	fin.open(filename);
	string turl = "";

	queue<string> outputQ;

	if (fin.fail()) {
		printf("File failed to open.\n");
		return 1;
	}

	string turl = "";
	queue<string> inputQ;
	while (!fin.eof()) {
		fin >> turl;
		cout << turl << endl;
		inputQ.push(turl);
	}

	fin.close();

	mutex m;

	// threading
	Parameters p;
	p.num_tasks = 0;
	p.inputQ = &inputQ;
	p.mutex = &m;
	thread t[num_threads];


	// parse url to get host name, port, path, and so on.
	URLParser parser(url);
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

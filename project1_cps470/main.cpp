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
	Winsock::initialize();	// initialize 

	Winsock ws; 

	// File I/O
	ifstream fin;
	ofstream fout;
	fin.open("URL-input-100.txt");
	fout.open("crawldata.txt");
	string url = "";
	if (fin.fail()) {
		printf("No such file. Failed to open.\n"); 
		return;
	}

	// push all URLs onto queue
	queue<string> Q;
	while (!fin.eof()) {
		fin >> url; // read a single string in
		cout << url << endl; // print for debugging
		Q.push(url);
	}


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

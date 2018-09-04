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

	// parse url to get host name, port, path, and so on.
	string url = "http://www.reddit.com/r/AnimalsBeingDerps"; 
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
		std::cout << "request success\n";
	}
	// receive reply
	string reply = "";
	if (ws.receive(reply)) {
		std::cout << "reply success\n";
		std::cout << reply;
	}

	ws.closeSocket();
	

	Winsock::cleanUp(); 

	printf("Enter any key to continue ...\n"); 
	getchar(); 

	return 0;   // 0 means successful
}

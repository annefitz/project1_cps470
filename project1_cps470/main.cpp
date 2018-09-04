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
	string url = "https://www.udayton.edu/apply/index.php"; 
	URLParser parser(url);
	string host = parser.getHost();
	string path = parser.getPath();
	short port = parser.getPort();

<<<<<<< HEAD
	ws.createTCPSocket();
=======
	// the following shows how to use winsock functions

	//string host = "www.yahoo.com";
	ws.createTCPSocket();
	ws.connectToServer(host0, port);
	// construct a GET or HEAD request (in a string), send request

	// receive reply

	ws.closeSocket(); 
>>>>>>> b9f0ece634e58b1719cde3b8fba3e2cf12e2a0f0

	if (ws.connectToServer(host, port) != 0) {

<<<<<<< HEAD
	}
	// construct a GET or HEAD request (in a string), send request
	if (ws.sendRequest(host, path)) {
=======
	//string hostIP = "131.238.72.77";  // udayton.edu's IP
	//ws.createTCPSocket();
	//ws.connectToServerIP(hostIP, port);
	// construct a GET or HEAD request (in a string), send request
	// receive reply
	//ws.closeSocket();


	printf("-----------------\n");
>>>>>>> b9f0ece634e58b1719cde3b8fba3e2cf12e2a0f0

	}
	// receive reply
	string reply = "";
	if (ws.receive(reply)) {

	}

	ws.closeSocket();
	

	Winsock::cleanUp(); 

	printf("Enter any key to continue ...\n"); 
	getchar(); 

	return 0;   // 0 means successful
}
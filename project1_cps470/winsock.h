#pragma once
#include "common.h" 

using namespace std::chrono;

#define BUF_SIZE 1024
#define TIMEOUT 20000
// the .h file defines all windows socket functions 

class Winsock
{
public: 
	static int initialize()   // call Winsock::intialize() in main, to intialize winsock only once
	{
		WSADATA wsaData;
		int iResult;

		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);   // defined in winsock2.h
		if (iResult != 0) {
			printf("WSAStartup failed: %d\n", iResult);
			WSACleanup();
			return 1;  // 1 means failed
		}
		return 0;   // 0 means no error (i.e., successful)
	}

	static void cleanUp() // call Winsock::cleanUp() in main only once
	{
		WSACleanup();
	}

	int createTCPSocket(void)
	{
		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == INVALID_SOCKET) {
			printf("socket() error %d\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}	
		return 0; 
	}

	/*
	int connectToServer(string host, short port, HANDLE print_mutex, int print)
	{


		WaitForSingleObject(print_mutex, INFINITE);
		cout << "\tDoing DNS... ";
		ReleaseMutex(print_mutex);

		// starting timer
		auto start = high_resolution_clock::now();
		// structure for connecting to server
		struct sockaddr_in server;

		// structure used in DNS lookups
		struct hostent *remote;

		// first assume that the string is an IP address
		DWORD IP = inet_addr(host.c_str());
		if (IP == INADDR_NONE)
		{
			// if not a valid IP, then do a DNS lookup
			if ((remote = gethostbyname(host.c_str())) == NULL)
			{
				WaitForSingleObject(print_mutex, INFINITE);
				printf("Invalid string: neither FQDN, nor IP address\n");
				ReleaseMutex(print_mutex);
				return 1;  // 1 means failure
			}
			else {// take the first IP address and copy into sin_addr
				memcpy((char *)&(server.sin_addr), remote->h_addr, remote->h_length);
			}
		}
		else
		{
			// if a valid IP, directly drop its binary version into sin_addr
			server.sin_addr.S_un.S_addr = IP;
		}

		// setup the port # and protocol type
		server.sin_family = AF_INET;
		server.sin_port = htons(port);		// host-to-network flips the byte order
		// connect to the server on that port 
		if (connect(sock, (struct sockaddr*) &server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
		{
			WaitForSingleObject(print_mutex, INFINITE);
			printf("Connection error: %d\n", WSAGetLastError());
			ReleaseMutex(print_mutex);
			return 1;
		}

		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(stop - start);
		if (print == 1) {
			WaitForSingleObject(print_mutex, INFINITE);
			cout << "done in " << duration.count() << "ms, found " << inet_ntoa(server.sin_addr) << "\n";
			//printf("Successfully connected to %s (%s) on port %d\n", host.c_str(), inet_ntoa(server.sin_addr), htons(server.sin_port));
			ReleaseMutex(print_mutex);

			// check IP for uniqueness: if not unique, return non-zero
			if (isIPUnique(host)) {
				WaitForSingleObject(print_mutex, INFINITE);
				cout << "\tChecking IP uniqueness... passed\n";
				ReleaseMutex(print_mutex);
			}
			else {
				WaitForSingleObject(print_mutex, INFINITE);
				cout << "\tChecking IP uniqueness... failed\n";
				ReleaseMutex(print_mutex);
				return 2; // failed
			}
		}
		return 0; 
	}
	*/

	string getIPfromhost(string host, HANDLE print_mutex) {
		// structure for connecting to server
		struct sockaddr_in server;

		// structure used in DNS lookups
		struct hostent *remote;

		// first assume that the string is an IP address
		DWORD IP = inet_addr(host.c_str());
		if (IP == INADDR_NONE)
		{
			// if not a valid IP, then do a DNS lookup
			if ((remote = gethostbyname(host.c_str())) == NULL)
			{
				WaitForSingleObject(print_mutex, INFINITE);
				printf("Invalid string: neither FQDN, nor IP address\n");
				ReleaseMutex(print_mutex);
				return NULL;  // NULL means failure
			}
			else {// take the first IP address and copy into sin_addr
				memcpy((char *)&(server.sin_addr), remote->h_addr, remote->h_length);
			}
		}
		else
		{
			// if a valid IP, directly drop its binary version into sin_addr
			server.sin_addr.S_un.S_addr = IP;
		}
		return inet_ntoa(server.sin_addr);
	}


	//// hostName (e.g., "www.google.com"),  2-byte port (e.g., 80)
	//int connectToServer(string hostName, short port)
	//{
	//	struct sockaddr_in server; // structure for connecting to server
	//	struct hostent *remote;    // structure used in DNS lookups: convert host name into IP address
	//	
	//	if ((remote = gethostbyname(hostName.c_str())) == NULL)
	//	{
	//		printf("Invalid host name string: not FQDN\n");
	//		return 1;  // 1 means failed
	//	}
	//	else // take the first IP address and copy into sin_addr
	//	{
	//		memcpy((char *)&(server.sin_addr), remote->h_addr, remote->h_length);
	//	}

	//	// setup the port # and protocol type
	//	server.sin_family = AF_INET;  // IPv4
	//	server.sin_port = htons(port);// host-to-network flips the byte order

	//	// connect to the server 
	//	if (connect(sock, (struct sockaddr*) &server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	//	{
	//		printf("Connection error: %d\n", WSAGetLastError());
	//		return 1;
	//	}	
	//	printf("Successfully connected to %s (%s) on port %d\n", hostName.c_str(), inet_ntoa(server.sin_addr),
	//		htons(server.sin_port));
	//	return 0; 
	//}

	// dot-separated hostIP (e.g., "132.11.22.2"), 2-byte port(e.g., 80)
	int connectToServerIP(string hostIP, short port)
	{
		struct sockaddr_in server; // structure for connecting to server

		DWORD IP = inet_addr(hostIP.c_str());
		if (IP == INADDR_NONE)
		{ 
			//printf("Invalid IP string: nor IP address\n");
			return 1;  // 1 means failed						
		}
		else
		{			
			server.sin_addr.S_un.S_addr = IP; // if a valid IP, directly drop its binary version into sin_addr
		}

		// setup the port # and protocol type
		server.sin_family = AF_INET;  // IPv4
		server.sin_port = htons(port);// host-to-network flips the byte order
									  
		if (connect(sock, (struct sockaddr*) &server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
		{
			//printf("Connection error: %d\n", WSAGetLastError());
			return 1;
		}
		//printf("Successfully connected to %s (%s) on port %d\n", hostIP.c_str(), inet_ntoa(server.sin_addr),
		//	htons(server.sin_port));

		return 0;
	}

	// define your sendRequest(...) function, to send a HEAD or GET request
	bool sendGETRequest(string host, string path, string query)
	{
		string sendstring = "GET /" + path + "/" + query + " HTTP/1.0\nUser-agent:UDCScrawler/1.0\nHost:" + host + "\nConnection: close" + "\n\n";
		int size = sendstring.length();
		if (send(sock, sendstring.c_str(), size, 0) == SOCKET_ERROR)
		{
			printf("send() error - %d\n", WSAGetLastError());
			return false;
		}
		return true;
	}

	// HEAD request
	bool sendHEADRequest(string host)
	{
		string sendstring = "HEAD /robots.txt HTTP/1.0\nHost: " + host + "\n\n";
		int size = sendstring.length();
		if (send(sock, sendstring.c_str(), size, 0) == SOCKET_ERROR)
		{
			printf("send() error - %d\n", WSAGetLastError());
			return false;
		}
		return true;
	}

	// define your receive(...) function, to receive the reply from the server
	bool receive(string & recv_string)
	{
		FD_SET Reader;
		FD_ZERO(&Reader);
		FD_SET(sock, &Reader);

		struct timeval timeout;
		timeout.tv_sec = TIMEOUT;
		timeout.tv_usec = 0;

		recv_string = "";
		int bytes = 0;
		do {
			if (select(0, &Reader, NULL, NULL, &timeout) > 0)
			{
				if ((bytes = recv(sock, buf, BUF_SIZE - 1, 0)) == SOCKET_ERROR)
				{
					printf("Failed with %d on recv.\n", WSAGetLastError());
					return false;
				}
				else if (bytes > 0)
				{
					buf[bytes] = 0;
					recv_string += buf;
				}
			}
			else
			{
				return false;
			}
		} while (bytes > 0);
		return true;
	}

	void closeSocket(void)
	{
		closesocket(sock);	
	}



private:
	SOCKET sock;
	char buf[BUF_SIZE];

	// define other private variables if needed

};
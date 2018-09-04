#pragma once
#include "common.h" 

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

	// connect to host (e.g., "www.google.com", or "121.223.12.2") on given port (e.g., 80)
	int connectToServer(string host, short port)
	{
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
				printf("Invalid string: neither FQDN, nor IP address\n");
				return 1;  // 1 means failure
			}
			else // take the first IP address and copy into sin_addr
				memcpy((char *)&(server.sin_addr), remote->h_addr, remote->h_length);
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
			printf("Connection error: %d\n", WSAGetLastError());
			return 1;
		}

		printf("Successfully connected to %s (%s) on port %d\n", host.c_str(), inet_ntoa(server.sin_addr), htons(server.sin_port));
		return 0; 
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
			printf("Invalid IP string: nor IP address\n");
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
			printf("Connection error: %d\n", WSAGetLastError());
			return 1;
		}
		printf("Successfully connected to %s (%s) on port %d\n", hostIP.c_str(), inet_ntoa(server.sin_addr),
			htons(server.sin_port));

		return 0;
	}

	// define your sendRequest(...) function, to send a HEAD or GET request
	

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
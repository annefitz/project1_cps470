#pragma once
#pragma comment (lib, "ws2_32.lib")  // link to winsock lib  in winsock.h
#pragma comment (lib, "winmm.lib")   // for timeGetTime() in main()
#define _WINSOCK_DEPRECATED_NO_WARNINGS  // for inet_addr(), gethostbyname() in winsock.h

#include <winsock2.h>
#include <windows.h>

#include <stdio.h>  // for printf
#include <iostream> // for cin, cout
#include <fstream> // for file open, close
#include <queue> // for queues
#include <thread>
#include <mutex> // for mutexes
#include <string> // for string manipulation
#include <unordered_set>  // this is Hash Table, used to check ip/host uniqueness
#include <chrono> // for setting timer on functions


using namespace std;   // if need std


#define THREAD_COUNT	5			// number of threads created in main
#define MAX_SEM_COUNT	10000000	// the maximum queue size
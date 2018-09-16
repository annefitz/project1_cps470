#pragma once
#include "common.h"

class URLParser {

public:

	// constructor 
	URLParser(string & link)  // & means pass the address of link to this function
	{
		url = link; 	
		host = ""; 
		port = 80;  // default port is 80 for web server
		path = "/";  // if path is empty, use "/"
		query = "";
	}


	/* url format: 
	* scheme://[user:pass@]host[:port][/path][?query][#fragment]
	*/ 

	// e.g., url: "http://cs.somepage.edu:467/index.php?addrbook.php"
	// host: "cs.somepage.edu"
	string getHost()
	{
		// implement here, you may use url.find(...) 
		int start;
<<<<<<< HEAD
		start = url.find("/"); start = url.find("/", start+1);
=======
		
		start = url.find("/"); start = url.find("/", start+1);
		
>>>>>>> a54a04f31f8c94e0dba803dd9184e315abfe0db1

		int stop = url.find('/', start+1); // check after the starting index
		if (stop == -1) {
			stop = url.size();
		}

		//std::cout << "start: " << start << " end: " << stop << "\n";

		if (start != -1) {
			host = url.substr(start + 1, stop - start - 1);
		}
		else {
			cout << "Bad hostname.\n";
		}
		
		return host;
	}

	// e.g., url: "http://cs.somepage.edu:467/index.php?addrbook.php"
	// port: 467
	short getPort()
	{		
		string sPort = ""; 

		// implement here: find substring that represents the port number
		int stop = url.find('/', 8);
		int start = (url.substr(0,stop)).find(':', 8);
		if (start != -1) { // 0 because -1 is the error code, but adding 1 above
			sPort = url.substr(start + 1, stop - start - 1);
		}
		else {
			sPort = "80";
		}

		if (sPort.length() > 0)
			port = atoi(sPort.c_str());  // convert substring sPort to an integer value

		return port;
	}

	// url: "http://cs.somepage.edu:467/index.php?addrbook.php"
	// path is "/index.php"
	string getPath()
	{
		// implement here
		int start = url.find('/', 8);
		int stop = url.find('?', 8);
		
		if (start != -1) {
		// std::cout << "start: " << start << "\n" << "end: " << stop << "\n";
			path = url.substr(start + 1, stop - start - 1);
		}
		else {
			path = "/";
		}
		return path;
	}

	// url: "http://cs.somepage.edu:467/index.php?addrbook.php"
	// query is "?addrbook.php"
	string getQuery()
	{
		// implement here
		int start = url.find('?', 8);
		int stop = url.size();
		if (start != -1) {
			query = url.substr(start + 1, stop - start);
		}
		return query;
	}


private:
	string url; 
	string host; 
	short port; 
	string path;
	string query; 

};
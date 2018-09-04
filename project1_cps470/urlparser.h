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

		return host; 
	}

	// e.g., url: "http://cs.somepage.edu:467/index.php?addrbook.php"
	// port: 467
	short getPort()
	{		
		string sPort = ""; 

		// implement here: find substring that represents the port number

		if (sPort.length() > 0)
			port = atoi(sPort.c_str());  // convert substring sPort to an integer value

		return port;
	}

	// url: "http://cs.somepage.edu:467/index.php?addrbook.php"
	// path is "/index.php"
	string getPath()
	{
		// implement here
	
		return path; 
	}

	// url: "http://cs.somepage.edu:467/index.php?addrbook.php"
	// query is "?addrbook.php"
	string getQuery()
	{
		// implement here

		return query;
	}


private:
	string url; 
	string host; 
	short port; 
	string path;
	string query; 

};
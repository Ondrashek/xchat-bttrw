#pragma implementation
#include "http.h"
#include "str.h"
#include <cstdio>

namespace net {
    TomiHTTP::TomiHTTP()
    {
    }

    TomiHTTP::~TomiHTTP()
    {
    }

    int TomiHTTP::GET(string address, TomiCookies* cookies)
    {
	string host, path;
	int port;

	headers.clear();

	address.erase(0,address.find_first_not_of(" \t"));

	if (string(address,0,7).compare(string("http://"))) {
	    throw runtime_error("Not a http URL!");
	}

	address.erase(0,7);

	unsigned int delim = address.find('/');
	if (delim == string::npos) {
	    address += '/';
	    delim = address.length() - 1;
	    if (delim < 1) {
		throw runtime_error("Not a http URL!");
	    }
	}

	path.assign(address,delim,address.length() - delim);
	host.assign(address,0,delim);
	address = host;

	delim = address.find(':');
	if (delim != string::npos) {
	    host.assign(address,0,delim);
	    port = atol(address.c_str() + delim + 1);
	}
	if (port < 1)
	    port = 80;

	connect(host,port);

	fprintf(stream,
		"GET %s HTTP/1.0\r\n"
		"User-Agent: TomiTCP/SVN\r\n"
		"Host: %s\r\n"
		"Accept: text/html, text/plain, text/*, */*\r\n",
		path.c_str(), host.c_str() );
	/* cookies... */
	if (cookies) {
	    string c;
	    cookies->http_getcookies(c);
	    fprintf(stream,"%s",c.c_str());
	}
	fprintf(stream,"\r\n");

	int ret;

	string line;
	if (getline(line)) {
	    chomp(line);
	    unsigned int space = line.find(' ');
	    if (space == string::npos) {
		throw runtime_error("parse error on HTTP response");
	    }
	    line.erase(0,space + 1);
	    space = line.find(' ');
	    if (space == string::npos) {
		space = line.length();
	    }
	    string rets(line,0,space);
	    ret = atol(rets.c_str());
	    if (!ret)
		throw runtime_error("parse error on HTTP response");
	} else {
	    throw runtime_error("zero sized HTTP reply");
	}

	while (getline(line)) {
	    chomp(line);
	    if (line.empty())
		break;
	    delim = line.find(':');
	    if (delim != string::npos) {
		string a(line,0,delim),b(line,delim+1);
		b.erase(0,b.find_first_not_of(" \t"));

		strtolower(a);
		headers[a] = b;

		if (cookies && a == "set-cookie") {
		    cookies->http_setcookie(b);
		}
	    }
	}
	
	return ret;
    }
}

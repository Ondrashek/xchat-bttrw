#pragma implementation
#include "http.h"
#include "str.h"
#include "version.h"
#include <cstdio>
#include <sstream>

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
	int port = 0;

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

	connect(host,uinttostr(port));

	if (port != 80) {
	    stringstream s;
	    s << host << ":" << port;
	    host = s.str();
	}

	fprintf(stream,
		"GET %s HTTP/1.0\r\n"
		"User-Agent: TomiTCP/" PACKAGE_VERSION "\r\n"
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

    int TomiHTTP::POST(string address, const string& data, TomiCookies* cookies)
    {
	string host, path;
	int port = 0;

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

	connect(host,uinttostr(port));

	if (port != 80) {
	    stringstream s;
	    s << host << ":" << port;
	    host = s.str();
	}

	fprintf(stream,
		"POST %s HTTP/1.0\r\n"
		"User-Agent: TomiTCP/" PACKAGE_VERSION "\r\n"
		"Host: %s\r\n"
		"Accept: text/html, text/plain, text/*, */*\r\n"
		"Content-type: application/x-www-form-urlencoded\r\n"
		"Content-length: %i\r\n",
		path.c_str(), host.c_str(), (int)data.length() );
	/* cookies... */
	if (cookies) {
	    string c;
	    cookies->http_getcookies(c);
	    fprintf(stream,"%s",c.c_str());
	}
	fprintf(stream,"\r\n");
	fprintf(stream,"%s",data.c_str());

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

    inline bool URLneedencode(unsigned char c)
    {
	if (c <= 0x1f || c >= 0x7f)
	    return 1;

	switch (c) {
	    case 0x24:
	    case 0x26:
	    case 0x2b:
	    case 0x2c:
	    case 0x2f:
	    case 0x3a:
	    case 0x3b:
	    case 0x3d:
	    case 0x3f:
	    case 0x40:
	    case 0x20:
	    case 0x22:
	    case 0x3c:
	    case 0x3e:
	    case 0x23:
	    case 0x25:
	    case 0x7b:
	    case 0x7d:
	    case 0x7c:
	    case 0x5c:
	    case 0x5e:
	    case 0x7e:
	    case 0x5b:
	    case 0x5d:
	    case 0x60:
		return 1;
	}

	return 0;
    }

    string TomiHTTP::URLencode(const string& s)
    {
	string out;

	for (string::const_iterator i = s.begin(); i != s.end(); i++) {
	    if (URLneedencode((unsigned char) *i)) {
		char c[5];
		sprintf(c,"%%%.2hhX",(unsigned char)*i);
		out += c;
	    } else {
		out += *i;
	    }
	}

	return out;
    }
}

#include "cookies.h"
#include <sstream>
#include <fstream>
#include <stdexcept>

namespace net {
    TomiCookies::TomiCookies()
    {
    }

    TomiCookies::~TomiCookies()
    {
    }

    void TomiCookies::http_setcookie(const string& cookie)
    {
	stringstream s(cookie);
	TomiCookie c;

	getline(s,c.name,'=');
	getline(s,c.value,';');

	cookies[c.name] = c;
    }

    void TomiCookies::http_getcookies(string& out)
    {
	for (cookies_t::iterator i = cookies.begin(); i != cookies.end();) {
	    if (i == cookies.begin()) {
		out += "Cookie: ";
	    }
	    out += i->first + "=" + i->second.value;

	    i++;

	    if (i == cookies.end()) {
		out += "\r\n";
	    } else {
		out += "; ";
	    }
	}
    }

    void TomiCookies::save(const string& file)
    {
	ofstream f(file.c_str());
	if (!f)
	    throw runtime_error("cannot open file for saving cookies");

	for (cookies_t::iterator i = cookies.begin(); i != cookies.end(); i++)
	    f << i->first << "=" << i->second.value << "\n";
    }

    void TomiCookies::load(const string& file)
    {
	ifstream f(file.c_str());
	if (!f)
	    throw runtime_error("cannot open file for loading cookies");

	string l;
	while (getline(f,l)) {
	    http_setcookie(l);
	}
    }
}

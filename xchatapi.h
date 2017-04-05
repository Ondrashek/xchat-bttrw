#ifndef _XCHATAPI_H_INCLUDED
#define _XCHATAPI_H_INCLUDED

#include <string>
#include <map>
#include "TomiTCP/cookies.h"

namespace net {
	using namespace std;

	class XChatAPI {
	private:
		string buffer;
	public:
		typedef map<string,string> headers_t;
		headers_t headers;
    long CB(const string url, const string post, const string json, TomiCookies* cookies);
		long GET(const string url, TomiCookies* cookies);
		long POST(const string url, const string post, TomiCookies* cookies);
		int parseresponse(TomiCookies* cookies);
		int getline(string& s, char delim = '\n');
		void close();
	};
}

#endif
#include "xchatapi.h"
#include <string>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "TomiTCP/str.h"

namespace net {
	static int xchatapi_curl_writer(char *data, size_t size, size_t nmemb, string *writerData)
	{
		if (writerData == NULL) {
			return 0;
		}

		writerData->append(data, size*nmemb);
		return size*nmemb;
	}

	long XChatAPI::GET(const string url, TomiCookies* cookies)
	{
		CURL *curl;
		CURLcode res;
		long http_code = 500;

		buffer.clear();
		curl_global_init(CURL_GLOBAL_DEFAULT);
		curl = curl_easy_init();
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_USERAGENT, "bttrw/2.0");
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
			curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, xchatapi_curl_writer);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
			res = curl_easy_perform(curl);

			if (res != CURLE_OK) {
				http_code = 500;
			} else {
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
			}

			curl_easy_cleanup(curl);
		}

		curl_global_cleanup();

		return http_code;
	}

	long XChatAPI::CB(const string url, const string post, const string json, TomiCookies* cookies)
	{
		CURL *curl;
		CURLcode res;
		long http_code = 500;

    struct curl_slist *slist1;

    slist1 = NULL;
    slist1 = curl_slist_append(slist1, "Content-Type: application/json");

		buffer.clear();
		curl_global_init(CURL_GLOBAL_DEFAULT);
		curl = curl_easy_init();
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_USERAGENT, "bttrw/2.0");
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist1);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
			curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, xchatapi_curl_writer);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
			res = curl_easy_perform(curl);

			if (res != CURLE_OK) {
				http_code = 500;
			} else {
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
			}

			curl_easy_cleanup(curl);
		}

		curl_global_cleanup();

		return http_code;
	}

	long XChatAPI::POST(const string url, const string post, TomiCookies* cookies)
	{
		CURL *curl;
		CURLcode res;
		long http_code = 500;

		buffer.clear();
		curl_global_init(CURL_GLOBAL_DEFAULT);
		curl = curl_easy_init();
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_USERAGENT, "bttrw/2.0");
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
			curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, xchatapi_curl_writer);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
			res = curl_easy_perform(curl);

			if (res != CURLE_OK) {
				http_code = 500;
			} else {
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
			}

			curl_easy_cleanup(curl);
		}

		curl_global_cleanup();

		return http_code;
	}

    int XChatAPI::parseresponse(TomiCookies* cookies) {
		int ret;
		string line;

		if (getline(line)) {
			chomp(line);

			string::size_type space = line.find(' ');
			if (space == string::npos) {
				return 200;
			}

			line.erase(0, space + 1);

			space = line.find(' ');
			if (space == string::npos) {
				space = line.length();
			}

			string rets(line, 0, space);
			ret = atoi(rets.c_str());
			if (!ret) {
				return 200;
			}
		} else {
			throw runtime_error("zero sized HTTP reply");
		}

		while (getline(line)) {
			chomp(line);
			if (line.empty())
				break;

			string::size_type delim = line.find(':');
			if (delim != string::npos) {
				string a(line, 0, delim), b(line, delim + 1);

				b.erase(0, b.find_first_not_of(" \t"));

				strtolower(a);
				headers[a] = b;

				if (cookies && a == "set-cookie") {
					cookies->http_setcookie(b);
				}
			}
		}

		return ret;
	}

	int XChatAPI::getline(string& s, char delim)
	{
		string ret;
		s.clear();

    if (buffer.length() == 0) { return 0; }

		string::size_type nl = buffer.find(delim);
		if (nl == string::npos) {
			nl = buffer.length();
		}
		s = buffer.substr(0, nl);
		buffer.erase(0, nl+1);

		return 1;

	}

	void XChatAPI::close()
	{ // empty
    //buffer.clear();
  }

}

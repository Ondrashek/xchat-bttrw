#include "irc.h"
#include "TomiTCP/str.h"

namespace std {
    /*
     * Parse IRC input line into source, command and parameters
     */
    void parsein(string buf, string& prefix, vector<string>& cmd)
    {
	const char *p = buf.c_str(), *e = buf.c_str() + buf.length();

	if (buf[0] == ':') {
	    p++;
	    char* d = strchr(p,' ');
	    if (d) {
		prefix = string(p,0,(d-p));
		p = d+1;
	    } else {
		prefix = p;
		p = e;
	    }
	}

	while (p < e) {
	    if (p[0] == ':') {
		cmd.push_back(string(p+1));
		p = e;
	    } else {
		char* d = strchr(p,' ');
		if (d) {
		    string s = string(p,0,(d-p));
		    wstrip(s);
		    if (s.length())
			cmd.push_back(s);
		    p = d+1;

		    if (cmd.size() == 1 && strtoupper_nr(cmd[0]) == "PASS" &&
			    p[0] != ':') {
			cmd.push_back(string(p));
			p = e;
		    }
		} else {
		    string s = string(p);
		    wstrip(s);
		    if (s.length())
			cmd.push_back(s);
		    p = e;
		}
	    }
	}
    }

    /*
     * Parse mode into array of mode with its parameter, if any
     */
    void parsemode(const vector<string>& cmd, vector<pair<string,string> >& mode)
    {
	char ch[2] = "?", t[2] = " ";
	vector<string>::const_iterator j = cmd.begin() + 1;

	for (string::const_iterator i = cmd[0].begin(); i != cmd[0].end(); i++) {
	    if (*i == '+' || *i == '-')
		ch[0] = *i;
	    else {
		t[0] = *i;

		if (j != cmd.end() && string("o").find(*i) != string::npos) {
		    mode.push_back(pair<string,string>(string(ch) + string(t), *j));
		    j++;
		} else {
		    mode.push_back(pair<string,string>(string(ch) + string(t), ""));
		}
	    }
	}
    }
}

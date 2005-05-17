#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "xchat.h"
#include "smiles.h"
#include "idle.h"
#include "TomiTCP/str.h"

namespace xchat {
    /*
     * Recode to client_charset
     */
    string XChat::recode_to_client(string s) {
	if (!client_charset.length())
	    return s;

	try {
	    /*
	     * Some exceptions...
	     */
	    if (client_charset != "UTF-8") {
		unsigned int a, pos = 0;

		while ((a = s.find("\xe2\x97\x8f", pos)) != string::npos) {
		    s.replace(a, 3, "*");
		    pos = a + 1;
		}
	    }

	    return recode(s, "UTF-8", client_charset);
	} catch (runtime_error er) {
	    EvError *e = new EvError;
	    e->s = er.what();
	    recvq_push(e);
	    return s;
	}
    }

    /*
     * Strip JavaScript escapes
     */
    void XChat::stripjsescapes(string &s)
    {
	unsigned int a, pos = 0;

	while ((a = s.find("\\", pos)) != string::npos) {
	    if (s.begin() + a + 2 <= s.end()) {
		s.erase(s.begin() + a);
		pos = a + 1;
	    } else
		break;
	}

	/*
	 * Second run - strip xchat double-escaped backslashes...
	 * (!!! this should be removed as soon as they fix that !!!)
	 */
	pos = 0;
	while ((a = s.find("\\\\", pos)) != string::npos) {
	    s.replace(a, 2, "\\");
	    pos = a + 1;
	}
    }

    /*
     * Strip HTML tags
     */
    void XChat::striphtml(string &s)
    {
	unsigned int a, b, lastbr = string::npos, pos = 0;

	while (((a = s.find('<', pos)) != string::npos) &&
		((b = string(s, a).find('>')) != string::npos)) {
	    int smile = 0;
	    bool br = 0;

	    static string pat = "<img src=\"//img.centrum.cz/xs/";
	    if (!s.compare(a, pat.length(), pat))
		smile = atol(string(s, a + pat.length()).c_str());
	    
	    static string pat2 = "<br";
	    if (!s.compare(a, pat2.length(), pat2))
		br = 1;

	    s.erase(s.begin() + a, s.begin() + a + b + 1);
	    pos = a;

	    if (smile) {
		s.insert(a, "*" + tostr(smile) + "*");
		pos += ("*" + tostr(smile) + "*").length();
	    }
	    if (br) {
		s.insert(a, " | ");
		lastbr = a;
		pos += 3;
	    }
	}

	/*
	 * Erase last br with a trailing whitespace, if there's no text.
	 */
	if (lastbr != string::npos &&
		s.find_first_not_of(" \f\n\r\t\v|", lastbr) == string::npos)
	    s.erase(lastbr);
    }

    /*
     * Strip and get date, if it is there.
     */
    void XChat::getdate(string &m, string &date)
    {
	string n = m, d;

	n.erase(0, n.find_first_not_of(" "));
	d = string(n, 0, n.find_first_of(" "));
	n.erase(0, n.find_first_of(" "));
	n.erase(0, n.find_first_not_of(" "));

	date = d;

	d.erase(0, d.find_first_of(":"));
	d.erase(d.find_last_of(":") + 1);
	if (d.length() > 1 && d[0] == ':' && d[d.length()-1] == ':')
	    m = n;
	else
	    date = "";
    }

    /*
     * Get nick (source) and, if given, target nick
     */
    void XChat::getnick(string &m, string &src, string &target)
    {
	string t = string(m, 0, m.find(": "));
	if (t.length() == m.length())
	    return;
	m.erase(0, t.length() + 2);
	if (m.length() && m[0] == ' ')
	    m.erase(m.begin());

	t.erase(0, t.find_first_not_of(" "));
	src = string(t, 0, t.find("->"));
	if (src.length() != t.length())
	    target = string(t, src.length() + 2);

	// strip [room]
	unsigned int a;
	if (src[0] == '[' && ((a = src.find(']')) != string::npos))
	    src.erase(0, a + 1);
	
	if (target[0] == '[' && ((a = target.find(']')) != string::npos))
	    target.erase(0, a + 1);

	if (src.find(' ') != string::npos || target.find(' ') != string::npos) {
	    src = "";
	    target = "";
	}
    }

    /*
     * Convert xchat smilies to human readable ones
     */
    void XChat::unsmilize(string &s)
    {
	unsigned int a, b;
	unsigned int pos = 0;

	while (((a = s.find('*', pos)) != string::npos) &&
		((b = s.find('*', a + 1)) != string::npos)) {
	    bool fail = 0;
	    for (string::iterator i = s.begin() + a + 1; i != s.begin() + b; i++)
		fail |= !isdigit(*i);

	    if (!fail) {
		int smile = atol(string(s.begin() + a + 1, s.begin() + b).c_str());
		if (smile < smiles_count && smiles[smile]) {
		    s.replace(a, b - a + 1, string("\002") + smiles[smile] + "\002");

		    int add = 2;
		    if (s[a + strlen(smiles[smile]) + 2] != ' ') {
			s.insert(a + strlen(smiles[smile]) + 2, " ");
			add++;
		    }
		    if (a > 0 && s[a - 1] != ' ') {
			s.insert(a, " ");
			add++;
		    }

		    pos = a + strlen(smiles[smile]) + add;
		} else {
		    pos = a + 1;
		}
	    } else {
		pos = a + 1;
	    }
	}
    }

    /*
     * Check for user joining a room
     */
    bool XChat::isjoin(room& r, string &m, string &src, int &sex)
    {
	unsigned int a,b;
	if ((a = m.find("Uživatel")) != string::npos &&
		(((b = m.find("vstoupil  do místnosti")) != string::npos) || 
		 ((b = m.find("vstoupila do místnosti")) != string::npos))) {
	    if (m.find("Uživatelka") != string::npos) {
		src = string(m, a + sizeof("Uživatelka ") - 1, b - a - sizeof("Uživatelka ") + 1);
		wstrip(src);
		r.nicklist[strtolower_nr(src)] = (struct x_nick){src, 0};
		sex = 0;
	    } else {
		src = string(m, a + sizeof("Uživatel ") - 1, b - a - sizeof("Uživatel ") + 1);
		wstrip(src);
		r.nicklist[strtolower_nr(src)] = (struct x_nick){src, 1};
		sex = 1;
	    }
	    return 1;
	}

	return 0;
    }

    /*
     * Check for user leaving a room
     */
    bool XChat::isleave(room& r, string &m, string &src, int &sex)
    {
	unsigned int a,b;
	if ((a = m.find("Uživatel")) != string::npos &&
		(((b = m.find("opustil  místnost")) != string::npos) || 
		 ((b = m.find("opustila místnost")) != string::npos))) {
	    if (m.find("Uživatelka") != string::npos) {
		src = string(m, a + sizeof("Uživatelka ") - 1, b - a - sizeof("Uživatelka ") + 1);
		wstrip(src);
	    } else {
		src = string(m, a + sizeof("Uživatel ") - 1, b - a - sizeof("Uživatel ") + 1);
		wstrip(src);
	    }

	    if (r.nicklist.find(strtolower_nr(src)) == r.nicklist.end())
		sex = 2;
	    else
		sex = r.nicklist[strtolower_nr(src)].sex;
	    r.nicklist.erase(strtolower_nr(src));
	    return 1;
	}

	return 0;
    }

    /*
     * Check for user being kicked from room
     */
    bool XChat::iskick(room& r, string &m, string &src, string &reason, string &who, int &sex)
    {
	unsigned int a,b;
	if ((a = m.find("Uživatel")) != string::npos &&
		(((b = m.find("byl  vyhozen")) != string::npos) ||
		(b = m.find("byla vyhozena")) != string::npos)) {
	    if (m.find("Uživatelka") != string::npos) {
		src = string(m, a + sizeof("Uživatelka ") - 1, b - a - sizeof("Uživatelka ") + 1);
		wstrip(src);
	    } else {
		src = string(m, a + sizeof("Uživatel ") - 1, b - a - sizeof("Uživatel ") + 1);
		wstrip(src);
	    }

	    if ((a = m.find("správcem")) != string::npos &&
		    (b = m.find("z místnosti")) != string::npos) {
		who = string(m, a + sizeof("správcem ") - 1, b - a - sizeof("správcem ") + 1);
		wstrip(who);
	    }
	    
	    if ((a = m.find("administrátorem")) != string::npos &&
		    (b = m.find("z místnosti")) != string::npos) {
		who = string(m, a + sizeof("administrátorem ") - 1,
			b - a - sizeof("administrátorem ") + 1);
		wstrip(who);
	    }

	    if ((a = m.find_last_of("(")) != string::npos &&
		    (b = m.find_last_of(")")) != string::npos) {
		reason = string(m, a + 1, b - a - 1);
	    }

	    if (r.nicklist.find(strtolower_nr(src)) == r.nicklist.end())
		sex = 2;
	    else
    		sex = r.nicklist[strtolower_nr(src)].sex;
	    r.nicklist.erase(strtolower_nr(src));
	    return 1;
	}

	return 0;
    }

    /*
     * Check for advertisement messages
     */
    bool XChat::isadvert(string &m, string &link)
    {
	static string pat = "<A TARGET=_blank HREF=\\\"/advert/advert.php";
	unsigned int pos;
	if ((pos = m.find(pat)) != string::npos) {
	    link = "http://xchat.centrum.cz/advert/advert.php" +
		string(m, pos + pat.length());
	    link.erase(link.find("\\\""));
	    return 1;
	}

	return 0;
    }

    /*
     * Should go as EvSysMsg instead of EvRoomSysMsg?
     */
    bool XChat::sysnoroom(string &m)
    {
	// Info
	static string pat1 = "INFO: ";
	if (!m.compare(0, pat1.length(), pat1)) {
	    return true;
	}

	// Info2
	static string pat2 = "Info2: ";
	if (!m.compare(0, pat2.length(), pat2)) {
	    return true;
	}

	static string pat3 = "není v žádné místnosti";
	if (m.find(pat3) != string::npos) {
	    return true;
	}
	
	static string pat4 = "neexistuje";
	if (m.find(pat4) != string::npos) {
	    return true;
	}

	return false;
    }

    /*
     * Check if this whisper is already in queue or in the secondary queue
     * with EvWhispers from previous refresh.
     */
    bool XChat::whisper_in_queue(string &m, string &src)
    {
	for (deque<recv_item>::iterator i = recvq.begin();
		i != recvq.end(); i++) {
	    EvWhisper *e;
	    if ((e = dynamic_cast<EvWhisper*>(i->e.get())))
		if (e->getsrc().nick == src && e->str() == m)
		    return true;
	}

	for (deque<recv_item>::iterator i = old_recvq.begin();
		i != old_recvq.end(); i++) {
	    EvWhisper *e;
	    if ((e = dynamic_cast<EvWhisper*>(i->e.get())))
		if (e->getsrc().nick == src && e->str() == m)
		    return true;
	}

	return false;
    }

    /*
     * Parse a line from xchat and push appropiate events to the recvq.
     */
    void XChat::recvq_parse_push(string m, room& r)
    {
	string link;
	bool advert = isadvert(m, link);

	x_nick *n;

	stripjsescapes(m);
	striphtml(m);
	striphtmlent(m);
	
	string date;
	getdate(m, date);

	if (advert) {
	    EvRoomAdvert *e = new EvRoomAdvert;
	    e->s = recode_to_client(m);
	    e->rid = r.rid;
	    e->link = link;
	    e->d = date;
	    recvq_push(e);
	    return;
	}

	string src, target;
	getnick(m, src, target);

	if (src.length()) {
	    unsmilize(m);

	    if (strtolower_nr(src) == "system" &&
		    strtolower_nr(target) == strtolower_nr(nick)) {
		if (checkidle(wstrip_nr(m))) {
		    EvRoomIdlerMsg *e = new EvRoomIdlerMsg;
		    e->s = recode_to_client(m);
		    e->rid = r.rid;
		    e->d = date;
		    recvq_push(e);
		} else if (sysnoroom(m)) {
		    EvSysMsg *e = new EvSysMsg;
		    e->s = recode_to_client(m);
		    e->d = date;
		    recvq_push(e);
		} else {
		    EvRoomSysMsg *e = new EvRoomSysMsg;
		    e->s = recode_to_client(m);
		    e->rid = r.rid;
		    e->d = date;
		    recvq_push(e);
		}
	    } else if (target.length() && strtolower_nr(src) != strtolower_nr(nick)) {
		EvWhisper *e = new EvWhisper;
		e->s = recode_to_client(m);
		e->src = (struct x_nick){ src, (n = findnick(src, 0))?n->sex:2 };
		e->target = (struct x_nick){ target, (n = findnick(target, 0))?n->sex:2 };
		e->d = date;
		if (!whisper_in_queue(e->s, e->src.nick))
		    recvq_push(e);
	    } else if (strtolower_nr(src) != strtolower_nr(nick)) {
		EvRoomMsg *e = new EvRoomMsg;
		e->s = recode_to_client(m);
		e->rid = r.rid;
		e->src = (struct x_nick){ src, (n = findnick(src, 0))?n->sex:2 };
		e->d = date;
		recvq_push(e);
	    }
	} else {
	    int sex;
	    string reason, who;

	    if (isjoin(r, m, src, sex)) {
		EvRoomJoin *e = new EvRoomJoin;
		e->s = recode_to_client(m);
		e->rid = r.rid;
		e->src = (struct x_nick){ src, sex };
		e->d = date;
		recvq_push(e);
	    } else if (isleave(r, m, src, sex)) {
		EvRoomLeave *e = new EvRoomLeave;
		e->s = recode_to_client(m);
		e->rid = r.rid;
		e->src = (struct x_nick){ src, sex };
		e->d = date;
		recvq_push(e);
	    } else if (iskick(r, m, src, reason, who, sex)) {
		if (who.length()) {
		    EvRoomKick *e = new EvRoomKick;
		    e->s = recode_to_client(m);
		    e->rid = r.rid;
		    e->src = (struct x_nick){ who, (n = findnick(who, 0))?n->sex:2 };
		    e->target = (struct x_nick){ src, sex };
		    e->reason = recode_to_client(reason);
		    e->d = date;
		    recvq_push(e);
		} else {
		    EvRoomLeave *e = new EvRoomLeave;
		    e->s = recode_to_client(m);
		    e->rid = r.rid;
		    e->src = (struct x_nick){ src, sex };
		    e->reason = recode_to_client(reason);
		    e->d = date;
		    recvq_push(e);
		}
	    } else {
		EvRoomSysText *e = new EvRoomSysText;
		e->s = recode_to_client(m);
		e->rid = r.rid;
		e->d = date;
		recvq_push(e);
	    }
	}
    }
}

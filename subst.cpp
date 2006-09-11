/*
 *	subst.cpp
 */
/*
 *	Copyright (C) 2006  K.Takata
 *
 *	You may distribute under the terms of either the GNU General Public
 *	License or the Artistic License, as specified in the perl_license.txt file.
 */
/*
 * Note:
 *	This file is based on K2Regexp.dll (bsubst.cpp)
 *	by Tatsuo Baba and Koyabu Kazuya (K2).
 */


#define _CRT_SECURE_NO_DEPRECATE
#define WIN32_LEAN_AND_MEAN

#define _BREGEXP_

#include <windows.h>
#include <new>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <mbstring.h>
#include <oniguruma.h>
#include "bregexp.h"
//#include "global.h"
#include "bregonig.h"
#include "mem_vc6.h"
#include "dbgtrace.h"


unsigned long scan_oct(const char *start, int len, int *retlen);
unsigned long scan_hex(const char *start, int len, int *retlen);
//unsigned long scan_dec(const char *start, int len, int *retlen);



char *realloc_buf(char *buf, int oldlen, int newlen, char *msg)
{
	char *tp = new (std::nothrow) char[newlen];
	if (tp == NULL) {
		strcpy(msg,"out of space buf");
		delete [] buf;
		return NULL;
	}
	memcpy(tp, buf, oldlen);
	delete [] buf;
	return tp;
}


int subst_onig(bregonig *rx, char *target, char *targetstartp, char *targetendp,
		char *msg, BCallBack callback)
{
//TRACE0("subst_onig()\n");
	char *orig,*m,*c;
	char *s = target;
	int len = targetendp - target;
    char *strend = s + len;
    int maxiters = (strend - s) + 10;
	int iters = 0;
	int clen;
    orig = m = s;
	s = targetstartp;	// added by K2
    int once = !(rx->pmflags & PMf_GLOBAL);
	c = rx->prerepp;
	clen = rx->prerependp - c;
	// 
    if (regexec_onig(rx, s, strend, orig, 0,1,0,msg) <= 0)
		return 0;
	int blen = len + clen + SUBST_BUF_SIZE;
	char *buf = new (std::nothrow) char[blen];
	int copycnt = 0;
	if (buf == NULL) {
		strcpy(msg,"out of space buf");
		return 0;
	}
	// now ready to go
	int subst_count = 0;
	do {
		if (iters++ > maxiters) {
			delete [] buf;
			strcpy(msg,"Substitution loop");
			return 0;
		}
		m = rx->startp[0];
		len = m - s;
		if (blen <= copycnt + len) {
			blen += len + SUBST_BUF_SIZE;
			buf = realloc_buf(buf, copycnt, blen, msg);
			if (buf == NULL)
				return 0;
		}
		if (len) {
			memcpy(buf+copycnt, s, len);
			copycnt += len;
		}
		s = rx->endp[0];
		if (!(rx->pmflags & PMf_CONST)) {	// we have \digits or $& 
			//	ok start magic
			REPSTR *rep = rx->repstr;
//			ASSERT(rep);
			for (int i = 0;i < rep->count;i++) {
				// normal char
				int dlen = rep->dlen[i]; 
				if (rep->startp[i] && dlen) { 
					if (blen <= copycnt + dlen) {
						blen += dlen + SUBST_BUF_SIZE;
						buf = realloc_buf(buf, copycnt, blen, msg);
						if (buf == NULL)
							return 0;
					}
					memcpy(buf+copycnt, rep->startp[i], dlen);
					copycnt += dlen;
				}
				
				else if (dlen <= rx->nparens && rx->startp[dlen] && rx->endp[dlen]) {
					// \digits or $&
					len = rx->endp[dlen] - rx->startp[dlen];
					if (blen <= copycnt + len) {
						blen += len + SUBST_BUF_SIZE;
						buf = realloc_buf(buf, copycnt, blen, msg);
						if (buf == NULL)
							return 0;
					}
					memcpy(buf+copycnt, rx->startp[dlen], len);
					copycnt += len;
				}
			}
		} else {
			if (clen) {		// no special char
				if (blen <= copycnt + clen) {
					blen += clen + SUBST_BUF_SIZE;
					buf = realloc_buf(buf, copycnt, blen, msg);
					if (buf == NULL)
						return 0;
				}
				memcpy(buf+copycnt, c, clen);
				copycnt += clen;
			}

		}
		subst_count++;
		if (once)
			break;
		if (callback)
			if (!callback(CALLBACK_KIND_REPLACE, subst_count, s - orig))
				break;
	} while (regexec_onig(rx, s, strend, orig, s == m, 1,0,msg) > 0);
//	len = rx->subend - s;
	len = targetendp - s;	// ???
	if (blen <= copycnt + len) {
		buf = realloc_buf(buf, copycnt, blen + len + 1, msg);
		if (buf == NULL)
			return 0;
	}
	if (len) {
		memcpy(buf+copycnt, s, len);
		copycnt += len;
	}
	if (copycnt) {
		rx->outp = buf;
		rx->outendp = buf + copycnt;
		*(rx->outendp) = '\0';
	}
	else
		delete [] buf;
	
	return subst_count;
}




int set_repstr(REPSTR *repstr, int num,
		int *pcindex, char **pdst, char **polddst)
{
TRACE0("set_repstr\n");
	int cindex = *pcindex;
	char *dst = *pdst;
	char *olddst = *polddst;
	
	if (/*num > 0 &&*/ cindex >= repstr->count - 2) {
		int newcount = repstr->count + 10;
		char **p1 = new char*[newcount];
		memcpy(p1, repstr->startp, repstr->count * sizeof(char*));
		delete [] repstr->startp;
		repstr->startp = p1;
		int *p2 = new int[newcount];
		memcpy(p2, repstr->dlen, repstr->count * sizeof(int));
		delete [] repstr->dlen;
		repstr->dlen = p2;
		repstr->count = newcount;
	}
	
	if (dst - olddst <= 0) {
		repstr->dlen[cindex] = num;	// paren number
		repstr->startp[cindex++] = NULL;	// try later
	} else {
		repstr->startp[cindex] = olddst;
		repstr->dlen[cindex++] = dst - olddst;
		repstr->dlen[cindex] = num;
		repstr->startp[cindex++] = NULL;	// try later
	}
	olddst = dst;
	
	*pcindex = cindex;
	*pdst = dst;
	*polddst = olddst;
	return 0;
}


char *parse_digit(const char *str, REPSTR *repstr, int *pcindex,
		char **pdst, char **polddst)
{
TRACE0("parse_digit\n");
	char *s;
	int num = (int) strtoul(str, &s, 10);
	
	set_repstr(repstr, num, pcindex, pdst, polddst);
	
	return s;
}


REPSTR *compile_rep(bregonig *rx, const char *str, const char *strend)
{
TRACE0("compile_rep()\n");
	rx->pmflags |= PMf_CONST;	/* default */
	int len = strend - str;
	if (len < 2)				// no special char
		return NULL;
	register const char *p = str;
	register const char *pend = strend;
	
	REPSTR *repstr = NULL;
	try {
		repstr = new (len) REPSTR;
	//	memset(repstr, 0, len + sizeof(REPSTR));
		char *dst = repstr->data;
		repstr->init(20);		// default \digits count in string
		int cindex = 0;
		char ender, prvch;
		int numlen;
		char *olddst = dst;
		bool special = false;		// found special char
		while (p < pend) {
			if (*p != '\\' && *p != '$') {	// magic char ?
				if (iskanji(*p))			// no
					*dst++ = *p++;
				*dst++ = *p++;
				continue;
			}
			if (p+1 >= pend) {		// end of the pattern
				*dst++ = *p++;
				break;
			}
			
			prvch = *p++;
			if (prvch == '$') {
				switch (*p) {
				case '&':	// $&
			//	case '0':	// $0
					special = true;
					set_repstr(repstr, 0, &cindex, &dst, &olddst);
					p++;
					break;
			/*
				case '+':	// $+
					special = true;
					break;
				case '`':	// $`
					special = true;
					break;
				case '\'':	// $'
					special = true;
					break;
				case '^':	// $^N
					special = true;
					break;
			*/
				case '{':	// ${nn}
					special = true;
					if (!isDIGIT(*++p)) {
						// SYNTAX ERROR
					}
					p = parse_digit(p, repstr, &cindex, &dst, &olddst);
					if (*p == '}') {
						p++;
					} else {
						// SYNTAX ERROR
					}
					break;
				default:
					if (isDIGIT(*p) && *p != '0') {
						special = true;
						p = parse_digit(p, repstr, &cindex, &dst, &olddst);
					} else {
						*dst++ = prvch;
						*dst++ = *p++;
					}
					break;
				}
				continue;
			}
			
			
			// now prvch == '\\'
			
			special = true;
			if (isDIGIT(*p)) {
				if (*p == '0'
						/* || (isDIGIT(p[1]) && atoi(p) >= rx->nparens)*/) {
					// '\0nn'
					ender = (char) scan_oct(p, 3, &numlen);
					p += numlen;
				//	*dst++ = ender;
				} else {
				//	num = atoi(p);
				//	if (num > 9 && num >= rx->nparens) {
				//		*dst++ = *p++;
				//	} else {
						// \digit found
						p = parse_digit(p, repstr, &cindex, &dst, &olddst);
				//	}
				}
			} else {
				prvch = *p++;
				switch (prvch) {
				case 'n':
					ender = '\n';
					break;
				case 'r':
					ender = '\r';
					break;
				case 't':
					ender = '\t';
					break;
				case 'f':
					ender = '\f';
					break;
				case 'e':
					ender = '\033';
					break;
				case 'a':
					ender = '\a';
					break;
				case 'v':
					ender = '\v';
					break;
				case 'b':
					ender = '\b';
					break;
				case 'x':	// '\xHH', '\x{HH}'
					if (isXDIGIT(*p)) {		// '\xHH'
						ender = (char) scan_hex(p, 2, &numlen);
						p += numlen;
					}
					else if (*p == '{') {	// '\x{HH}'
						unsigned int code = scan_hex(p, 4, &numlen);
						p += numlen;
						if (*p != '}') {
							// SYNTAX ERROR
						} else {
							if (code > 0xff)
								*dst++ = code >> 16;
							ender = (char) code;
							p++;
						}
					}
					break;
				case 'c':	// '\cx'	(ex. '\c[' == Ctrl-[ == '\x1b')
					ender = *p++;
					if (ender == '\\')
						ender = *p++;
					ender = toupper((unsigned char) ender);
					ender ^= 64;
					break;
				default:	// '/', '\\' and the other char
					ender = prvch;
					break;
				}
				*dst++ = ender;
			}
		}
		if (!special) {		// no special char found
		//	delete [] repstr->startp;	// deleted by the deconstructor
		//	delete [] repstr->dlen;		// deleted by the deconstructor
			delete repstr;
			return NULL;
		}
		
		rx->pmflags &= ~PMf_CONST;	/* off known replacement string */
		
		
		if (dst - olddst > 0) {
			repstr->startp[cindex] = olddst;
			repstr->dlen[cindex++] = dst - olddst;
		}
		repstr->count = cindex;
		
		return repstr;
	}
	catch (std::exception& ex) {
TRACE0("out of space in compile_rep()\n");
		delete repstr;
		throw ex;
	}
}



unsigned long
scan_oct(const char *start, int len, int *retlen)
{
	register const char *s = start;
	register unsigned long retval = 0;
	
	while (len && *s >= '0' && *s <= '7') {
		retval <<= 3;
		retval |= *s++ - '0';
		len--;
	}
	*retlen = s - start;
	return retval;
}

//static char hexdigit[] = "0123456789abcdef0123456789ABCDEFx";
static char hexdigit[] = "0123456789abcdef0123456789ABCDEF";

unsigned long
scan_hex(const char *start, int len, int *retlen)
{
	register const char *s = start;
	register unsigned long retval = 0;
	char *tmp;
	
	while (len-- && *s && (tmp = strchr(hexdigit, *s))) {
		retval <<= 4;
		retval |= (tmp - hexdigit) & 15;
		s++;
	}
	*retlen = s - start;
	return retval;
}

/*
unsigned long
scan_dec(const char *start, int len, int *retlen)
{
	char *s;
	unsigned long retval;
	
	retval = strtoul(start, &s, 10);
	*retlen = s - start;
	return retval;
}
*/


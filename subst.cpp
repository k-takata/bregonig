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


char *bufcat(char *buf, int *copycnt, const char *src, int len,
		int *blen, int bufsize, char *msg)
{
	if (*blen <= *copycnt + len) {
		*blen += len + bufsize;
		char *tp = new (std::nothrow) char[*blen];
		if (tp == NULL) {
			strcpy(msg,"out of space buf");
			delete [] buf;
			return NULL;
		}
		memcpy(tp, buf, *copycnt);
		delete [] buf;
		buf = tp;
	}
	if (len) {
		memcpy(buf + *copycnt, src, len);
		*copycnt += len;
	}
	return buf;
}


int dec2oct(int dec)
{
	int i, j;
	if (dec > 377)	// 0377 == 0xFF
		return -1;
	i = dec % 10;
	if (i > 7)
		return -1;
	j = (dec / 10) % 10;
	if (j > 7)
		return -1;
	return i + j*8 + (dec/100)*64;
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
	bool once = !(rx->pmflags & PMf_GLOBAL);
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
		buf = bufcat(buf, &copycnt, s, len, &blen, SUBST_BUF_SIZE, msg);
		if (buf == NULL)
			return 0;
		s = rx->endp[0];
		if (!(rx->pmflags & PMf_CONST)) {	// we have \digits or $&
			//	ok start magic
			REPSTR *rep = rx->repstr;
//			ASSERT(rep);
			for (int i = 0; i < rep->count; i++) {
				int j;
				// normal char
				int dlen = rep->dlen[i];
				if (rep->startp[i] && ((int) rep->startp[i] > 1) && dlen) {
					buf = bufcat(buf, &copycnt, rep->startp[i], dlen, &blen,
							SUBST_BUF_SIZE, msg);
					if (buf == NULL)
						return 0;
				}
				
				else if (dlen <= rx->nparens && rx->startp[dlen] && rx->endp[dlen]) {
					// \digits, $digits or $&
					len = rx->endp[dlen] - rx->startp[dlen];
					buf = bufcat(buf, &copycnt, rx->startp[dlen], len, &blen,
							SUBST_BUF_SIZE, msg);
					if (buf == NULL)
						return 0;
				}
				
				else if ((10<=dlen && (j=dec2oct(dlen)) > 0)
						&& dlen > rx->nparens && ((int) rep->startp[i] == 1)) {
					// \nnn
					char ch = (char) j;
					buf = bufcat(buf, &copycnt, &ch, 1, &blen,
							SUBST_BUF_SIZE, msg);
					if (buf == NULL)
						return 0;
				}
			}
		} else {
			if (clen) {		// no special char
				buf = bufcat(buf, &copycnt, c, clen, &blen,
						SUBST_BUF_SIZE, msg);
				if (buf == NULL)
					return 0;
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
	buf = bufcat(buf, &copycnt, s, len, &blen, 1, msg);
	if (buf == NULL)
		return 0;
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
		int *pcindex, char *dst, char **polddst, bool bksl = false)
{
TRACE0("set_repstr\n");
	int cindex = *pcindex;
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
	
	if (dst - olddst > 0) {
		repstr->startp[cindex] = olddst;
		repstr->dlen[cindex++] = dst - olddst;
	}
	repstr->dlen[cindex] = num;		// paren number
	if (bksl) {
		repstr->startp[cindex] = (char *) 1;	// \digits
	} else {
		repstr->startp[cindex] = NULL;			// $digits
	}
	cindex++;
	
	olddst = dst;
	
	*pcindex = cindex;
	*polddst = olddst;
	return 0;
}


char *parse_digits(const char *str, REPSTR *repstr, int *pcindex,
		char *dst, char **polddst, bool bksl = false)
{
TRACE0("parse_digits\n");
	char *s;
	int num = (int) strtoul(str, &s, 10);
	
	set_repstr(repstr, num, pcindex, dst, polddst, bksl);
	
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
					set_repstr(repstr, 0, &cindex, dst, &olddst);
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
				case '{':	// ${nn}, ${name}
					special = true;
					++p;
					if (isDIGIT(*p)) {
						// ${nn}
						p = parse_digits(p, repstr, &cindex, dst, &olddst);
						if (*p == '}') {
							p++;
						} else {
							// SYNTAX ERROR
						}
					} else {
						// ${name}
						const char *q = p;
						while (q < pend && *q != '}')
							++q;
						if (*q == '}') {
							int num = onig_name_to_backref_number(rx->reg,
									(UChar*) p, (UChar*) q, NULL);
							if (num > 0)
								set_repstr(repstr, num, &cindex, dst, &olddst);
							p = ++q;
						} else {
							// SYNTAX ERROR
						}
					}
					break;
				default:
					if (isDIGIT(*p) && *p != '0') {		// $digits
						special = true;
						p = parse_digits(p, repstr, &cindex, dst, &olddst);
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
				if (*p == '0') {
					// '\0nn'
					ender = (char) scan_oct(p, 3, &numlen);
					p += numlen;
					*dst++ = ender;
				} else {
					// \digits found
					p = parse_digits(p, repstr, &cindex, dst, &olddst, true);
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
					if (ender == '\\')	// '\c\x' == '\cx'
						ender = *p++;
					ender = toupper((unsigned char) ender);
					ender ^= 64;
					break;
				case 'k':	// \k<name>
					if (*p++ == '<') {
						const char *q = p;
						while (q < pend && *q != '>')
							++q;
						if (*q == '>') {
							int num = onig_name_to_backref_number(rx->reg,
									(UChar*) p, (UChar*) q, NULL);
							if (num > 0)
								set_repstr(repstr, num, &cindex, dst, &olddst);
							p = ++q;
						} else {
							// SYNTAX ERROR
						}
						continue;
					} else {
						// SYNTAX ERROR
					}
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


/*
 *	subst.cpp
 */
/*
 *	Copyright (C) 2006-2007  K.Takata
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
//#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <mbstring.h>
#include <tchar.h>
#include <oniguruma.h>
#include "bregexp.h"
//#include "global.h"
#include "bregonig.h"
#include "mem_vc6.h"
#include "dbgtrace.h"


#ifdef UNICODE
using namespace unicode;
namespace unicode {
#else
using namespace ansi;
namespace ansi {
#endif

unsigned long scan_oct(const TCHAR *start, int len, int *retlen);
unsigned long scan_hex(const TCHAR *start, int len, int *retlen);
//unsigned long scan_dec(const TCHAR *start, int len, int *retlen);


TCHAR *bufcat(TCHAR *buf, int *copycnt, const TCHAR *src, int len,
		int *blen, int bufsize, char *msg)
{
	if (*blen <= *copycnt + len) {
		*blen += len + bufsize;
		TCHAR *tp = new (std::nothrow) TCHAR[*blen];
		if (tp == NULL) {
			strcpy(msg,"out of space buf");
			delete [] buf;
			return NULL;
		}
		memcpy(tp, buf, *copycnt * sizeof(TCHAR));
		delete [] buf;
		buf = tp;
	}
	if (len) {
		memcpy(buf + *copycnt, src, len * sizeof(TCHAR));
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


int subst_onig(bregonig *rx, TCHAR *target, TCHAR *targetstartp, TCHAR *targetendp,
		char *msg, BCallBack callback)
{
//TRACE0(_T("subst_onig()\n"));
	TCHAR *orig,*m,*c;
	TCHAR *s = target;
	int len = targetendp - target;
	TCHAR *strend = s + len;
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
	TCHAR *buf = new (std::nothrow) TCHAR[blen];
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
					TCHAR ch = (TCHAR) j;
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
		int *pcindex, TCHAR *dst, TCHAR **polddst, bool bksl = false)
{
TRACE0(_T("set_repstr\n"));
	int cindex = *pcindex;
	TCHAR *olddst = *polddst;
	
	if (/*num > 0 &&*/ cindex >= repstr->count - 2) {
		int newcount = repstr->count + 10;
		TCHAR **p1 = new TCHAR*[newcount];
		memcpy(p1, repstr->startp, repstr->count * sizeof(TCHAR*));
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
		repstr->startp[cindex] = (TCHAR *) 1;	// \digits
	} else {
		repstr->startp[cindex] = NULL;			// $digits
	}
	cindex++;
	
	olddst = dst;
	
	*pcindex = cindex;
	*polddst = olddst;
	return 0;
}


TCHAR *parse_digits(const TCHAR *str, REPSTR *repstr, int *pcindex,
		TCHAR *dst, TCHAR **polddst, int endch = 0, bool bksl = false)
{
TRACE0(_T("parse_digits\n"));
	TCHAR *s;
	int num = (int) _tcstoul(str, &s, 10);
	
	if (endch) {
		if (*s != endch)
			return NULL;
		++s;
	}
	set_repstr(repstr, num, pcindex, dst, polddst, bksl);
	
	return s;
}


const TCHAR *parse_groupname(bregonig *rx, const TCHAR *str, const TCHAR *strend,
		REPSTR *repstr, int *pcindex, TCHAR *dst, TCHAR **polddst, TCHAR endch)
{
	const TCHAR *q = str;
	while (q < strend && *q != endch) {
		++q;
	}
	if (*q != endch) {
		return NULL;
	}
	int num = onig_name_to_backref_number(rx->reg,
			(UChar*) str, (UChar*) q, NULL);
	if (num > 0)
		set_repstr(repstr, num, pcindex, dst, polddst);
	return q+1;
}


REPSTR *compile_rep(bregonig *rx, const TCHAR *str, const TCHAR *strend)
{
TRACE0(_T("compile_rep()\n"));
	rx->pmflags |= PMf_CONST;	/* default */
	int len = strend - str;
	if (len < 2)				// no special char
		return NULL;
	register const TCHAR *p = str;
	register const TCHAR *pend = strend;
	
	REPSTR *repstr = NULL;
	try {
		repstr = new (len) REPSTR;
	//	memset(repstr, 0, len + sizeof(REPSTR));
		TCHAR *dst = repstr->data;
		repstr->init(20);		// default \digits count in string
		int cindex = 0;
		TCHAR ender, prvch;
		int numlen;
		TCHAR *olddst = dst;
		bool special = false;		// found special char
		while (p < pend) {
			if (*p != '\\' && *p != '$') {	// magic char ?
#ifndef UNICODE
				if (iskanji(*p))			// no
					*dst++ = *p++;
#endif
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
						TCHAR *q = parse_digits(p, repstr, &cindex, dst, &olddst, '}');
						if (q != NULL) {
							p = q;
						} else {
							// SYNTAX ERROR
						//	throw std::invalid_argument("} not found");
							*dst++ = prvch;
							--p;
						}
					} else {
						// ${name}
						const TCHAR *q = parse_groupname(rx, p, pend, repstr,
								&cindex, dst, &olddst, '}');
						if (q != NULL) {
							p = q;
						} else {
							// SYNTAX ERROR
							*dst++ = prvch;
							--p;
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
					ender = (TCHAR) scan_oct(p, 3, &numlen);
					p += numlen;
					*dst++ = ender;
				} else {
					// \digits found
					p = parse_digits(p, repstr, &cindex, dst, &olddst, 0, true);
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
						ender = (TCHAR) scan_hex(p, 2, &numlen);
						p += numlen;
					}
					else {
						const TCHAR *q = p;
						if (*p == '{') {	// '\x{HH}'
							unsigned int code = scan_hex(++p, 8, &numlen);
							p += numlen;
							if (*p == '}') {
#ifdef UNICODE
								if (code > 0xffff) {	// Surrogate Pair
									unsigned int c = code - 0x10000;
									*dst++ = (c >> 10) | 0xd800;
									code = (c & 0x3ff) | 0xdc00;
								}
#else
								if (code > 0xff) {
									*dst++ = code >> 8;
								}
#endif
								ender = (TCHAR) code;
								p++;
								break;
							}
						}
						// SYNTAX ERROR
						ender = 'x';
						p = q;
					}
					break;
				case 'c':	// '\cx'	(ex. '\c[' == Ctrl-[ == '\x1b')
					ender = *p++;
					if (ender == '\\')	// '\c\x' == '\cx'
						ender = *p++;
					ender = toupper((TBYTE) ender);
					ender ^= 64;
					break;
				case 'k':	// \k<name>, \k'name'
					if (*p == '<' || *p == '\'') {
						const TCHAR endch = (*p == '<') ? '>' : '\'';
						const TCHAR *q = parse_groupname(rx, p+1, pend, repstr,
								&cindex, dst, &olddst, endch);
						if (q != NULL) {
							p = q;
							continue;
						}
					}
					// SYNTAX ERROR
					ender = prvch;
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
TRACE0(_T("out of space in compile_rep()\n"));
		delete repstr;
		throw;
	}
}



unsigned long
scan_oct(const TCHAR *start, int len, int *retlen)
{
	register const TCHAR *s = start;
	register unsigned long retval = 0;
	
	while (len && *s >= '0' && *s <= '7') {
		retval <<= 3;
		retval |= *s++ - '0';
		len--;
	}
	*retlen = s - start;
	return retval;
}

//static TCHAR hexdigit[] = "0123456789abcdef0123456789ABCDEFx";
static TCHAR hexdigit[] = _T("0123456789abcdef0123456789ABCDEF");

unsigned long
scan_hex(const TCHAR *start, int len, int *retlen)
{
	register const TCHAR *s = start;
	register unsigned long retval = 0;
	TCHAR *tmp;
	
	while (len-- && *s && (tmp = _tcschr(hexdigit, *s))) {
		retval <<= 4;
		retval |= (tmp - hexdigit) & 15;
		s++;
	}
	*retlen = s - start;
	return retval;
}

/*
unsigned long
scan_dec(const TCHAR *start, int len, int *retlen)
{
	TCHAR *s;
	unsigned long retval;
	
	retval = _tcstoul(start, &s, 10);
	*retlen = s - start;
	return retval;
}
*/

} // namespace

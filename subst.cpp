/*
 *	subst.cpp
 */
/*
 *	Copyright (C) 2006-2011  K.Takata
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


int dec2oct(int dec);
int get_right_most_captured_num(OnigRegion *region);

#ifndef UNICODE
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

int get_right_most_captured_num(OnigRegion *region)
{
	for (int i = region->num_regs - 1; i > 0; i--) {
		if (region->beg[i] != ONIG_REGION_NOTPOS)
			return i;
	}
	return -1;
}
#endif


using namespace BREGONIG_NS;
namespace BREGONIG_NS {

enum casetype {
	CASE_NONE, CASE_UPPER, CASE_LOWER
};


unsigned long scan_oct(const TCHAR *start, int len, int *retlen);
unsigned long scan_hex(const TCHAR *start, int len, int *retlen);
//unsigned long scan_dec(const TCHAR *start, int len, int *retlen);


TCHAR *bufcat(TCHAR *buf, ptrdiff_t *copycnt, const TCHAR *src, ptrdiff_t len,
		ptrdiff_t *blen, int bufsize = SUBST_BUF_SIZE)
{
	if (*blen <= *copycnt + len) {
		*blen += len + bufsize;
		TCHAR *tp = new (std::nothrow) TCHAR[*blen];
		if (tp == NULL) {
		//	asc2tcs(msg, "out of space buf", BREGEXP_MAX_ERROR_MESSAGE_LEN);
			delete [] buf;
			throw std::bad_alloc();
		//	return NULL;
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



int subst_onig(bregonig *rx, const TCHAR *target,
		const TCHAR *targetstartp, const TCHAR *targetendp,
		TCHAR *msg, BCallBack callback)
{
TRACE0(_T("subst_onig()\n"));
	const TCHAR *orig,*m,*c;
	const TCHAR *s = target;
	ptrdiff_t len = targetendp - target;
	const TCHAR *strend = s + len;
	ptrdiff_t maxiters = (strend - s) + 10;
	ptrdiff_t iters = 0;
	ptrdiff_t clen;
	orig = m = s;
	s = targetstartp;	// added by K2
	bool once = !(rx->pmflags & PMf_GLOBAL);
	c = rx->prerepp;
	clen = rx->prerependp - c;
	// 
	if (regexec_onig(rx, s, strend, orig, 0,1,0,msg) <= 0)
		return 0;
	try {
		ptrdiff_t blen = len + clen + SUBST_BUF_SIZE;
		TCHAR *buf = new TCHAR[blen];
		ptrdiff_t copycnt = 0;
		// now ready to go
		int subst_count = 0;
		do {
			if (iters++ > maxiters) {
				delete [] buf;
TRACE0(_T("Substitution loop\n"));
				asc2tcs(msg, "Substitution loop", BREGEXP_MAX_ERROR_MESSAGE_LEN);
				return 0;
			}
			m = rx->startp[0];
			len = m - s;
			buf = bufcat(buf, &copycnt, s, len, &blen);
			s = rx->endp[0];
			if (!(rx->pmflags & PMf_CONST)) {	// we have \digits or $&
				//	ok start magic
				REPSTR *rep = rx->repstr;
	//			ASSERT(rep);
				for (int i = 0; i < rep->count; i++) {
					int j;
					// normal char
					ptrdiff_t dlen = rep->dlen[i];
					if (rep->is_normal_string(i) && dlen) {
						buf = bufcat(buf, &copycnt, rep->startp[i], dlen, &blen);
					}
					
					else if (0 <= dlen && dlen <= rx->nparens
							&& rx->startp[dlen] && rx->endp[dlen]) {
						// \digits, $digits or $&
						len = rx->endp[dlen] - rx->startp[dlen];
						buf = bufcat(buf, &copycnt, rx->startp[dlen], len, &blen);
					}
					
					else if (dlen == -1
							&& (j=get_right_most_captured_num(rx->region)) > 0) {
						// $+
						len = rx->endp[j] - rx->startp[j];
						buf = bufcat(buf, &copycnt, rx->startp[j], len, &blen);
					}
					
					else if ((10<=dlen && (j=dec2oct((int) dlen)) > 0)
							&& dlen > rx->nparens && rep->is_backslash(i)) {
						// \nnn
						TCHAR ch = (TCHAR) j;
						buf = bufcat(buf, &copycnt, &ch, 1, &blen);
					}
				}
			} else {
				if (clen) {		// no special char
					buf = bufcat(buf, &copycnt, c, clen, &blen);
				}
			}
			subst_count++;
			if (once)
				break;
			if (callback)
				if (!callback(CALLBACK_KIND_REPLACE, subst_count, s - orig))
					break;
		} while (regexec_onig(rx, s, strend, orig, s == m, 1,0,msg) > 0);
		len = targetendp - s;
		buf = bufcat(buf, &copycnt, s, len, &blen, 1);
		if (copycnt) {
			rx->outp = buf;
			rx->outendp = buf + copycnt;
			*(rx->outendp) = '\0';
		}
		else
			delete [] buf;
		
TRACE1(_T("subst_count: %d\n"), subst_count);
		return subst_count;
	}
	catch (std::bad_alloc& /*ex*/) {
TRACE0(_T("out of space in subst_onig()\n"));
		asc2tcs(msg, "out of space buf", BREGEXP_MAX_ERROR_MESSAGE_LEN);
		return 0;
	}
}




int set_repstr(REPSTR *repstr, int num,
		int *pcindex, TCHAR *dst, TCHAR **polddst, bool backslash = false)
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
		ptrdiff_t *p2 = new ptrdiff_t[newcount];
		memcpy(p2, repstr->dlen, repstr->count * sizeof(ptrdiff_t));
		delete [] repstr->dlen;
		repstr->dlen = p2;
		repstr->count = newcount;
	}
	
	if (dst - olddst > 0) {
		repstr->startp[cindex] = olddst;
		repstr->dlen[cindex++] = dst - olddst;
	}
	repstr->dlen[cindex] = num;		// paren number
	if (backslash) {
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


const TCHAR *parse_digits(const TCHAR *str, REPSTR *repstr, int *pcindex,
		TCHAR *dst, TCHAR **polddst, bool backslash = false)
{
TRACE0(_T("parse_digits\n"));
	TCHAR *s;
	TCHAR endch = 0;
	if (*str == '{') {
		++str;
		endch = '}';
	}
	int num = (int) _tcstoul(str, &s, 10);
	
	if (endch) {
		if (*s != endch)
			return NULL;	// SYNTAX ERROR
		++s;
	}
	set_repstr(repstr, num, pcindex, dst, polddst, backslash);
	
	return s;
}


const TCHAR *parse_groupname(bregonig *rx, const TCHAR *str, const TCHAR *strend,
		REPSTR *repstr, int *pcindex, TCHAR *dst, TCHAR **polddst,
		bool bracket = false)
{
	TCHAR endch;
	switch (*str++) {
	case '<':	endch = '>';	break;
	case '{':	endch = '}';	break;
	case '\'':	endch = '\'';	break;
	default:
		return NULL;
	}
	const TCHAR *q = str;
	while (q < strend && *q != endch) {
		++q;
	}
	if (*q != endch) {
		return NULL;	// SYNTAX ERROR
	}
	int arrnum = 0;
	const TCHAR *nameend = q;
	if (bracket) {	// [n]
		if (q[1] != '[')
			return NULL;	// SYNTAX ERROR
		arrnum = (int) _tcstol(q+2, (TCHAR**) &q, 10);
		if (*q != ']' || q >= strend)
			return NULL;	// SYNTAX ERROR
	}
	int *num_list;
	int num = onig_name_to_group_numbers(rx->reg,
			(UChar*) str, (UChar*) nameend, &num_list);
#ifdef NAMEGROUP_RIGHTMOST
	int n = num - 1;
#else
	int n = 0;
#endif
	if (bracket) {
		n = arrnum;
		if (arrnum < 0) {
			n += num;
		}
	}
	if ((num > 0) && (0 <= n || n < num))
		set_repstr(repstr, num_list[n], pcindex, dst, polddst);	// leftmost group-num
	return q+1;
}


REPSTR *compile_rep(bregonig *rx, const TCHAR *str, const TCHAR *strend)
{
TRACE0(_T("compile_rep()\n"));
	rx->pmflags |= PMf_CONST;	/* default */
	ptrdiff_t len = strend - str;
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
		casetype nextchcase = CASE_NONE;
		casetype chcase = CASE_NONE;
		OnigEncoding enc = onig_get_encoding(rx->reg);
		while (p < pend) {
			if (*p != '\\' && *p != '$') {	// magic char ?
				// copy one char
				int len = ONIGENC_MBC_ENC_LEN(enc, (UChar*) p);
				memcpy(dst, p, len);
				p += len / sizeof(TCHAR);
				dst += len / sizeof(TCHAR);
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
				case '+':	// $+, $+{name}
					special = true;
					if (p[1] == '{') {	// $+{name}
						const TCHAR *q = parse_groupname(rx, p+1, pend, repstr,
								&cindex, dst, &olddst);
						if (q != NULL) {
							p = q;
						} else {
							// SYNTAX ERROR
							*dst++ = prvch;
						}
					} else {			// $+
						set_repstr(repstr, -1, &cindex, dst, &olddst);
						p++;
					}
					break;
				case '-':	// $-{name}[n]
					special = true;
					if (p[1] == '{') {
						const TCHAR *q = parse_groupname(rx, p+1, pend, repstr,
								&cindex, dst, &olddst, true);
						if (q != NULL) {
							p = q;
							continue;
						}
					}
					// SYNTAX ERROR
					*dst++ = prvch;
					break;
			/*
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
					if (isDIGIT(p[1])) {
						// ${nn}
						const TCHAR *q = parse_digits(p, repstr, &cindex, dst, &olddst);
						if (q != NULL) {
							p = q;
						} else {
							// SYNTAX ERROR
						//	throw std::invalid_argument("} not found");
							*dst++ = prvch;
						}
					} else {
						// ${name}
						const TCHAR *q = parse_groupname(rx, p, pend, repstr,
								&cindex, dst, &olddst);
						if (q != NULL) {
							p = q;
						} else {
							// SYNTAX ERROR
							*dst++ = prvch;
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
#ifdef USE_VTAB
				case 'v':
					ender = '\v';
					break;
#endif
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
						if (*q == '{') {	// '\x{HH}'
							unsigned int code = scan_hex(++q, 8, &numlen);
							q += numlen;
							if (*q == '}') {
								int len = ONIGENC_CODE_TO_MBC(
										enc, code, (UChar*) dst)
											/ sizeof(TCHAR);
								dst += len - 1;
								ender = *dst;
								p = q+1;
								break;
							}
						}
						// SYNTAX ERROR
						ender = prvch;
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
						const TCHAR *q = parse_groupname(rx, p, pend, repstr,
								&cindex, dst, &olddst);
						if (q != NULL) {
							p = q;
							continue;
						}
					}
					// SYNTAX ERROR
					ender = prvch;
					break;
			/*
				case 'l':	// lower next
					nextchcase = CASE_LOWER;
					continue;
				case 'u':	// upper next
					nextchcase = CASE_UPPER;
					continue;
				case 'L':	// lower till \E
					chcase = CASE_LOWER;
					continue;
				case 'U':	// upper till \E
					chcase = CASE_UPPER;
					continue;
				case 'Q':	// quote
					continue;
				case 'E':	// end of \L/\U
					chcase = CASE_NONE;
					continue;
			*/
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
	catch (std::exception& /*ex*/) {
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
	*retlen = (int) (s - start);
	return retval;
}

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
	*retlen = (int) (s - start);
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

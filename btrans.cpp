//
// btrans.cc
//
//   translate front-end
////////////////////////////////////////////////////////////////////////////////
//  1999.11.24   update by Tatsuo Baba
//  2006.08.30   update by K.Takata
//
//  You may distribute under the terms of either the GNU General Public
//  License or the Artistic License, as specified in the perl_license.txt file.
////////////////////////////////////////////////////////////////////////////////

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
#include <tchar.h>
#include "bregexp.h"
//#include "global.h"
#include "bregonig.h"
#include "mem_vc6.h"
#include "dbgtrace.h"

#define KANJI
//#include "global.h"
#include "sv.h"
//#include "intreg.h"


using namespace BREGONIG_NS;
namespace BREGONIG_NS {

int trans(bregonig *rx, TCHAR *target, TCHAR *targetendp, TCHAR *msg);

bregonig *trcomp(TCHAR *res, TCHAR *resend, TCHAR *rp, TCHAR *rpend,
		int flag, TCHAR *msg);
static SV *cvchar(const TCHAR *str, const TCHAR *strend);
static TWORD specchar(const TCHAR* p,int *next);
void sv_catkanji(SV *sv,U32 tch);


/*
static inline int is_char_pair(const TBYTE *s)
{
#ifdef UNICODE
	if (((s[0] & 0xfc00) == 0xd800) && ((s[1] & 0xfc00) == 0xdc00)) {
		return true;
	}
	return false;
#else
	return iskanji(*s);
#endif
}

static inline TWORD get_codepoint(const TBYTE *s)
{
#ifdef UNICODE
	return (((s[0] - 0xd800) << 10) | (s[1] - 0xdc00)) + 0x10000;
#else
	return ((U8)s[0] <<8) | (U8)s[1];
#endif
}
*/


// compile translate string
bregonig *trcomp(const TCHAR *str, const TCHAR *strend,
		const TCHAR *rp, const TCHAR *rpend,
		int flag, TCHAR *msg)
{
	int slen = strend - str;
	int rlen = rpend - rp;
	if (slen < 1)
		return NULL;
	register const TCHAR *p = str;
	register const TCHAR *pend = strend;

//	bregonig *rx = (bregonig*) new char[sizeof(bregonig)];
	bregonig *rx = new (std::nothrow) bregonig();
	if (rx == NULL) {
		asc2tcs(msg,"out of space trcomp");
		return NULL;
	}

//	memset(rx,0,sizeof(bregonig));
	rx->pmflags = flag | PMf_TRANSLATE;

	SV *tstr = NULL;
	SV *rstr = NULL;

	/* the even index holds the t-char(in 2byte), and the odd index
	   holds the r-char(in 2 byte) if t-char is to be removed, then
	   r-char is -2. */
	register TWORD *tbl = NULL;

	try {
		tstr = cvchar(str,strend);
		rstr = cvchar(rp,rpend);
	
	
		int tlen = SvCUR(tstr);
		rlen = SvCUR(rstr);
		register TBYTE *t = (TBYTE*)SvPVX(tstr);
		register TBYTE *r = (TBYTE*)SvPVX(rstr);
	
		register int i; /* indexes t */
		register int j; /* indexes j */
		register int k; /* indexes tbl */
		int lastrch = -1;
		int tbl_size = 256;
		
		int del_char;
		int complement;
		int kanji;
	
		tbl = new TWORD[tbl_size];
	
		
		complement	= rx->pmflags & PMf_TRANS_COMPLEMENT;
		del_char	= rx->pmflags & PMf_TRANS_DELETE;
#ifdef UNICODE
		kanji	= 1;
#else
		kanji	= rx->pmflags & PMf_KANJI;
#endif
	
		for (i = 0, j = 0, k = 0; i < tlen; ) {
			U32 tch, rch;
			if (kanji && i < tlen-1 && is_char_pair(t+i)) {
				tch = get_codepoint(t+i);
				i+=2;
			} else {
				tch = (TBYTE)t[i];
				i++;
			}
			if (j >= rlen) {
				if (del_char) rch = (unsigned)-2;
				else rch = lastrch;
			} else {
				if (kanji && j < rlen-1 && is_char_pair(r+j)) {
					rch = get_codepoint(r+j);
					j += 2;
				} else {
					rch = (TBYTE)r[j];
					j++;
				}
				lastrch = rch;
			}
			if (k >= tbl_size) {
				TWORD *tp = new TWORD[tbl_size+256];
				memcpy(tp,tbl,tbl_size * sizeof(TWORD));
				delete [] tbl;
				tbl = tp;
				tbl_size += 256;
			}
			tbl[k++] = tch;
			tbl[k++] = rch;
		}
		if (k >= tbl_size) {
			TWORD *tp = new TWORD[tbl_size+4];
			memcpy(tp,tbl,tbl_size * sizeof(TWORD));
			delete [] tbl;
			tbl = tp;
			tbl_size += 4;
		}
		/* mark the end */
		tbl[k++] = (TWORD)-1;
		tbl[k++] = (TWORD)-1;
		rx->transtblp = (TCHAR*)tbl;
		sv_free(tstr);
		sv_free(rstr);
	
		return rx;
	}
	catch (std::exception& ex) {
TRACE0(_T("out of space in trcomp()\n"));
		if (tstr)
			sv_free(tstr);
		if (rstr)
			sv_free(rstr);
		delete tbl;
		delete rx;
		asc2tcs(msg, ex.what());
		return NULL;
	}
}

static SV *cvchar(const TCHAR *str, const TCHAR *strend)
{
	int next;
	TWORD ender;
	TWORD lastch = 0;
	int len = strend - str;
	const TCHAR *p = str;
	const TCHAR *pend = strend;
	SV *dst = newSVpv(_T(""),0);
	while (p < pend) {
		if (*p != '\\' && *p != '-') {	// no magic char ?
			lastch = *p;
			if (is_char_pair((TBYTE*)p)) {	// Surrogate Pair or kanji ?
				lastch = get_codepoint((TBYTE*)p);
				sv_catkanji(dst,lastch);
				p++;
			} else
				sv_catpvn(dst,p,1);
			p++;
			continue;
		}
		p++;
		if (p >= pend) {
			sv_catpvn(dst,p-1,1);
			break;
		}
		if (p[-1] == '-') {		// - ?
			const TCHAR* tp = p -1;
			TWORD toch;
			if (is_char_pair((TBYTE*)p)) {	// Surrogate Pair or kanji ?
				toch = get_codepoint((TBYTE*)p);
				p += 2;
			} else {
				toch = *p++;
				if (p[-1] == '\\') {
					if (p +1 >= pend) {
						sv_catpvn(dst,p-1,2);
						break;
					}
					toch = specchar(p,&next);
					p += next;
				}
			}
					
			if (lastch >= toch || toch - lastch > 255) {
				sv_catpvn(dst,tp,p - tp);
				continue;
			}
//			int clen = toch > 255 ? 2 :1;
			for (lastch++;toch >= lastch;lastch++) {
/*
				if (clen ==1)
					sv_catpvn(dst,(TCHAR*)&lastch,clen);
				else {
					TCHAR ch[2];
					ch[0] = lastch >> 8;
					ch[1] = (TCHAR)lastch;
					sv_catpvn(dst,ch,clen);
				}
*/
				sv_catkanji(dst,lastch);
			}
			lastch--;
			continue;
		}

		if (*p == '\\') {
			sv_catpvn(dst,p,1);
			p++;
			continue;
		}
		ender = specchar(p,&next);
//		sv_catpvn(dst,&ender,1);
		sv_catkanji(dst,ender);
		lastch = ender;
		p += next;
	}


	return dst;
}


static TWORD specchar(const TCHAR *p, int *next)
{
	TWORD ender;
	int numlen = 0;
	switch (*p++) {
	case '/':
		ender = '/';
		break;
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
//		ender = '\007';
		ender = '\a';
		break;
#ifdef USE_VTAB
	case 'v':			// added by K.Takata
		ender = '\v';
		break;
#endif
	case 'b':			// added by K.Takata
		ender = '\b';
		break;
	case 'x':
		if (isXDIGIT(*p)) {		// '\xHH'
			ender = (TBYTE)scan_hex(p, 2, &numlen);
		}
		else {
			if (*p == '{') {	// '\x{HH}'
				TWORD code = (TWORD)scan_hex(++p, 8, &numlen);
				if (p[numlen] == '}') {
					ender = code;
					numlen += 2;
					break;
				}
			}
			// SYNTAX ERROR
			ender = p[-1];
			numlen = 0;
		}
		break;
	case 'c':
		ender = *p++;
		if (isLOWER(ender))
			ender = toUPPER(ender);
		ender ^= 64;
		++numlen;
		break;
	case '0':	case '1':	case '2':	case '3':
	case '4':	case '5':	case '6':	case '7':
		--p;
		ender = (TCHAR)scan_oct(p, 3, &numlen);
		--numlen;
		break;
	case '\0':
		/* FALL THROUGH */
	default:
		ender = p[-1];
	}
	*next = numlen + 1;
	return ender;
}



int trans(bregonig *rx, TCHAR *target, TCHAR *targetendp, TCHAR *msg)
{
	register short *tbl;
	register TBYTE *s;
	register TBYTE *send;
	register int matches = 0;
	register int squash = rx->pmflags & PMf_TRANS_SQUASH;
	int len;
	U32 last_rch;
	// This variable need, doesn't it?
	// replase sv ;
	SV *dest_sv = NULL;
	try {
		dest_sv = newSVpv(_T(""),0);
	
		int del_char = rx->pmflags & PMf_TRANS_DELETE;
		int complement = rx->pmflags & PMf_TRANS_COMPLEMENT;
#ifdef UNICODE
		int kanji = 1;
#else
		int kanji = rx->pmflags & PMf_KANJI;
#endif
	
	
		tbl = (short*)rx->transtblp;
		s = (TBYTE*)target;
		len = targetendp - target;
		if (!len)
			return 0;
		send = s + len;
		while (s < send) {
			U32 tch, rch;
			TBYTE *next_s;
			TWORD *tp;
			int matched;
			if (kanji && s < send-1 && is_char_pair(s)) {
				tch = get_codepoint(s);
				next_s = s+2;
			} else {
				tch = *(TBYTE*)s;
				next_s = s+1;
			}
			/* look for ch in tbl */
			if (!complement) {
				for (tp = (TWORD*)tbl; *tp != (TWORD)(-1); tp += 2) {
					if (*tp == tch) break;
				}
				matched = (*tp != (TWORD)(-1));
				rch = tp[1];
			} else {
				for (tp = (TWORD*)tbl; *tp != (TWORD)(-1); tp += 2) {
					if (*tp == tch) break;
				}
				matched = (*tp == (TWORD)(-1));
				rch = (TWORD)(del_char ? -2 : -1);
			}
	
		
			if (!matched) {
				sv_catkanji(dest_sv, tch);
			} else {
				matches++;
				if (complement && (rch == (TWORD)(-1))) {
					rch = *(tp - 1);
				}
				if (rch == (TWORD)(-2)) {
				/* delete this character */
				} else if (squash) {
					if (last_rch == (rch==(TWORD)(-1)?tch:rch)) {
						;	// delete this char
					} else {
						sv_catkanji(dest_sv, rch == (TWORD)(-1) ? tch : rch);
	//					matches++;
					}
				} else {
					sv_catkanji(dest_sv, rch==(TWORD)(-1) ? tch : rch);
	//				matches++;
				}
			}
			last_rch = ((rch==(TWORD)(-1)||rch==(TWORD)(-2)) ? tch : rch);
			s = next_s;
		}
	//	matches += (s-(TBYTE*)target) - dlen;	/* account for disappeared chars */
		
	
		rx->outp = SvPVX(dest_sv);
		rx->outendp = rx->outp + SvCUR(dest_sv);
		dest_sv->xpv_pv = NULL;
		sv_free(dest_sv);
	
	
		return matches;
	}
	catch (std::exception& ex) {
TRACE0(_T("out of space in trans()\n"));
		if (dest_sv)
			sv_free(dest_sv);
		asc2tcs(msg, ex.what());
		return -1;
	}
}

void sv_catkanji(SV *sv, U32 tch)
{
	TCHAR ch[2];
	int len = set_codepoint(tch, (TBYTE*)ch);
	sv_catpvn(sv,ch,len);
	return;
}

} // namespace

//
// bsplit.cc
//   Split front-end  
////////////////////////////////////////////////////////////////////////////////
//  1999.11.24   update by Tatsuo Baba
//  2006.08.29   update by K.Takata
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
#include "bregexp.h"
//#include "global.h"
#include "bregonig.h"
#include "mem_vc6.h"
#include "dbgtrace.h"


using namespace BREGONIG_NS;
namespace BREGONIG_NS {

int split_onig(bregonig *rx, TCHAR *target, TCHAR *targetendp, int limit, TCHAR *msg)
{
	TCHAR *orig,*m;
	TCHAR *s = target;
	int len = targetendp - target;
	if (len < 1)
		return -1;
	TCHAR *strend = s + len;
	int maxiters = (strend - s) + 10;
	int iters = 0;
	orig = m = s;

	rx->splitctr = 0;	// split counter


	// rx->prelen = 0 means split each characters
	//   and limit is 1 returns all string
	if (!rx->prelen || limit == 1) {
		int blen = 2*len + 3;
		if (limit == 1)
			blen = 5;
		TCHAR **buf = new (std::nothrow) TCHAR*[blen];
		int copycnt = 0;
		if (buf == NULL) {
			asc2tcs(msg,"out of space buf");
			return -1;
		}
#ifdef UNICODE
		int kanjiflag = rx->pmflags & PMf_KANJI;
#else
		int kanjiflag = 1;
#endif
		while (s < strend) {
			if (--limit == 0) {
				buf[copycnt++] = s;
				buf[copycnt++] = strend;
				break;
			}
			if (kanjiflag && is_char_pair((TBYTE*)s)) {
				buf[copycnt++] = s;
				s += 2;
				buf[copycnt++] = s;
			} else {
				buf[copycnt++] = s++;
				buf[copycnt++] = s;
			}
		}
		if (copycnt) {
			rx->splitctr = copycnt / 2;	// split counter
			buf[copycnt] = NULL;	// set stopper
			buf[copycnt+1] = NULL;	// set stopper
			rx->splitp = buf;
		}
		else
			delete [] buf;

		return rx->splitctr;
	}

	// now ready
	int blen = 256;				// initial size
	TCHAR **buf = new (std::nothrow) TCHAR*[blen];
	int copycnt = 0;
	if (buf == NULL) {
		asc2tcs(msg,"out of space buf");
		return -1;
	}
	if (!regexec_onig(rx, s, strend, orig, 0,1,0,msg)) { // no split ?
		buf[0] = target;
		buf[1] = targetendp;
		rx->splitctr = 1;	// split counter
		buf[2] = NULL;	// set stopper
		buf[3] = NULL;	// set stopper
		rx->splitp = buf;
		return 1;
	}
	// now ready to go
	limit--;
	do {
		if (iters++ > maxiters) {
			delete [] buf;
			asc2tcs(msg,"Split loop");
			return -1;
		}
		m = rx->startp[0];
		len = m - s;
		if (blen <= copycnt + 3) {
			TCHAR **tp = new (std::nothrow) TCHAR*[blen + 256];
			if (tp == NULL) {
				asc2tcs(msg,"out of space buf");
				delete [] buf;
				return -1;
			}
			memcpy(tp,buf,copycnt*sizeof(TCHAR*));
			delete [] buf;
			buf = tp; blen += 256;
		}
		buf[copycnt++] = s;
		buf[copycnt++] = s+ len;
		s = rx->endp[0];
		if (--limit == 0)
			break;
	} while (regexec_onig(rx, s, strend, orig, s == m, 1, 0, msg));
//	len = rx->subend - s;
	len = targetendp - s;	// ???
	if (blen <= copycnt + 3) {
		TCHAR **tp = new (std::nothrow) TCHAR*[blen + 3];
		if (tp == NULL) {
			asc2tcs(msg,"out of space buf");
			delete [] buf;
			return -1;
		}
		memcpy(tp,buf,copycnt*sizeof(TCHAR*));
		delete [] buf;
		buf = tp;
	}
	if (len) {
		buf[copycnt++] = s;
		buf[copycnt++] = s+ len;
	}
	if (copycnt) {
		rx->splitctr = copycnt / 2;	// split counter
		buf[copycnt] = NULL;	// set stopper
		buf[copycnt+1] = NULL;	// set stopper
		rx->splitp = buf;
	}
	else
		delete [] buf;

	return rx->splitctr;
}


} // namespace

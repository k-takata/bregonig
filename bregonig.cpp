/*
 *	bregonig.cpp
 */
/*
 *	Copyright (C) 2006-2009  K.Takata
 *
 *	You may distribute under the terms of either the GNU General Public
 *	License or the Artistic License, as specified in the perl_license.txt file.
 */
/*
 * Note:
 *	This file is based on the following files:
 *	 Bregexp.dll (main.cc, bsubst.cc) by Tatsuo Baba
 *	 K2Regexp.dll (main.cpp) by Koyabu Kazuya (K2)
 *	 Bregexp.dll for SAKURA (main.cpp) by Karoto
 */


#define _CRT_SECURE_NO_DEPRECATE
#define WIN32_LEAN_AND_MEAN

#define _BREGEXP_
#define GLOBAL_VALUE_DEFINE

#include <windows.h>
#include <new>
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
#include "version.h"
#include "mem_vc6.h"
#include "dbgtrace.h"


using namespace BREGONIG_NS;


extern OnigSyntaxType OnigSyntaxPerl_NG_EX;
#ifndef UNICODE
OnigSyntaxType OnigSyntaxPerl_NG_EX = *ONIG_SYNTAX_PERL_NG;
/*
OnigSyntaxType OnigSyntaxPerl_NG_EX = {
	ONIG_SYNTAX_PERL_NG->op,
	ONIG_SYNTAX_PERL_NG->op2,
	ONIG_SYNTAX_PERL_NG->behavior | ONIG_SYN_DIFFERENT_LEN_ALT_LOOK_BEHIND,
	ONIG_SYNTAX_PERL_NG->options,
};
*/


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		OnigSyntaxPerl_NG_EX.op2 |=
#ifdef USE_VTAB
				ONIG_SYN_OP2_ESC_V_VTAB |
#endif
#ifndef PERL_5_8_COMPAT
				ONIG_SYN_OP2_PLUS_POSSESSIVE_REPEAT |
				ONIG_SYN_OP2_PLUS_POSSESSIVE_INTERVAL |
#endif
				ONIG_SYN_OP2_CCLASS_SET_OP;
		OnigSyntaxPerl_NG_EX.behavior |= ONIG_SYN_DIFFERENT_LEN_ALT_LOOK_BEHIND;
		OnigSyntaxPerl_NG_EX.options |= ONIG_OPTION_CAPTURE_GROUP;
		onig_init();
		break;
		
	case DLL_PROCESS_DETACH:
		if (lpvReserved == NULL) {	// called via FreeLibrary()
			onig_end();
		}
		break;
		
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}
#endif


TCHAR *::BRegexpVersion(void)
{
	static TCHAR version[80];
	_sntprintf(version, lengthof(version),
			_T("bregonig.dll Ver.%hs with Oniguruma %hs"),
			BREGONIG_VERSION_STRING,
			onig_version());
	version[lengthof(version) - 1] = '\0';	// Ensure NUL termination.
	
	return version;
}


#ifdef _K2REGEXP_
int ::BMatch(TCHAR *str, TCHAR *target, TCHAR *targetstartp, TCHAR *targetendp,
		int one_shot,
		BREGEXP **rxp, TCHAR *msg)
{
	return BMatch_s(str, target, targetstartp, targetendp, one_shot, rxp, msg);
}
#else
int ::BMatch(TCHAR *str, TCHAR *target, TCHAR *targetendp,
		BREGEXP **rxp, TCHAR *msg)
{
	return BMatch_s(str, target, target, targetendp, 0, rxp, msg);
}
int ::BMatchEx(TCHAR *str, TCHAR *targetbegp, TCHAR *target, TCHAR *targetendp,
		BREGEXP **rxp, TCHAR *msg)
{
	return BMatch_s(str, targetbegp, target, targetendp, 0, rxp, msg);
}
#endif


#ifdef _K2REGEXP_
int ::BSubst(TCHAR *str, TCHAR *target, TCHAR *targetstartp, TCHAR *targetendp,
		BREGEXP **rxp, TCHAR *msg, BCallBack callback)
{
	return BSubst_s(str, target, targetstartp, targetendp, rxp, msg, callback);
}
#else
int ::BSubst(TCHAR *str, TCHAR *target, TCHAR *targetendp,
		BREGEXP **rxp, TCHAR *msg)
{
	return BSubst_s(str, target, target, targetendp, rxp, msg, NULL);
}
int ::BSubstEx(TCHAR *str, TCHAR *targetbegp, TCHAR *target, TCHAR *targetendp,
		BREGEXP **rxp, TCHAR *msg)
{
	return BSubst_s(str, targetbegp, target, targetendp, rxp, msg, NULL);
}
#endif


int ::BTrans(TCHAR *str, TCHAR *target, TCHAR *targetendp,
		BREGEXP **rxp, TCHAR *msg)
{
TRACE1(_T("BTrans(): %s\n"), str);
	set_new_throw_bad_alloc();
	
	int plen;
	if (check_params(str, target, target/*startp*/, targetendp,
			rxp, true, msg, &plen) < 0) {
		return -1;
	}
	bregonig *rx = static_cast<bregonig*>(*rxp);
	if (rx == NULL) {
		rx = compile_onig(str, plen, msg);
		if (rx == NULL) {
			*rxp = NULL;
			return -1;
		}
	}
	*rxp = rx;
	
	if (rx->outp) {				// free old result string
		delete [] rx->outp;
		rx->outp = NULL;
	}
	
	if (!(rx->pmflags & PMf_TRANSLATE)) {
		delete rx;
		*rxp = NULL;
		asc2tcs(msg, "no translate parameter", BREGEXP_MAX_ERROR_MESSAGE_LEN);
		return -1;
	}
	
	int matched = trans(rx,target,targetendp,msg);
	return msg[0] == '\0' ? matched: -1;
}


int ::BSplit(TCHAR *str, TCHAR *target, TCHAR *targetendp,
		int limit, BREGEXP **rxp, TCHAR *msg)
{
TRACE1(_T("BSplit(): %s\n"), str);
	set_new_throw_bad_alloc();
	
	int plen;
	if (check_params(str, target, target/*startp*/, targetendp,
			rxp, false, msg, &plen) < 0) {
		return -1;
	}
	bregonig *rx = static_cast<bregonig*>(*rxp);
	if (rx == NULL) {
		rx = compile_onig(str, plen, msg);
		if (rx == NULL) {
			*rxp = NULL;
			return -1;
		}
	}
	*rxp = rx;
	
	if (rx->splitp) {
		delete [] rx->splitp;
		rx->splitp = NULL;
	}
	
	int ctr = split_onig(rx,target,targetendp,limit,msg);
	return msg[0] == '\0' ? ctr : -1;
}


void ::BRegfree(BREGEXP *rx)
{
TRACE1(_T("BRegfree(): rx=0x%08x\n"), rx);
	if (rx) {
		delete static_cast<bregonig*>(rx);
	}
}





namespace BREGONIG_NS {

int onig_err_to_bregexp_msg(int err_code, OnigErrorInfo* err_info, TCHAR *msg)
{
	char err_str[ONIG_MAX_ERROR_MESSAGE_LEN];
	int ret = onig_error_code_to_str((UChar*) err_str, err_code, err_info);
	err_str[BREGEXP_MAX_ERROR_MESSAGE_LEN-1] = '\0';
	asc2tcs(msg, err_str, BREGEXP_MAX_ERROR_MESSAGE_LEN);
	return ret;
}

int check_params(const TCHAR *str, const TCHAR *target, const TCHAR *targetstartp,
		const TCHAR *targetendp,
		BREGEXP **rxp, bool trans, TCHAR *msg, int *pplen)
{
	if (msg == NULL)		// no message area
		return -1;
	msg[0] = '\0';			// ensure no error
TRACE3(_T("str:%s, len:%d, len:%d\n"), str, targetendp-target, targetendp-targetstartp);
TRACE1(_T("rxp:0x%08x\n"), rxp);
	
	if (rxp == NULL) {
		asc2tcs(msg, "invalid BREGEXP parameter", BREGEXP_MAX_ERROR_MESSAGE_LEN);
		return -1;
	}
TRACE1(_T("rx:0x%08x\n"), *rxp);
	if (target == NULL || targetstartp == NULL || targetendp == NULL
		|| targetstartp >= targetendp || target > targetstartp) { // bad target parameter ?
		asc2tcs(msg, "invalid target parameter", BREGEXP_MAX_ERROR_MESSAGE_LEN);
		return -1;
	}
	
	
	int plen = (str == NULL) ? 0 : _tcslen(str);
	*pplen = plen;
	
	bregonig *rx = static_cast<bregonig*>(*rxp);
	if (plen == 0) {	// null string
		if (rx == NULL) {
			asc2tcs(msg, "invalid reg parameter", BREGEXP_MAX_ERROR_MESSAGE_LEN);
			return -1;
		}
	} else if (rx) {
		if ((trans == !(rx->pmflags & PMf_TRANSLATE)
				|| rx->paraendp - rx->parap != plen
				|| memcmp(str,rx->parap,plen*sizeof(TCHAR)) != 0)) {
			// differ from the previous pattern
			delete rx;
			*rxp = NULL;
TRACE0(_T("delete rx\n"));
		}
	}
//	*rxp = rx;
	return 0;
}


int BMatch_s(TCHAR *str, TCHAR *target, TCHAR *targetstartp, TCHAR *targetendp,
		int one_shot,
		BREGEXP **rxp, TCHAR *msg)
{
TRACE1(_T("BMatch(): %s\n"), str);
	set_new_throw_bad_alloc();
	
	int plen;
	if (check_params(str, target, targetstartp, targetendp,
			rxp, false, msg, &plen) < 0) {
		return -1;
	}
	bregonig *rx = static_cast<bregonig*>(*rxp);
	if (rx == NULL) {
		rx = compile_onig(str, plen, msg);
		if (rx == NULL) {
			*rxp = NULL;
			return -1;
		}
	}
	*rxp = rx;
	
	if (rx->outp) {
		delete [] rx->outp;
		rx->outp = NULL;
	}
	
	if (rx->prelen == 0) {			// no string
		return 0;
	}
	
	int err_code = regexec_onig(rx, targetstartp, targetendp, target,
			0, 1, one_shot, msg);
#if 0
	if (err_code > 0 && rx->nparens && rx->endp[1] > rx->startp[1]) {
		int len = rx->endp[1] - rx->startp[1];
		TCHAR *tp = new (std::nothrow) TCHAR[len+1];
		if (tp == NULL) {
			asc2tcs(msg, "match out of space", BREGEXP_MAX_ERROR_MESSAGE_LEN);
			return -1;
		}
		memcpy(tp,rx->startp[1],len*sizeof(TCHAR));
		rx->outp = tp;
		rx->outendp = tp + len;
		*(rx->outendp) = '\0';
	}
#endif
	return err_code;
}


int BSubst_s(TCHAR *str, TCHAR *target, TCHAR *targetstartp, TCHAR *targetendp,
		BREGEXP **rxp, TCHAR *msg, BCallBack callback)
{
TRACE1(_T("BSubst(): %s\n"), str);
	set_new_throw_bad_alloc();
	
	int plen;
	if (check_params(str, target, targetstartp, targetendp,
			rxp, false, msg, &plen) < 0) {
		return -1;
	}
	bregonig *rx = static_cast<bregonig*>(*rxp);
	if (rx == NULL) {
		rx = compile_onig(str, plen, msg);
		if (rx == NULL) {
TRACE0(_T("rx == NULL\n"));
			*rxp = NULL;
			return -1;
		}
	}
	*rxp = rx;
	
	if (rx->outp) {
		delete [] rx->outp;
		rx->outp = NULL;
	}
	
	if (rx->prelen == 0) {			// no string
		return 0;
	}
	
	if (rx->pmflags & PMf_SUBSTITUTE) {
		return subst_onig(rx,target,targetstartp,targetendp,msg,callback);
	}
	// unusual case
TRACE0(_T("Match in Subst"));
#if 0
	int err_code = regexec_onig(rx, targetstartp,targetendp,target,0,1,0,msg);
	if (err_code > 0 && rx->nparens && rx->endp[1] > rx->startp[1]) {
		int len = rx->endp[1] - rx->startp[1];
		TCHAR *tp = new (std::nothrow) TCHAR[len+1];
		if (tp == NULL) {
			asc2tcs(msg, "match out of space", BREGEXP_MAX_ERROR_MESSAGE_LEN);
			return -1;
		}
		memcpy(tp,rx->startp[1],len*sizeof(TCHAR));
		rx->outp = tp;
		rx->outendp = tp + len;
		*(rx->outendp) = '\0';
	}
	return err_code;
#else
	delete rx;
	*rxp = NULL;
	return -1;
#endif
}




bregonig::bregonig()
{
	memset(this, 0, sizeof(bregonig));
	region = onig_region_new();
}

bregonig::~bregonig()
{
	if (region) {
		onig_region_free(region, 1);
	}
	if (reg) {
		onig_free(reg);
	}
	delete [] outp;
	delete [] splitp;
	delete [] parap;
	delete [] transtblp;
	delete [] startp;
//	delete [] endp;
	if (repstr) {
		delete repstr;
	}
}






bregonig *compile_onig(const TCHAR *ptn, int plen, TCHAR *msg)
{
TRACE0(_T("compile_onig()\n"));
TRACE1(_T("ptn:%s\n"), ptn);
TRACE1(_T("plen:%d\n"), plen);
	TCHAR *parap = new (std::nothrow) TCHAR[plen+1];	// parameter copy
	if (parap == NULL) {
		asc2tcs(msg, "precompile out of space", BREGEXP_MAX_ERROR_MESSAGE_LEN);
		return NULL;
	}
	memcpy(parap, ptn, (plen+1)*sizeof(TCHAR));	// copy include null
	
	TCHAR type = 'm';		// default is match
	TCHAR *p = parap;
	TCHAR *pend = p + plen;
	TCHAR sep = '/';			// default separater
	if (*p != '/') {
		if (*p != 's' && *p != 'm' && memcmp(p,_T("tr"),2*sizeof(TCHAR)) != 0) {
			asc2tcs(msg,"do not start 'm' or 's' or 'tr'",
					BREGEXP_MAX_ERROR_MESSAGE_LEN);
			delete [] parap;
			return NULL;
		}
		if (*p == 's')
			type = 's';		// substitute command
		else if (*p == 't') {
			type = 't';		// translate command
			p++;
		}
		sep = *++p;
	}
	p++;
	TCHAR *res = p;
	TCHAR *resend = NULL, *rp = NULL, *rpend = NULL;
	TCHAR prev = 0;
	while (*p != '\0') {
#ifndef UNICODE
		if (iskanji(*p)) {
			prev = 0; p += 2;
			continue;
		}
#endif
		if (*p == '\\' && prev == '\\') {
			prev = 0; p++;
			continue;
		}
		if (*p == '/' && prev == '\\') {	// \/ means /
			prev = 0; p++;
			continue;
		}
		if (*p == sep && prev != '\\') {
			if (!resend) {
				resend = p;
				rp = ++p;
				continue;
			} else {
				rpend = p;
			}
			p++;
			break;
		}
		prev = *p++;
	}
	if (!resend || (!rpend && type != 'm')) {
		asc2tcs(msg, "unmatch separater", BREGEXP_MAX_ERROR_MESSAGE_LEN);
		delete [] parap;
		return NULL;
	}
	
	if (!rpend)
		p = resend + 1;
	
	int flag = 0;
	OnigOptionType option = ONIG_OPTION_NONE;
#ifdef UNICODE
	OnigEncoding enc = ONIG_ENCODING_UTF16_LE;
#else
	OnigEncoding enc = ONIG_ENCODING_ASCII;
#endif
	while (*p != '\0') {
		switch (*p++) {
		case 'g':
			flag |= PMf_GLOBAL;
			break;
		case 'i':
			flag |= PMf_FOLD;
			option |= ONIG_OPTION_IGNORECASE;
			break;
		case 'm':
			//flag |= PMf_MULTILINE;
			option |= ONIG_OPTION_NEGATE_SINGLELINE;
			break;
		case 'o':
			flag |= PMf_KEEP;
			break;
		case 'k':
			flag |= PMf_KANJI;
#ifndef UNICODE
			enc = ONIG_ENCODING_SJIS;
#endif
			break;
		case 'c':
			flag |= PMf_TRANS_COMPLEMENT;
			break;
		case 'd':
			flag |= PMf_TRANS_DELETE;
			break;
		case 's':
			flag |= PMf_TRANS_SQUASH;
			//flag |= PMf_SINGLELINE;
			option |= ONIG_OPTION_MULTILINE;
			break;
		case 'x':
			option |= ONIG_OPTION_EXTEND;
			break;
		default:
			break;
		}
	}
	
	
/*
	TRACE1(_T("parap: %s\r\n"), parap);
	TRACE1(_T("res: %s\r\n"), res);
	TRACE1(_T("resend: %s\r\n"), resend);
	TRACE1(_T("rp: %s\r\n"), rp);
	TRACE1(_T("rpend: %s\r\n"), rpend);
*/
	
	if (type == 't') {
		bregonig *rx = trcomp(res,resend,rp,rpend,flag,msg);
		if (rx == NULL) {
			delete [] parap;
			return NULL;
		}
		rx->parap = parap;
		rx->paraendp = rx->parap + plen;
		return rx;
	}
	
	
	bregonig *rx = new (std::nothrow) bregonig();
	if (rx == NULL) {
		asc2tcs(msg, "out of space regexp", BREGEXP_MAX_ERROR_MESSAGE_LEN);
		delete [] parap;
		return NULL;
	}
//	OnigSyntaxPerl_NG_EX.behavior |= ONIG_SYN_DIFFERENT_LEN_ALT_LOOK_BEHIND;
	OnigErrorInfo err_info;
	int err_code = onig_new(&rx->reg, (UChar*) res, (UChar*) resend,
			option, enc, /*ONIG_SYNTAX_PERL_NG*/&OnigSyntaxPerl_NG_EX, &err_info);
	
	if (err_code != ONIG_NORMAL) {
TRACE0(_T("Error: onig_new()\n"));
		onig_err_to_bregexp_msg(err_code, &err_info, msg);
		delete rx;
		delete [] parap;
		return NULL;
	}
	
	rx->nparens = onig_number_of_captures(rx->reg);	//
	rx->parap = parap;
	rx->paraendp = parap + plen;
	rx->pmflags = flag;
	rx->prelen = resend - res;
	
	if (type == 's') {						// substitute
		try {
			rx->pmflags |= PMf_SUBSTITUTE;
			rx->repstr = compile_rep(rx, rp, rpend);	// compile replace string
			rx->prerepp = rp;
			rx->prerependp = rpend;
		} catch (std::exception& ex) {
			delete rx;
		//	delete [] parap;		// deleted by the deconstructor of rx
			asc2tcs(msg, ex.what(), BREGEXP_MAX_ERROR_MESSAGE_LEN);
			return NULL;
		}
	}
	
TRACE1(_T("rx:0x%08x\n"), rx);
	return rx;
}


int regexec_onig(bregonig *rx, TCHAR *stringarg,
	register TCHAR *strend,	/* pointer to null at end of string */
	TCHAR *strbeg,	/* real beginning of string */
	int minend,		/* end of match must be at least minend after stringarg */
	int safebase,	/* no need to remember string in subbase */
	int one_shot,	/* if not match then break without proceed str pointer */
	TCHAR *msg)		/* fatal error message */
{
TRACE1(_T("one_shot: %d\n"), one_shot);
	int err_code;
	
	if (one_shot) {
		OnigOptionType option = (minend > 0) ?
				ONIG_OPTION_FIND_NOT_EMPTY : ONIG_OPTION_NONE;
		err_code = onig_match(rx->reg, (UChar*) strbeg, (UChar*) strend,
				(UChar*) stringarg, rx->region,
				option);
	} else {
		TCHAR *global_pos = stringarg;		/* \G */
		if (minend > 0) {
#ifdef UNICODE
			int kanjiflag = 1;
#else
			int kanjiflag = rx->pmflags & PMf_KANJI;
#endif
			if (kanjiflag && is_char_pair((TBYTE*) stringarg)) {
				stringarg += 2;
			} else {
				stringarg++;
			}
		}
		err_code = onig_search2(rx->reg, (UChar*) strbeg, (UChar*) strend,
				(UChar*) global_pos,
				(UChar*) stringarg, (UChar*) strend, rx->region,
				ONIG_OPTION_NONE);
	}
	
	if (err_code >= 0) {
		/* FOUND */
		if (rx->startp) {
			delete [] rx->startp;
		}
		rx->nparens = rx->region->num_regs - 1;
		rx->startp = new (std::nothrow) TCHAR*[rx->region->num_regs * 2];
			/* allocate startp and endp together */
		if (rx->startp == NULL) {
			asc2tcs(msg, "out of space", BREGEXP_MAX_ERROR_MESSAGE_LEN);
			return -1;
		}
		rx->endp = rx->startp + rx->region->num_regs;
		
		for (int i = 0; i < rx->region->num_regs; i++) {
			if (rx->region->beg[i] != ONIG_REGION_NOTPOS) {
				// found
				rx->startp[i] = strbeg/**/ + rx->region->beg[i] / sizeof(TCHAR);
				rx->endp[i] = strbeg/**/ + rx->region->end[i] / sizeof(TCHAR);
			} else {
				// not found
				rx->startp[i] = NULL;
				rx->endp[i] = NULL;
			}
		}
		return 1;
	} else if (err_code == ONIG_MISMATCH) {
		/* NOT FOUND */
		return 0;
	} else {
		/* ERROR */
		onig_err_to_bregexp_msg(err_code, NULL, msg);
		return -1;
	}
}

} // namespace

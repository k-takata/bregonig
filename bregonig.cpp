/*
 *	bregonig.cpp
 */
/*
 *	Copyright (C) 2006-2011  K.Takata
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
#ifdef USE_ONIGMO_6
# include <onigmo.h>
#else
# include <oniguruma.h>
#endif
#include "bregexp.h"
//#include "global.h"
#include "bregonig.h"
#include "version.h"
#include "mem_vc6.h"
#include "dbgtrace.h"


using namespace BREGONIG_NS;


extern OnigSyntaxType OnigSyntaxPerl_NG_EX;
#ifndef UNICODE
OnigSyntaxType OnigSyntaxPerl_NG_EX = OnigSyntaxPerl;
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
				ONIG_SYN_OP2_ESC_G_SUBEXP_CALL |			/* bregonig.dll extension */
				ONIG_SYN_OP2_CCLASS_SET_OP;					/* bregonig.dll extension */
		OnigSyntaxPerl_NG_EX.behavior |=
				ONIG_SYN_DIFFERENT_LEN_ALT_LOOK_BEHIND;		/* bregonig.dll extension */
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
			_T("bregonig.dll Ver.%hs with Onigmo %hs"),
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
	
	if (check_params(target, target/*startp*/, targetendp, rxp, msg, false) < 0) {
		return -1;
	}
	bregonig *rx = static_cast<bregonig*>(*rxp);
	*rxp = rx = recompile_onig(rx, PTN_TRANS, str, msg);
	if (*rxp == NULL) {
		return -1;
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
	
	if (check_params(target, target/*startp*/, targetendp, rxp, msg, false) < 0) {
		return -1;
	}
	bregonig *rx = static_cast<bregonig*>(*rxp);
	*rxp = rx = recompile_onig(rx, PTN_MATCH, str, msg);
	if (*rxp == NULL) {
		return -1;
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


#ifndef _K2REGEXP_
int ::BoMatch(const TCHAR *patternp, const TCHAR *optionp,
		const TCHAR *strstartp,
		const TCHAR *targetstartp, const TCHAR *targetendp,
		BOOL one_shot,
		BREGEXP **rxp, TCHAR *msg)
{
	set_new_throw_bad_alloc();
	
	const TCHAR *substp = NULL;
	const TCHAR *patternendp = (patternp != NULL) ? patternp + _tcslen(patternp) : NULL;
	const TCHAR *substendp = NULL;
	const TCHAR *optionendp = (optionp != NULL) ? optionp + _tcslen(optionp) : NULL;
	
	if (check_params(strstartp, targetstartp, targetendp, rxp, msg, true) < 0) {
		return -1;
	}
	
	bregonig *rx = static_cast<bregonig*>(*rxp);
	*rxp = rx = recompile_onig_ex(rx, PTN_MATCH, NULL, patternp, patternendp,
			substp, substendp, optionp, optionendp, msg);
	if (rx == NULL) {
		return -1;
	}
	
	int err_code = regexec_onig(rx, targetstartp, targetendp, strstartp,
			0, 1, one_shot, msg);
	
	return err_code;
}

int ::BoSubst(const TCHAR *patternp, const TCHAR *substp, const TCHAR *optionp,
		const TCHAR *strstartp,
		const TCHAR *targetstartp, const TCHAR *targetendp,
		BCallBack callback,
		BREGEXP **rxp, TCHAR *msg)
{
	set_new_throw_bad_alloc();
	
	const TCHAR *patternendp = (patternp != NULL) ? patternp + _tcslen(patternp) : NULL;
	const TCHAR *substendp = (substp != NULL) ? substp + _tcslen(substp) : NULL;
	const TCHAR *optionendp = (optionp != NULL) ? optionp + _tcslen(optionp) : NULL;
	
	if (check_params(strstartp, targetstartp, targetendp, rxp, msg, true) < 0) {
		return -1;
	}
	
	bregonig *rx = static_cast<bregonig*>(*rxp);
	*rxp = rx = recompile_onig_ex(rx, PTN_SUBST, NULL, patternp, patternendp,
			substp, substendp, optionp, optionendp, msg);
	if (rx == NULL) {
		return -1;
	}
	return subst_onig(rx, strstartp, targetstartp, targetendp, msg, callback);
}
#endif



namespace BREGONIG_NS {

int onig_err_to_bregexp_msg(OnigPosition err_code, OnigErrorInfo* err_info, TCHAR *msg)
{
	char err_str[ONIG_MAX_ERROR_MESSAGE_LEN];
	int ret = onig_error_code_to_str((UChar*) err_str, err_code, err_info);
	err_str[BREGEXP_MAX_ERROR_MESSAGE_LEN-1] = '\0';
	asc2tcs(msg, err_str, BREGEXP_MAX_ERROR_MESSAGE_LEN);
	return ret;
}

int check_params(const TCHAR *target, const TCHAR *targetstartp,
		const TCHAR *targetendp, BREGEXP **rxp, TCHAR *msg, bool allownullstr)
{
	if (msg == NULL)		// no message area
		return -1;
	msg[0] = '\0';			// ensure no error
	
	if (rxp == NULL) {
		asc2tcs(msg, "invalid BREGEXP parameter", BREGEXP_MAX_ERROR_MESSAGE_LEN);
		return -1;
	}
	const TCHAR *endp = targetendp;
	if (allownullstr) {
		endp++;
	}
	if (target == NULL || targetstartp == NULL || targetendp == NULL
			|| targetstartp >= endp || target > targetstartp) { // bad target parameter ?
		asc2tcs(msg, "invalid target parameter", BREGEXP_MAX_ERROR_MESSAGE_LEN);
		return -1;
	}
	
	return 0;
}


int BMatch_s(TCHAR *str, TCHAR *target, TCHAR *targetstartp, TCHAR *targetendp,
		int one_shot,
		BREGEXP **rxp, TCHAR *msg)
{
TRACE(_T("BMatch(): '%s' (%p), %p, %p, %p\n"), str, str, target, targetstartp, targetendp);
	set_new_throw_bad_alloc();
	
	if (check_params(target, targetstartp, targetendp, rxp, msg, false) < 0) {
		return -1;
	}
	bregonig *rx = static_cast<bregonig*>(*rxp);
	*rxp = rx = recompile_onig(rx, PTN_MATCH, str, msg);
	if (rx == NULL) {
		return -1;
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
TRACE(_T("BSubst(): '%s' (%p), %p, %p, %p\n"), str, str, target, targetstartp, targetendp);
	set_new_throw_bad_alloc();
	
	if (check_params(target, targetstartp, targetendp, rxp, msg, false) < 0) {
		return -1;
	}
	bregonig *rx = static_cast<bregonig*>(*rxp);
	*rxp = rx = recompile_onig(rx, PTN_SUBST, str, msg);
	if (rx == NULL) {
		return -1;
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
	
	delete [] patternp;
//	delete [] prerepp;
//	delete [] optionp;
	
	if (repstr) {
		delete repstr;
	}
}



pattern_type parse_pattern(const TCHAR *ptn, pattern_type typeold,
		const TCHAR **patternp, const TCHAR **patternendp,
		const TCHAR **prerepp, const TCHAR **prerependp,
		const TCHAR **optionp, const TCHAR **optionendp,
		TCHAR *msg)
{
	if (ptn == NULL) {
		*patternp = NULL;
		*patternendp = NULL;
		*prerepp = NULL;
		*prerependp = NULL;
		*optionp = NULL;
		*optionendp = NULL;
		return typeold;
	}
	
	pattern_type type = PTN_MATCH;
	const TCHAR *p = ptn;
	const TCHAR *ptnend = ptn + _tcslen(ptn);
	TCHAR sep = '/';			// default separater
	
	if (*p != sep) {
		if (*p != 's' && *p != 'm' && *p != 'y'
				&& (p[0] != 't' || p[1] != 'r')) {
			asc2tcs(msg, "does not start with 'm', 's', 'tr' or 'y'",
					BREGEXP_MAX_ERROR_MESSAGE_LEN);
			return PTN_ERROR;
		}
		if (*p == 's') {
			type = PTN_SUBST;		// substitute command
		} else if (*p == 'y') {
			type = PTN_TRANS;		// translate command
		} else if (*p == 't') {
			type = PTN_TRANS;		// translate command
			p++;
		}
		sep = *++p;
	}
	p++;		// skip separater
	*patternp = p;
	
	const TCHAR *res = p;
	const TCHAR *resend = NULL, *rp = NULL, *rpend = NULL;
	TCHAR prev = 0;
	while (p < ptnend) {
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
			if (resend == NULL) {
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
	if ((resend == NULL) || (rpend == NULL && type != PTN_MATCH)) {
		asc2tcs(msg, "unmatch separater", BREGEXP_MAX_ERROR_MESSAGE_LEN);
		return PTN_ERROR;
	}
	if (rpend == NULL) {
		p = resend + 1;
		rp = NULL;
	}
	
	*patternendp = resend;
	*prerepp = rp;
	*prerependp = rpend;
	*optionp = p;
	*optionendp = ptnend;
	
	return type;
}

void parse_option(const TCHAR *optionp, const TCHAR *optionendp,
		OnigOptionType *onigoption, OnigEncoding *enc, int *flagp)
{
	const TCHAR *p = optionp;
	int flag = 0;
	OnigOptionType option = ONIG_OPTION_NONE;
#ifdef UNICODE
	*enc = ONIG_ENCODING_UTF16_LE;
#else
	*enc = ONIG_ENCODING_ASCII;
#endif
TRACE1(_T("option: %s"), optionp);
	while (p < optionendp) {
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
			*enc = ONIG_ENCODING_CP932;
#endif
			break;
#if !defined(UNICODE) && !defined(_K2REGEXP_)
		case '8':		/* bregonig.dll extension */
			*enc = ONIG_ENCODING_UTF8;
			break;
#endif
		case 'c':
			flag |= PMf_TRANS_COMPLEMENT;
			break;
		case 'd':
			flag |= PMf_TRANS_DELETE;
			option &= ~ONIG_OPTION_ASCII_RANGE;
			break;
		case 's':
			flag |= PMf_TRANS_SQUASH;
			//flag |= PMf_SINGLELINE;
			option |= ONIG_OPTION_MULTILINE;
			break;
		case 'x':
			option |= ONIG_OPTION_EXTEND;
			break;
		case 'a':
			option |= ONIG_OPTION_ASCII_RANGE;
			break;
		case 'l':
		case 'u':
			option &= ~ONIG_OPTION_ASCII_RANGE;
			break;
		case 'R':
			option |= ONIG_OPTION_NEWLINE_CRLF;
			break;
		default:
			break;
		}
	}
	*flagp = flag;
	*onigoption = option;
}


bregonig *recompile_onig(bregonig *rxold, pattern_type type,
		const TCHAR *ptn, TCHAR *msg)
{
	const TCHAR *patternp;
	const TCHAR *patternendp;
	const TCHAR *prerepp;
	const TCHAR *prerependp;
	const TCHAR *optionp;
	const TCHAR *optionendp;
	
TRACE1(_T("recompile_onig(): %s\n"), ptn);
	type = parse_pattern(ptn, type, &patternp, &patternendp,
			&prerepp, &prerependp, &optionp, &optionendp, msg);
	if (type == PTN_ERROR) {
		return NULL;
	}
	return recompile_onig_ex(rxold, type, ptn, patternp, patternendp,
			prerepp, prerependp, optionp, optionendp, msg);
}


/**
 * Compare the old regexp object and new pattern.
 *
 * return:
 *     -2: parameter error
 *     -1: Need to compile.
 *      0: No need to compile. The old regexp object can be reused.
 *      1: Replace string needs to compile.
 */
int compare_pattern(const bregonig *rxold,
		pattern_type type,
		const TCHAR *patternp, const TCHAR *patternendp,
		const TCHAR *prerepp, const TCHAR *prerependp,
		const TCHAR *optionp, const TCHAR *optionendp)
{
	pattern_type typeold;
	ptrdiff_t len1 = patternendp - patternp;
	ptrdiff_t len2 = prerependp - prerepp;
	ptrdiff_t len3 = optionendp - optionp;
	
TRACE2(_T("compare_pattern: %s, len: %d"), patternp, patternendp-patternp);
	if (rxold == NULL) {
		return -1;
	}
	
	if (rxold->pmflags & PMf_TRANSLATE) {
		typeold = PTN_TRANS;
	} else if (rxold->pmflags & PMf_SUBSTITUTE) {
		typeold = PTN_SUBST;
	} else {
		typeold = PTN_MATCH;
	}
	
	if ((typeold == PTN_TRANS) || (type == PTN_TRANS)) {
		if (typeold != type) {
			return -1;
		}
		if (patternp == NULL) {
			return 0;
		}
		if ((len1 != rxold->patternendp - rxold->patternp)
				|| (len2 != rxold->prerependp - rxold->prerepp)
				|| (len3 != rxold->optionendp - rxold->optionp)) {
			return -1;
		}
		if ((memcmp(patternp, rxold->patternp, len1*sizeof(TCHAR)) != 0)
				|| (memcmp(prerepp, rxold->prerepp, len2*sizeof(TCHAR)) != 0)
				|| (memcmp(optionp, rxold->optionp, len3*sizeof(TCHAR)) != 0)) {
			return -1;
		}
		return 0;
	} else if (type == PTN_SUBST) {
		if (prerepp == NULL) {
			if (patternp == NULL) {
				if (typeold == PTN_SUBST) {
					return 0;
				} else {
					return -2;	// error
				}
			}
			return -2;	// error
		}
		if (patternp != NULL) {
			if ((len1 != rxold->patternendp - rxold->patternp)
					|| (len3 != rxold->optionendp - rxold->optionp)) {
				return -1;
			}
			if ((memcmp(patternp, rxold->patternp, len1*sizeof(TCHAR)) != 0)
					|| (memcmp(optionp, rxold->optionp, len3*sizeof(TCHAR)) != 0)) {
				return -1;
			}
		}
		if ((typeold == PTN_SUBST)
				&& (len2 == rxold->prerependp - rxold->prerepp)
				&& (memcmp(prerepp, rxold->prerepp, len2*sizeof(TCHAR)) == 0)) {
			return 0;
		}
		return 1;		// compile_rep() is needed
	} else {
		if (patternp == NULL) {
			return 0;
		}
		if ((len1 != rxold->patternendp - rxold->patternp)
				|| (len3 != rxold->optionendp - rxold->optionp)) {
			return -1;
		}
		if ((memcmp(patternp, rxold->patternp, len1*sizeof(TCHAR)) != 0)
				|| (memcmp(optionp, rxold->optionp, len3*sizeof(TCHAR)) != 0)) {
			return -1;
		}
		return 0;
	}
}


bregonig *recompile_onig_ex(bregonig *rxold,
		pattern_type type, const TCHAR *ptn,
		const TCHAR *patternp, const TCHAR *patternendp,
		const TCHAR *prerepp, const TCHAR *prerependp,
		const TCHAR *optionp, const TCHAR *optionendp,
		TCHAR *msg)
{
	int flag, compare;
	bregonig *rx;
	OnigOptionType option;
	OnigEncoding enc;
TRACE0(_T("recompile_onig_ex()\n"));
TRACE2(_T("patternp: %s, len: %d\n"), patternp, patternendp-patternp);
TRACE2(_T("prerepp: %s, len: %d\n"), prerepp, prerependp-prerepp);
TRACE2(_T("optionp: %s, len: %d\n"), optionp, optionendp-optionp);
	
	
	compare = compare_pattern(rxold, type, patternp, patternendp,
			prerepp, prerependp, optionp, optionendp);
TRACE1(_T("compare: %d\n"), compare);
	if (compare < 0) {
		// need to compile
		delete rxold;
		rxold = NULL;
		
		if (patternp == NULL
				|| ((type == PTN_SUBST || type == PTN_TRANS) && prerepp == NULL)) {
			asc2tcs(msg, "invalid reg parameter", BREGEXP_MAX_ERROR_MESSAGE_LEN);
			return NULL;
		}
	} else {
		// no need to compile
		if (rxold->outp) {
			delete [] rxold->outp;
			rxold->outp = NULL;
		}
		if (rxold->splitp) {
			delete [] rxold->splitp;
			rxold->splitp = NULL;
		}
	}
	
	parse_option(optionp, optionendp, &option, &enc, &flag);
	
	if (type == PTN_TRANS) {
		if (compare == 0) {
			// no need to compile
TRACE1(_T("rxold(1):0x%08x\n"), rxold);
			return rxold;
		}
		rx = trcomp(patternp, patternendp, prerepp, prerependp, flag, msg);
		if (rx == NULL) {
			return NULL;
		}
	} else {
		if (compare == 0) {
			// no need to compile
TRACE1(_T("rxold(2):0x%08x\n"), rxold);
			return rxold;
		} else if (compare < 0) {
			// pattern string needs to compile.
			rx = new (std::nothrow) bregonig();
			if (rx == NULL) {
				asc2tcs(msg, "out of space regexp", BREGEXP_MAX_ERROR_MESSAGE_LEN);
				return NULL;
			}
			OnigErrorInfo err_info;
			int err_code = onig_new(&rx->reg,
					(UChar*) patternp, (UChar*) patternendp,
					option, enc, &OnigSyntaxPerl_NG_EX, &err_info);
			if (err_code != ONIG_NORMAL) {
				onig_err_to_bregexp_msg(err_code, &err_info, msg);
				delete rx;
				return NULL;
			}
			
			rx->nparens = onig_number_of_captures(rx->reg);	//
			rx->pmflags = flag;
		} else {
			// only replace string needs to compile.
			rx = rxold;
		}
		if (rxold != NULL && rxold->repstr != NULL) {
			delete rxold->repstr;
			rxold->repstr = NULL;
		}
		if (type == PTN_SUBST) {						// substitute
			try {
				rx->pmflags |= PMf_SUBSTITUTE;
				rx->repstr = compile_rep(rx, prerepp, prerependp);	// compile replace string
			} catch (std::exception& ex) {
				asc2tcs(msg, ex.what(), BREGEXP_MAX_ERROR_MESSAGE_LEN);
				delete rx;
				return NULL;
			}
		}
	}
	
	if (ptn != NULL) {
		size_t plen = _tcslen(ptn);
		delete [] rx->parap;
		rx->parap = new (std::nothrow) TCHAR[plen+1];	// parameter copy
		if (rx->parap == NULL) {
			asc2tcs(msg, "precompile out of space", BREGEXP_MAX_ERROR_MESSAGE_LEN);
			delete rx;
			return NULL;
		}
		memcpy(rx->parap, ptn, (plen+1)*sizeof(TCHAR));	// copy include null
		rx->paraendp = rx->parap + plen;
	}
	
	TCHAR *oldpatternp = rx->patternp;
	
	if (patternp == NULL) {
		patternp = rx->patternp;
		patternendp = rx->patternendp;
		optionp = rx->optionp;
		optionendp = rx->optionendp;
	}
	
	/* save pattern, replace and option string */
	ptrdiff_t len1 = patternendp - patternp;
	ptrdiff_t len2 = prerependp - prerepp;
	ptrdiff_t len3 = optionendp - optionp;
	rx->patternp = new (std::nothrow) TCHAR[len1+1 + len2+1 + len3+1];
	if (rx->patternp == NULL) {
		delete rx;
		delete [] oldpatternp;
		return NULL;
	}
	memcpy(rx->patternp, patternp, len1*sizeof(TCHAR));
	rx->patternp[len1] = 0;
	rx->patternendp = rx->patternp + len1;
	
	rx->prerepp = rx->patternp + len1 + 1;
	memcpy(rx->prerepp, prerepp, len2*sizeof(TCHAR));
	rx->prerepp[len2] = 0;
	rx->prerependp = rx->prerepp + len2;
	
	rx->optionp = rx->prerepp + len2 + 1;
	memcpy(rx->optionp, optionp, len3*sizeof(TCHAR));
	rx->optionp[len3] = 0;
	rx->optionendp = rx->optionp + len3;
	
	
	delete [] oldpatternp;
	
TRACE1(_T("rx:0x%08x\n"), rx);
	return rx;
}


int regexec_onig(bregonig *rx, const TCHAR *stringarg,
	const TCHAR *strend,	/* pointer to null at end of string */
	const TCHAR *strbeg,	/* real beginning of string */
	int minend,		/* end of match must be at least minend after stringarg */
	int safebase,	/* no need to remember string in subbase */
	int one_shot,	/* if not match then break without proceed str pointer */
	TCHAR *msg)		/* fatal error message */
{
TRACE1(_T("one_shot: %d\n"), one_shot);
	OnigPosition err_code;
	
	if (one_shot) {
		OnigOptionType option = (minend > 0) ?
				ONIG_OPTION_FIND_NOT_EMPTY : ONIG_OPTION_NONE;
		err_code = onig_match(rx->reg, (UChar*) strbeg, (UChar*) strend,
				(UChar*) stringarg, rx->region,
				option);
	} else {
		const TCHAR *global_pos = stringarg;		/* \G */
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
		err_code = onig_search_gpos(rx->reg, (UChar*) strbeg, (UChar*) strend,
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
				rx->startp[i] = const_cast<TCHAR *>(strbeg) + rx->region->beg[i] / sizeof(TCHAR);
				rx->endp[i] = const_cast<TCHAR *>(strbeg) + rx->region->end[i] / sizeof(TCHAR);
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

/*
 *	bregonig.cpp
 */
/*
 *	Copyright (C) 2006-2007  K.Takata
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
#include <oniguruma.h>
#include "bregexp.h"
//#include "global.h"
#include "bregonig.h"
#include "version.h"
#include "mem_vc6.h"
#include "dbgtrace.h"


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
		OnigSyntaxPerl_NG_EX.op2 |= ONIG_SYN_OP2_ESC_V_VTAB | ONIG_SYN_OP2_CCLASS_SET_OP;
		OnigSyntaxPerl_NG_EX.behavior |= ONIG_SYN_DIFFERENT_LEN_ALT_LOOK_BEHIND;
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


char *BRegexpVersion(void)
{
	static char version[256];
//	sprintf(version, "bregonig.dll Ver.%d.%02d%s %s with Oniguruma %s",
	sprintf(version, "bregonig.dll Ver.%d.%02d%s with Oniguruma %s",
			BREGONIG_VERSION_MAJOR, BREGONIG_VERSION_MINOR,
			BREGONIG_VERSION_PREFIX,
			/*__DATE__,*/ onig_version());
	
	return version;
}


int check_params(const char *str, const char *target, const char *targetstartp,
		const char *targetendp,
		BREGEXP **rxp, bool trans, char *msg, int *pplen)
{
	if (msg == NULL)		// no message area
		return -1;
	msg[0] = '\0';			// ensure no error
TRACE3("str:%s, len:%d, len:%d\n", str, targetendp-target, targetendp-targetstartp);
TRACE1("rxp:0x%08x\n", rxp);
	
	if (rxp == NULL) {
		strcpy(msg, "invalid BREGEXP parameter");
		return -1;
	}
TRACE1("rx:0x%08x\n", *rxp);
	if (target == NULL || targetstartp == NULL || targetendp == NULL
		|| targetstartp >= targetendp || target > targetstartp) { // bad targer parameter ?
		strcpy(msg, "invalid target parameter");
		return -1;
	}
	
	
	int plen = (str == NULL) ? 0 : strlen(str);
	*pplen = plen;
	
	bregonig *rx = static_cast<bregonig*>(*rxp);
	if (plen == 0) {	// null string
		if (rx == NULL) {
			strcpy(msg, "invalid reg parameter");
			return -1;
		}
	} else if (rx) {
		if ((trans == !(rx->pmflags & PMf_TRANSLATE)
				|| rx->paraendp - rx->parap != plen
				|| memcmp(str,rx->parap,plen) != 0)) {
			// differ from the previous pattern
			delete rx;
			*rxp = NULL;
TRACE0("delete rx\n");
		}
	}
//	*rxp = rx;
	return 0;
}


int BMatch_s(char *str, char *target, char *targetstartp, char *targetendp,
		int one_shot,
		BREGEXP **rxp, char *msg)
{
TRACE1("BMatch(): %s\n", str);
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
		char *tp = new (std::nothrow) char[len+1];
		if (tp == NULL) {
			strcpy(msg,"match out of space");
			return -1;
		}
		memcpy(tp,rx->startp[1],len);
		rx->outp = tp;
		rx->outendp = tp + len;
		*(rx->outendp) = '\0';
	}
#endif
	return err_code;
}

#ifdef _K2REGEXP_
int BMatch(char *str, char *target, char *targetstartp, char *targetendp,
		int one_shot,
		BREGEXP **rxp, char *msg)
{
	return BMatch_s(str, target, targetstartp, targetendp, one_shot, rxp, msg);
}
#else
int BMatch(char *str, char *target, char *targetendp,
		BREGEXP **rxp, char *msg)
{
	return BMatch_s(str, target, target, targetendp, 0, rxp, msg);
}
int BMatchEx(char *str, char *targetbegp, char *target, char *targetendp,
		BREGEXP **rxp, char *msg)
{
	return BMatch_s(str, targetbegp, target, targetendp, 0, rxp, msg);
}
#endif


int BSubst_s(char *str, char *target, char *targetstartp, char *targetendp,
		BREGEXP **rxp, char *msg, BCallBack callback)
{
TRACE1("BSubst(): %s\n", str);
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
TRACE0("rx == NULL\n");
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
TRACE0("Match in Subst");
#if 0
	int err_code = regexec_onig(rx, targetstartp,targetendp,target,0,1,0,msg);
	if (err_code > 0 && rx->nparens && rx->endp[1] > rx->startp[1]) {
		int len = rx->endp[1] - rx->startp[1];
		char *tp = new (std::nothrow) char[len+1];
		if (tp == NULL) {
			strcpy(msg,"match out of space");
			return -1;
		}
		memcpy(tp,rx->startp[1],len);
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

#ifdef _K2REGEXP_
int BSubst(char *str, char *target, char *targetstartp, char *targetendp,
		BREGEXP **rxp, char *msg, BCallBack callback)
{
	return BSubst_s(str, target, targetstartp, targetendp, rxp, msg, callback);
}
#else
int BSubst(char *str, char *target, char *targetendp,
		BREGEXP **rxp, char *msg)
{
	return BSubst_s(str, target, target, targetendp, rxp, msg, NULL);
}
int BSubstEx(char *str, char *targetbegp, char *target, char *targetendp,
		BREGEXP **rxp, char *msg)
{
	return BSubst_s(str, targetbegp, target, targetendp, rxp, msg, NULL);
}
#endif


int BTrans(char *str, char *target, char *targetendp,
		BREGEXP **rxp, char *msg)
{
TRACE1("BTrans(): %s\n", str);
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
		strcpy(msg,"no translate parameter");
		return -1;
	}
	
	int matched = trans(rx,target,targetendp,msg);
	return msg[0] == '\0' ? matched: -1;
}


int BSplit(char *str, char *target, char *targetendp,
		int limit, BREGEXP **rxp, char *msg)
{
TRACE1("BSplit(): %s\n", str);
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

void BRegfree(BREGEXP *rx)
{
TRACE1("BRegfree(): rx=0x%08x\n", rx);
	if (rx) {
		delete static_cast<bregonig*>(rx);
	}
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




int onig_err_to_bregexp_msg(int err_code, OnigErrorInfo* err_info, char *msg)
{
	int ret = -1;
	char *err_str = new (std::nothrow) char[ONIG_MAX_ERROR_MESSAGE_LEN];
	if (err_str != NULL) {
		ret = onig_error_code_to_str((UChar*) err_str, err_code, err_info);
		
		strncpy(msg, err_str, BREGEXP_MAX_ERROR_MESSAGE_LEN);
		msg[BREGEXP_MAX_ERROR_MESSAGE_LEN-1] = '\0';
		
		delete [] err_str;
	}
	return ret;
}


bregonig *compile_onig(const char *ptn, int plen, char *msg)
{
TRACE0("compile_onig()\n");
TRACE1("ptn:%s\n", ptn);
TRACE1("plen:%d\n", plen);
	char *parap = new (std::nothrow) char[plen+1];	// parameter copy
	if (parap == NULL) {
		strcpy(msg, "precompile out of space");
		return NULL;
	}
	memcpy(parap, ptn, plen+1);	// copy include null
	
	char type = 'm';		// default is match
	char *p = parap;
	char *pend = p + plen;
	char sep = '/';			// default separater
	if (*p != '/') {
		if (*p != 's' && *p != 'm' && memcmp(p,"tr",2) != 0) {
			strcpy(msg,"do not start 'm' or 's' or 'tr'");
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
	char *res = p;
	char *resend = NULL, *rp = NULL, *rpend = NULL;
	char prev = 0;
	while (*p != '\0') {
		if (iskanji(*p)) {
			prev = 0; p += 2;
			continue;
		}
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
		strcpy(msg, "unmatch separater");
		delete [] parap;
		return NULL;
	}
	
	if (!rpend)
		p = resend + 1;
	
	int flag = 0;
	OnigOptionType option = ONIG_OPTION_NONE;
	OnigEncoding enc = ONIG_ENCODING_ASCII;
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
			enc = ONIG_ENCODING_SJIS;
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
	TRACE1("parap: %s\r\n", parap);
	TRACE1("res: %s\r\n", res);
	TRACE1("resend: %s\r\n", resend);
	TRACE1("rp: %s\r\n", rp);
	TRACE1("rpend: %s\r\n", rpend);
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
		strcpy(msg, "out of space regexp");
		delete [] parap;
		return NULL;
	}
//	OnigSyntaxPerl_NG_EX.behavior |= ONIG_SYN_DIFFERENT_LEN_ALT_LOOK_BEHIND;
	OnigErrorInfo err_info;
	int err_code = onig_new(&rx->reg, (UChar*) res, (UChar*) resend,
			option, enc, /*ONIG_SYNTAX_PERL_NG*/&OnigSyntaxPerl_NG_EX, &err_info);
	
	if (err_code != ONIG_NORMAL) {
TRACE0("Error: onig_new()\n");
		onig_err_to_bregexp_msg(err_code, &err_info, msg);
		delete rx;
		delete [] parap;
		return NULL;
	}
	
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
			strcpy(msg, ex.what());
			return NULL;
		}
	}
	
TRACE1("rx:0x%08x\n", rx);
	return rx;
}


int regexec_onig(bregonig *rx, char *stringarg,
	register char *strend,	/* pointer to null at end of string */
	char *strbeg,	/* real beginning of string */
	int minend,		/* end of match must be at least minend after stringarg */
	int safebase,	/* no need to remember string in subbase */
	int one_shot,	/* if not match then break without proceed str pointer */
	char *msg)		/* fatal error message */
{
TRACE1("one_shot: %d\n", one_shot);
	int err_code;
	try {
		if (one_shot) {
			err_code = onig_match(rx->reg, (UChar*) strbeg, (UChar*) strend,
					(UChar*) stringarg, rx->region,
					ONIG_OPTION_NONE);
		} else {
			err_code = onig_search(rx->reg, (UChar*) strbeg, (UChar*) strend,
					(UChar*) stringarg, (UChar*) strend, rx->region,
					ONIG_OPTION_NONE);
		}
#if 1
	} catch (...) {	// catch NULL pointer exception. need /EHa option
OutputDebugString("bregonig.dll: fatal error\n");
		// Multithread BUG???
		// should be fixed
		return -1;
	}
#endif
	if (err_code >= 0) {
		/* FOUND */
		if (rx->startp) {
			delete [] rx->startp;
		}
		rx->nparens = rx->region->num_regs - 1;
		rx->startp = new (std::nothrow) char*[rx->region->num_regs * 2];
			/* allocate startp and endp together */
		if (rx->startp == NULL) {
			strcpy(msg, "out of space");
			return -1;
		}
		rx->endp = rx->startp + rx->region->num_regs;
		
		for (int i = 0; i < rx->region->num_regs; i++) {
			rx->startp[i] = strbeg/**/ + rx->region->beg[i];
			rx->endp[i] = strbeg/**/ + rx->region->end[i];
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


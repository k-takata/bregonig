/*
 *	bregonig.h
 */
/*
 *	Copyright (C) 2006-2011  K.Takata
 *
 *	You may distribute under the terms of either the GNU General Public
 *	License or the Artistic License, as specified in the perl_license.txt file.
 */


#ifndef BREGONIG_H_
#define BREGONIG_H_

#ifdef UNICODE
typedef DWORD TWORD;
#define BREGONIG_NS	unicode
#else
typedef WORD TWORD;
#define BREGONIG_NS	ansi
#endif

namespace BREGONIG_NS {

typedef struct repstr {
	int  count;		/* entry counter */
	TCHAR **startp;	/* start address  if <256 \digit	*/
	int  *dlen;		/* data length / backref num	*/
	TCHAR data[1];	/* data start	*/
	
	repstr() { count = 0; startp = 0; dlen = 0; }
	~repstr() { delete [] startp; delete [] dlen; }
	
	void init(int cnt) {
		count = cnt;		// default \digits count in string
		startp = new TCHAR*[cnt];
		dlen = new int[cnt];
	}
	
	bool is_normal_string(int i) {
		return ((startp[i] != NULL) && ((INT_PTR) startp[i] > 1));
	}
	bool is_backslash(int i) {
		return ((INT_PTR) startp[i] == 1);
	}
	
	static void *operator new(size_t cb, size_t data_size) {
		return ::operator new (cb + data_size * sizeof(TCHAR));
	}
	static void operator delete(void *p, size_t data_size) {
		::operator delete (p);
	}
} REPSTR;


enum pattern_type {
	PTN_ERROR = -1,
	PTN_MATCH = 0,
	PTN_SUBST,
	PTN_TRANS
};

struct bregonig : bregexp {
#if 0
	TCHAR *outp;		/* matched or substitute string start ptr   */
	TCHAR *outendp;		/* matched or substitute string end ptr     */
	int  splitctr;		/* split result counrer     */
	TCHAR **splitp;		/* split result pointer ptr     */
	int  rsv1;			/* reserved for external use    */
	TCHAR *parap;		/* parameter start ptr ie. "s/xxxxx/yy/gi"  */
	TCHAR *paraendp;	/* parameter end ptr     */
	TCHAR *transtblp;	/* translate table ptr   */
	TCHAR **startp;		/* match string start ptr   */
	TCHAR **endp;		/* match string end ptr     */
	int  nparens;		/* number of parentheses */
#endif
// external field end point
	int pmflags;
	
	regex_t *reg;
	OnigRegion *region;
	REPSTR *repstr;
	
	/* "s/pattern/replace/option" */
	TCHAR *patternp;	/* original pattern string */
	TCHAR *patternendp;	/* original pattern string end */
	TCHAR *prerepp;		/* original replace string */
	TCHAR *prerependp;	/* original replace string end */
	TCHAR *optionp;		/* original option string */
	TCHAR *optionendp;	/* original option string end */
	
	
	bregonig();
	~bregonig();
};


#define iskanji(c)	_ismbblead(c)

inline int is_char_pair(const TBYTE *s)
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

inline TWORD get_codepoint(const TBYTE *s)
{
#ifdef UNICODE
	return (((s[0] - 0xd800) << 10) | (s[1] - 0xdc00)) + 0x10000;
#else
	return (s[0] << 8) | s[1];
#endif
}

inline int set_codepoint(TWORD codepoint, TBYTE *s)
{
	TBYTE *t = s;
#ifdef UNICODE
	if (codepoint > 0xffff) {	// Surrogate Pair
		unsigned int c = codepoint - 0x10000;
		*s++ = (c >> 10) | 0xd800;
		codepoint = (c & 0x3ff) | 0xdc00;
	}
#else
	if (codepoint > 0xff) {
		*s++ = codepoint >> 8;
	}
#endif
	*s++ = (TBYTE) codepoint;
	return s - t;
}

// ASCII to TCHAR string
inline TCHAR *asc2tcs(TCHAR *dst, const char *src)
{
#ifdef UNICODE
	swprintf(dst, L"%hs", src);
	return dst;
#else
	return strcpy(dst, src);
#endif
}


#define BREGEXP_MAX_ERROR_MESSAGE_LEN	80

#define CALLBACK_KIND_REPLACE	0
#define SUBST_BUF_SIZE 256


int check_params(const TCHAR *target, const TCHAR *targetstartp,
		const TCHAR *targetendp, BREGEXP **rxp, TCHAR *msg);
int BMatch_s(TCHAR *str, TCHAR *target, TCHAR *targetstartp, TCHAR *targetendp,
		int one_shot,
		BREGEXP **rxp, TCHAR *msg);
int BSubst_s(TCHAR *str, TCHAR *target, TCHAR *targetstartp, TCHAR *targetendp,
		BREGEXP **rxp, TCHAR *msg, BCallBack callback);

int onig_err_to_bregexp_msg(int err_code, OnigErrorInfo* err_info, TCHAR *msg);


bregonig *recompile_onig(bregonig *rxold, pattern_type type,
		const TCHAR *ptn, TCHAR *msg);
bregonig *recompile_onig_ex(bregonig *rxold,
		pattern_type type,
		const TCHAR *patternp, const TCHAR *patternendp,
		const TCHAR *prerepp, const TCHAR *prerependp,
		const TCHAR *optionp, const TCHAR *optionendp,
		TCHAR *msg);

//bregonig *compile_onig(const TCHAR *ptn, int plen, TCHAR *msg);
REPSTR *compile_rep(bregonig *rx, const TCHAR *str, const TCHAR *strend);

int subst_onig(bregonig *rx, const TCHAR *target,
		const TCHAR *targetstartp, const TCHAR *targetendp,
		TCHAR *msg, BCallBack callback);
int split_onig(bregonig *rx, TCHAR *target, TCHAR *targetendp, int limit, TCHAR *msg);


int regexec_onig(bregonig *rx, const TCHAR *stringarg,
	const TCHAR *strend,	/* pointer to null at end of string */
	const TCHAR *strbeg,	/* real beginning of string */
	int minend,		/* end of match must be at least minend after stringarg */
	int safebase,	/* no need to remember string in subbase */
	int one_shot,	/* if not match then break without proceed str pointer */
	TCHAR *msg);	/* fatal error message */

int trans(bregonig *rx, TCHAR *target, TCHAR *targetendp, TCHAR *msg);

bregonig *trcomp(const TCHAR *res, const TCHAR *resend,
		const TCHAR *rp, const TCHAR *rpend,
		int flag, TCHAR *msg);


#define isALNUM(c)   (isascii(c) && (isalpha(c) || isdigit(c) || c == '_'))
#define isIDFIRST(c) (isascii(c) && (isalpha(c) || (c) == '_'))
#define isALPHA(c)   (isascii(c) && isalpha(c))
#define isSPACE(c)   (isascii(c) && isspace(c))
#define isDIGIT(c)   (isascii(c) && isdigit(c))
#define isXDIGIT(c)  (isascii(c) && isxdigit(c))
#define isUPPER(c)   (isascii(c) && isupper(c))
#define isLOWER(c)   (isascii(c) && islower(c))
#define toUPPER(c)   toupper(c)
#define toLOWER(c)   tolower(c)

#define PMf_USED		0x0001		/* pm has been used once already */
#define PMf_ONCE		0x0002		/* use pattern only once per reset */
#define PMf_SCANFIRST	0x0004		/* initial constant not anchored */
#define PMf_ALL			0x0008		/* initial constant is whole pat */
#define PMf_SKIPWHITE	0x0010		/* skip leading whitespace for split */
#define PMf_FOLD		0x0020		/* case insensitivity */
#define PMf_CONST		0x0040		/* subst replacement is constant */
#define PMf_KEEP		0x0080		/* keep 1st runtime pattern forever */
#define PMf_GLOBAL		0x0100		/* pattern had a g modifier */
#define PMf_RUNTIME		0x0200		/* pattern coming in on the stack */
#define PMf_EVAL		0x0400		/* evaluating replacement as expr */
#define PMf_WHITE		0x0800		/* pattern is \s+ */
#define PMf_MULTILINE	0x1000		/* assume multiple lines */
#define PMf_SINGLELINE	0x2000		/* assume single line */
#define PMf_KANJI		0x4000		/* KANJI mode */
#define PMf_EXTENDED	0x8000		/* chuck embedded whitespace */
#define PMf_SUBSTITUTE	0x010000	/* substitute */
#define PMf_TRANSLATE	0x020000	/* translate  */
#define PMf_TRANS_COMPLEMENT	0x040000	/* translate complement */
#define PMf_TRANS_DELETE		0x080000	/* translate delete */
#define PMf_TRANS_SQUASH		0x100000	/* translate squash */

} // namespace

#endif /* BREGONIG_H_ */

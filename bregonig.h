/*
 *	bregonig.h
 */
/*
 *	Copyright (C) 2006  K.Takata
 *
 *	You may distribute under the terms of either the GNU General Public
 *	License or the Artistic License, as specified in the perl_license.txt file.
 */


#ifndef BREGONIG_H_
#define BREGONIG_H_


typedef struct repstr {
	int  count;		/* entry counter */
	char **startp;	/* start address  if <256 \digit	*/
	int  *dlen;		/* data length	*/
	char data[1];	/* data start	*/
	
	repstr() { count = 0; startp = 0; dlen = 0; }
	~repstr() { delete [] startp; delete [] dlen; }
	
	void init(int cnt) {
		count = cnt;		// default \digits count in string
		startp = new char*[cnt];
		dlen = new int[cnt];
	}
	
	static void *operator new(size_t cb, size_t data_size) {
		return ::operator new (cb + data_size);
	}
	static void operator delete(void *p, size_t data_size) {
		::operator delete (p);
	}
} REPSTR;


struct bregonig : bregexp {
#if 0
	char *outp;			/* matched or substitute string start ptr   */
	char *outendp;		/* matched or substitute string end ptr     */
	int  splitctr;		/* split result counrer     */
	char **splitp;		/* split result pointer ptr     */
	int  rsv1;			/* reserved for external use    */
	char *parap;		/* parameter start ptr ie. "s/xxxxx/yy/gi"  */
	char *paraendp;		/* parameter end ptr     */
	char *transtblp;	/* translate table ptr   */
	char **startp;		/* match string start ptr   */
	char **endp;		/* match string end ptr     */
	int  nparens;		/* number of parentheses */
#endif
// external field end point
	int pmflags;
	
	regex_t *reg;
	OnigRegion *region;
	REPSTR *repstr;
	char *prerepp;		/* original replace string */
	char *prerependp;	/* original replace string end */
	int prelen;
	
	
	bregonig();
	~bregonig();
};


#define iskanji(c)	_ismbblead(c)

#define BREGEXP_MAX_ERROR_MESSAGE_LEN	80

#define CALLBACK_KIND_REPLACE	0
#define SUBST_BUF_SIZE 256


int onig_err_to_bregexp_msg(int err_code, OnigErrorInfo* err_info, char *msg);

bregonig *compile_onig(const char *ptn, int plen, char *msg);
REPSTR *compile_rep(bregonig *rx, const char *str, const char *strend);

int subst_onig(bregonig *rx, char *target, char *targetstartp, char *targetendp,
		char *msg, BCallBack callback);
int split_onig(bregonig *rx, char *target, char *targetendp, int limit, char *msg);


int regexec_onig(bregonig *rx, char *stringarg,
	register char *strend,	/* pointer to null at end of string */
	char *strbeg,	/* real beginning of string */
	int minend,		/* end of match must be at least minend after stringarg */
	int safebase,	/* no need to remember string in subbase */
	int one_shot,   /* if not match then break without proceed str pointer */
	char *msg);		/* fatal error message */


int trans(bregonig *rx, char *target, char *targetendp, char *msg);

bregonig *trcomp(char *res, char *resend, char *rp, char *rpend,
		int flag, char *msg);


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

#endif /* BREGONIG_H_ */

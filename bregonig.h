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


#define BREGONIG_VERSION_MAJOR	0
#define BREGONIG_VERSION_MINOR	1
#define BREGONIG_VERSION_PREFIX	""



typedef struct repstr {
	int  count;		/* entry counter */
	char **startp;	/* start address  if <256 \digit	*/
	int  *dlen;		/* data length	*/
	char data[1];	/* data start   */
	
	repstr() {}
	~repstr() { delete [] startp; delete [] dlen; }
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

int regexec_onig(bregonig *rx, char *stringarg,
	register char *strend,	/* pointer to null at end of string */
	char *strbeg,	/* real beginning of string */
	int minend,		/* end of match must be at least minend after stringarg */
	int safebase,	/* no need to remember string in subbase */
	int one_shot,   /* if not match then break without proceed str pointer */
	char *msg);		/* fatal error message */


#endif /* BREGONIG_H_ */

/*     bregexp.h      
	external use header file 
						1999.11.22  T.Baba
*/
/*
 *	2002.08.24	modified by K2
 *	2006.08.23	modified by K.Takata
 */


#ifdef _BREGEXP_
/* for internal use */
#define BREGEXPAPI	__declspec(dllexport) 
#define BREGCONST
#else
/* for external use */
#define BREGEXPAPI	__declspec(dllimport) 
#define BREGCONST	const
#endif

typedef struct bregexp {
        BREGCONST char *outp;   /* result string start ptr  */
        BREGCONST char *outendp;/* result string end ptr    */
        BREGCONST int  splitctr;/* split result counter     */
        BREGCONST char **splitp;/* split result pointer ptr     */
        int rsv1;               /* reserved for external use    */
        char *parap;            /* parameter start ptr ie. "s/xxxxx/yy/gi"  */
        char *paraendp;         /* parameter end ptr     */
        char *transtblp;        /* translate table ptr   */
        char **startp;          /* match string start ptr   */
        char **endp;            /* match string end ptr     */
        int nparens;            /* number of parentheses */
} BREGEXP;

#if defined(_BREGEXP_) || defined(_K2REGEXP_)
typedef int (__stdcall *BCallBack)(int, int, int);
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

#ifdef _K2REGEXP_
/* K2Editor */
BREGEXPAPI
int BMatch(char *str, char *target, char *targetstartp, char *targetendp,
		int one_shot,
		BREGEXP **rxp, char *msg);
BREGEXPAPI
int BSubst(char *str, char *target, char *targetstartp, char *targetendp,
		BREGEXP **rxp, char *msg, BCallBack callback);
#else
/* Original */
BREGEXPAPI
int BMatch(char *str, char *target, char *targetendp,
		BREGEXP **rxp, char *msg);
BREGEXPAPI
int BSubst(char *str, char *target, char *targetendp,
		BREGEXP **rxp, char *msg);

/* Sakura Editor */
BREGEXPAPI
int BMatchEx(char *str, char *targetbegp, char *target, char *targetendp,
		BREGEXP **rxp, char *msg);
BREGEXPAPI
int BSubstEx(char *str, char *targetbegp, char *target, char *targetendp,
		BREGEXP **rxp, char *msg);
#endif


BREGEXPAPI
int BTrans(char *str, char *target, char *targetendp,
		BREGEXP **rxp, char *msg);
BREGEXPAPI
int BSplit(char *str, char *target, char *targetendp,
		int limit, BREGEXP **rxp, char *msg);
BREGEXPAPI
void BRegfree(BREGEXP *rx);

BREGEXPAPI
char *BRegexpVersion(void);


#if defined(__cplusplus)
}
#endif


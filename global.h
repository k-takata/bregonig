// global.h

//	2006.08.23	modified by K.Takata


#define	KANJI

//#define KANJI_MODE (_prog->reganch & ROPT_KANJI)
#define KANJI_MODE kanji_mode_flag


typedef union any ANY;
union any {
    void*	any_ptr;
    int		any_i32;
    long	any_iv;
    long	any_long;
};

/**************************************************************************/
/* This regexp stuff is global since it always happens within 1 expr eval */
/**************************************************************************/

/* Current curly descriptor */
typedef struct curcur CURCUR;
struct curcur {
    int		parenfloor;	/* how far back to strip paren data */
    int		cur;		/* how many instances of scan we've matched */
    int		min;		/* the minimal number of scans to match */
    int		max;		/* the maximal number of scans to match */
    int		minmod;		/* whether to work our way up or down */
    char *	scan;		/* the thing to match */
    char *	next;		/* what has to match after it */
    char *	lastloc;	/* where we started matching this scan */
    CURCUR *	oldcc;		/* current curly before we started this one */
};


typedef struct global GLOBAL;
struct global {

ANY*	savestack;	/* to save non-local values on */
int		savestack_ix;
int		savestack_max;

char *	regprecomp;	/* uncompiled string. */
char *	regparse;	/* Input-scan pointer. */
char *	regxend;	/* End of input for compile */
int		regnpar;	/* () count. */
char *	regcode;	/* Code-emit pointer; &regdummy = don't. */
int		regsize;	/* Code size. */
int		regnaughty;	/* How bad is this pattern? */
int		regsawback;	/* Did we see \1, ...? */

char *	reginput;	/* String-input pointer. */
char *	regbol;		/* Beginning of input, for ^ check. */
char *	regeol;		/* End of input, for $ check. */
char **	regstartp;	/* Pointer to startp array. */
char **	regendp;	/* Ditto for endp. */
int *	reglastparen;	/* Similarly for lastparen. */
char *	regtill;	/* How far we are required to go. */
int		regflags;	/* are we folding, multilining? */
char	regprev;	/* char before regbol, \n if none */

// regcomp part
unsigned char char_bitmap[32];
// current regexp pointer
char* regexp_current;


// regexec.cpp and sv.cpp part
int multiline;
// regexec.cpp 
int kanji_mode_flag;

CURCUR* regcc;
};

#define savestack		globalp->savestack	
#define	savestack_ix	globalp->savestack_ix
#define	savestack_max	globalp->savestack_max

#define	regprecomp	globalp->regprecomp
#define	regparse	globalp->regparse
#define	regxend		globalp->regxend
#define	regnpar		globalp->regnpar
#define	regcode		globalp->regcode
#define	regsize		globalp->regsize
#define	regnaughty	globalp->regnaughty
#define	regsawback	globalp->regsawback

#define	reginput	globalp->reginput
#define	regbol		globalp->regbol
#define	regeol		globalp->regeol
#define	regstartp	globalp->regstartp
#define	regendp		globalp->regendp
#define	reglastparen	globalp->reglastparen
#define	regtill		globalp->regtill
#define	regflags	globalp->regflags
#define	regprev		globalp->regprev
#define char_bitmap	globalp->char_bitmap
#define multiline	globalp->multiline

#define regexp_current	globalp->regexp_current

#define regcc		globalp->regcc
#define kanji_mode_flag		globalp->kanji_mode_flag


/**************************************************************************/
/* end og global                                                          */
/**************************************************************************/


#define isALNUM(c)   (isascii(c) && (isalpha(c) || isdigit(c) || c == '_'))
#define isIDFIRST(c) (isascii(c) && (isalpha(c) || (c) == '_'))
#define isALPHA(c)   (isascii(c) && isalpha(c))
#define isSPACE(c)   (isascii(c) && isspace(c))
#define isDIGIT(c)   (isascii(c) && isdigit(c))
#define isUPPER(c)   (isascii(c) && isupper(c))
#define isLOWER(c)   (isascii(c) && islower(c))
#define toUPPER(c)   toupper(c)
#define toLOWER(c)   tolower(c)


/* definition	number	opnd?	meaning */
#define ANYOF_EXACT   128	/* sv */
#define ANYOF_FROMTO  129	/* sv */
#define ANYOF_ENDMARK 130	/* sv */
#define ANYOF_COMPL   131	/* sv */


#define ROPT_KANJI	8
#define ROPT_ANCH 1
#define ROPT_SKIP 2
#define ROPT_IMPLICIT 4


/* definition	number	opnd?	meaning */
#define	END	0	/* no	End of program. */
#define	BOL	1	/* no	Match "" at beginning of line. */
#define MBOL	2	/* no	Same, assuming multiline. */
#define SBOL	3	/* no	Same, assuming singleline. */
#define	EOL	4	/* no	Match "" at end of line. */
#define MEOL	5	/* no	Same, assuming multiline. */
#define SEOL	6	/* no	Same, assuming singleline. */
#define	ANY	7	/* no	Match any one character (except newline). */
#define	SANY	8	/* no	Match any one character. */
#define	ANYOF	9	/* sv	Match character in (or not in) this class. */
#define	CURLY	10	/* sv	Match this simple thing {n,m} times. */
#define	CURLYX	11	/* sv	Match this complex thing {n,m} times. */
#define	BRANCH	12	/* node	Match this alternative, or the next... */
#define	BACK	13	/* no	Match "", "next" ptr points backward. */
#define	EXACTLY	14	/* sv	Match this string (preceded by length). */
#define	NOTHING	15	/* no	Match empty string. */
#define	STAR	16	/* node	Match this (simple) thing 0 or more times. */
#define	PLUS	17	/* node	Match this (simple) thing 1 or more times. */
#define ALNUM	18	/* no	Match any alphanumeric character */
#define NALNUM	19	/* no	Match any non-alphanumeric character */
#define BOUND	20	/* no	Match "" at any word boundary */
#define NBOUND	21	/* no	Match "" at any word non-boundary */
#define SPACE	22	/* no	Match any whitespace character */
#define NSPACE	23	/* no	Match any non-whitespace character */
#define DIGIT	24	/* no	Match any numeric character */
#define NDIGIT	25	/* no	Match any non-numeric character */
#define REF	26	/* num	Match some already matched string */
#define	OPEN	27	/* num	Mark this point in input as start of #n. */
#define	CLOSE	28	/* num	Analogous to OPEN. */
#define MINMOD	29	/* no	Next operator is not greedy. */
#define GBOL	30	/* no	Matches where last m//g left off. */
#define IFMATCH	31	/* no	Succeeds if the following matches. */
#define UNLESSM	32	/* no	Fails if the following matches. */
#define SUCCEED	33	/* no	Return from a subroutine, basically. */
#define WHILEM	34	/* no	Do curly processing and see if rest matches. */

/*
 * Opcode notes:
 *
 * BRANCH	The set of branches constituting a single choice are hooked
 *		together with their "next" pointers, since precedence prevents
 *		anything being concatenated to any individual branch.  The
 *		"next" pointer of the last BRANCH in a choice points to the
 *		thing following the whole choice.  This is also where the
 *		final "next" pointer of each individual branch points; each
 *		branch starts with the operand node of a BRANCH node.
 *
 * BACK		Normal "next" pointers all implicitly point forward; BACK
 *		exists to make loop structures possible.
 *
 * STAR,PLUS	'?', and complex '*' and '+', are implemented as circular
 *		BRANCH structures using BACK.  Simple cases (one character
 *		per match) are implemented with STAR and PLUS for speed
 *		and to minimize recursive plunges.
 *
 * OPEN,CLOSE	...are numbered at compile time.
 */


#ifndef GLOBAL_VALUE_DEFINE
extern char regkind[];
#else
char regkind[] = {
	END,
	BOL,
	BOL,
	BOL,
	EOL,
	EOL,
	EOL,
	ANY,
	ANY,
	ANYOF,
	CURLY,
	CURLY,
	BRANCH,
	BACK,
	EXACTLY,
	NOTHING,
	STAR,
	PLUS,
	ALNUM,
	NALNUM,
	BOUND,
	NBOUND,
	SPACE,
	NSPACE,
	DIGIT,
	NDIGIT,
	REF,
	OPEN,
	CLOSE,
	MINMOD,
	BOL,
	BRANCH,
	BRANCH,
	END,
	WHILEM
};
#endif


/*
 * A node is one char of opcode followed by two chars of "next" pointer.
 * "Next" pointers are stored as two 8-bit pieces, high order first.  The
 * value is a positive offset from the opcode of the node containing it.
 * An operand, if any, simply follows the node.  (Note that much of the
 * code generation knows about this implicit relationship.)
 *
 * Using two bytes for the "next" pointer is vast overkill for most things,
 * but allows patterns to get big without disasters.
 *
 * [If REGALIGN is defined, the "next" pointer is always aligned on an even
 * boundary, and reads the offset directly as a short.  Also, there is no
 * special test to reverse the sign of BACK pointers since the offset is
 * stored negative.]
 */

#define REGALIGN

#define	OP(p)	(*(p))

#define NEXT(p) (*(short*)(p+1))
#define ARG1(p) (*(unsigned short*)(p+3))
#define ARG2(p) (*(unsigned short*)(p+5))

#define	OPERAND(p)	((p) + 3)

#define	NEXTOPER(p)	((p) + 4)
#define	PREVOPER(p)	((p) - 4)

#define MAGIC 0234

/*
 * Utility definitions.
 */
#ifndef CHARMASK
#define	UCHARAT(p)	((int)*(unsigned char *)(p))
#else
#define	UCHARAT(p)	((int)*(p)&CHARMASK)
#endif

#define FAIL(c)	throw(c)


#define PMf_USED	0x0001		/* pm has been used once already */
#define PMf_ONCE	0x0002		/* use pattern only once per reset */
#define PMf_SCANFIRST	0x0004		/* initial constant not anchored */
#define PMf_ALL		0x0008		/* initial constant is whole pat */
#define PMf_SKIPWHITE	0x0010		/* skip leading whitespace for split */
#define PMf_FOLD	0x0020		/* case insensitivity */
#define PMf_CONST	0x0040		/* subst replacement is constant */
#define PMf_KEEP	0x0080		/* keep 1st runtime pattern forever */
#define PMf_GLOBAL	0x0100		/* pattern had a g modifier */
#define PMf_RUNTIME	0x0200		/* pattern coming in on the stack */
#define PMf_EVAL	0x0400		/* evaluating replacement as expr */
#define PMf_WHITE	0x0800		/* pattern is \s+ */
#define PMf_MULTILINE	0x1000		/* assume multiple lines */
#define PMf_SINGLELINE	0x2000		/* assume single line */
#define PMf_KANJI	0x4000		/* KANJI mode */
#define PMf_EXTENDED	0x8000		/* chuck embedded whitespace */
#define PMf_SUBSTITUTE	0x010000	/* substitute */
#define PMf_TRANSLATE	0x020000	/* translate  */
#define PMf_TRANS_COMPLEMENT	0x040000	/* translate complement */
#define PMf_TRANS_DELETE	0x080000	/* translate delete */
#define PMf_TRANS_SQUASH	0x100000	/* translate squash */

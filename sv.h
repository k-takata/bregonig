/*    sv.h
 *
 *    Copyright (c) 1991-1994, Larry Wall
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */
//	2006.08.29	updated by K.Takata


#ifndef BREGONIG_NS
#ifdef UNICODE
#define BREGONIG_NS	unicode
#else
#define BREGONIG_NS	ansi
#endif
#endif

namespace BREGONIG_NS {

#ifndef TRUE
//#define BOOL int
#define TRUE 1
#define FALSE 0
typedef int BOOL;
#endif

/*
#define U16 unsigned short
#define U8 unsigned char
#define U32 unsigned int
*/
typedef unsigned short U16;
typedef unsigned char U8;
typedef unsigned int U32;


typedef enum {
	SVt_NULL,	/* 0 */
	SVt_IV,		/* 1 */
	SVt_NV,		/* 2 */
	SVt_RV,		/* 3 */
	SVt_PV,		/* 4 */
	SVt_PVIV,	/* 5 */
	SVt_PVNV,	/* 6 */
	SVt_PVMG,	/* 7 */
	SVt_PVBM,	/* 8 */
	SVt_PVLV,	/* 9 */
	SVt_PVAV,	/* 10 */
	SVt_PVHV,	/* 11 */
	SVt_PVCV,	/* 12 */
	SVt_PVGV,	/* 13 */
	SVt_PVFM,	/* 14 */
	SVt_PVIO	/* 15 */
} svtype;

/* Using C's structural equivalence to help emulate C++ inheritance here... */
struct sv {
//    void*	sv_any;		/* pointer to something */
    int		sv_refcnt;	/* how many references to us */
    int		sv_flags;	/* what we are */
//    struct xpvbm     sv_xpv;		/* struct xpvbm */ 
    TCHAR*	xpv_pv;		/* pointer to malloced string */
    int		xpv_cur;		/* length of xpv_pv as a C string */
    int		xpv_len;		/* allocated size */
    int		xbm_useful;	/* is this constant pattern being useful? */
    int		xbm_previous;	/* how many characters in string before rare? */
    TCHAR	xbm_rare;	/* rarest character in string */
};

typedef struct sv SV;


#define SvFLAGS(sv)	(sv)->sv_flags

#define SvREFCNT(sv)	(sv)->sv_refcnt
#define SvREFCNT_inc(sv)	((Sv = (SV*)(sv)), \
				    (Sv && ++SvREFCNT(Sv)), (SV*)Sv)
#define SvREFCNT_dec(sv)	sv_free((SV*)sv)

#define SVTYPEMASK	0xff
#define SvTYPE(sv)	((sv)->sv_flags & SVTYPEMASK)

#define SvUPGRADE(sv, mt) (SvTYPE(sv) >= mt || sv_upgrade(sv, mt))

#define SVs_PADBUSY	0x00000100	/* reserved for tmp or my already */
#define SVs_PADTMP	0x00000200	/* in use as tmp */
#define SVs_PADMY	0x00000400	/* in use a "my" variable */
#define SVs_TEMP	0x00000800	/* string is stealable? */
#define SVs_OBJECT	0x00001000	/* is "blessed" */
#define SVs_GMG		0x00002000	/* has magical get method */
#define SVs_SMG		0x00004000	/* has magical set method */
#define SVs_RMG		0x00008000	/* has random magical methods */

#define SVf_IOK		0x00010000	/* has valid public integer value */
#define SVf_NOK		0x00020000	/* has valid public numeric value */
#define SVf_POK		0x00040000	/* has valid public pointer value */
#define SVf_ROK		0x00080000	/* has a valid reference pointer */

#define SVf_FAKE	0x00100000	/* glob or lexical is just a copy */
#define SVf_OOK		0x00200000	/* has valid offset value */
#define SVf_BREAK	0x00400000	/* refcnt is artificially low */
#define SVf_READONLY	0x00800000	/* may not be modified */

#define SVf_THINKFIRST	(SVf_READONLY|SVf_ROK)

#define SVp_IOK		0x01000000	/* has valid non-public integer value */
#define SVp_NOK		0x02000000	/* has valid non-public numeric value */
#define SVp_POK		0x04000000	/* has valid non-public pointer value */
#define SVp_SCREAM	0x08000000	/* has been studied? */

#define SVf_OK		(SVf_IOK|SVf_NOK|SVf_POK|SVf_ROK| \
			 SVp_IOK|SVp_NOK|SVp_POK)


#define PRIVSHIFT 8

/* Some private flags. */

#define SVpfm_COMPILED	0x80000000

#define SVpbm_VALID	0x80000000
#define SVpbm_CASEFOLD	0x40000000
#define SVpbm_TAIL	0x20000000
#define SVphv_SHAREKEYS 0x20000000	/* keys live on shared string table */
#define SVphv_LAZYDEL	0x40000000	/* entry in xhv_eiter must be deleted */







/* The following macros define implementation-independent predicates on SVs. */

#define SvNIOK(sv)		(SvFLAGS(sv) & (SVf_IOK|SVf_NOK))
#define SvNIOKp(sv)		(SvFLAGS(sv) & (SVp_IOK|SVp_NOK))
#define SvNIOK_off(sv)		(SvFLAGS(sv) &= ~(SVf_IOK|SVf_NOK| \
						  SVp_IOK|SVp_NOK))

#define SvOK(sv)		(SvFLAGS(sv) & SVf_OK)

#define SvOK_off(sv)		(SvFLAGS(sv) &=	~SVf_OK, SvOOK_off(sv))

#define SvOKp(sv)		(SvFLAGS(sv) & (SVp_IOK|SVp_NOK|SVp_POK))
#define SvIOKp(sv)		(SvFLAGS(sv) & SVp_IOK)
#define SvIOKp_on(sv)		(SvOOK_off(sv), SvFLAGS(sv) |= SVp_IOK)
#define SvNOKp(sv)		(SvFLAGS(sv) & SVp_NOK)
#define SvNOKp_on(sv)		(SvFLAGS(sv) |= SVp_NOK)
#define SvPOKp(sv)		(SvFLAGS(sv) & SVp_POK)
#define SvPOKp_on(sv)		(SvFLAGS(sv) |= SVp_POK)

#define SvIOK(sv)		(SvFLAGS(sv) & SVf_IOK)
#define SvIOK_on(sv)		(SvOOK_off(sv), \
				    SvFLAGS(sv) |= (SVf_IOK|SVp_IOK))
#define SvIOK_off(sv)		(SvFLAGS(sv) &= ~(SVf_IOK|SVp_IOK))
#define SvIOK_only(sv)		(SvOOK_off(sv), SvOK_off(sv), \
				    SvFLAGS(sv) |= (SVf_IOK|SVp_IOK))

#define SvNOK(sv)		(SvFLAGS(sv) & SVf_NOK)
#define SvNOK_on(sv)		(SvFLAGS(sv) |= (SVf_NOK|SVp_NOK))
#define SvNOK_off(sv)		(SvFLAGS(sv) &= ~(SVf_NOK|SVp_NOK))
#define SvNOK_only(sv)		(SvOK_off(sv), \
				    SvFLAGS(sv) |= (SVf_NOK|SVp_NOK))

#define SvPOK(sv)		(SvFLAGS(sv) & SVf_POK)
#define SvPOK_on(sv)		(SvFLAGS(sv) |= (SVf_POK|SVp_POK))
#define SvPOK_off(sv)		(SvFLAGS(sv) &= ~(SVf_POK|SVp_POK))

#define SvPOK_only(sv)            (SvFLAGS(sv) &= ~SVf_OK, \
				    SvFLAGS(sv) |= (SVf_POK|SVp_POK))

#define SvOOK(sv)		(SvFLAGS(sv) & SVf_OOK)
#define SvOOK_on(sv)		(SvIOK_off(sv), SvFLAGS(sv) |= SVf_OOK)
#define SvOOK_off(sv)		(SvOOK(sv) && sv_backoff(sv))

#define SvFAKE(sv)		(SvFLAGS(sv) & SVf_FAKE)
#define SvFAKE_on(sv)		(SvFLAGS(sv) |= SVf_FAKE)
#define SvFAKE_off(sv)		(SvFLAGS(sv) &= ~SVf_FAKE)

#define SvROK(sv)		(SvFLAGS(sv) & SVf_ROK)
#define SvROK_on(sv)		(SvFLAGS(sv) |= SVf_ROK)





#define SvSCREAM(sv)		(SvFLAGS(sv) & SVp_SCREAM)
#define SvSCREAM_on(sv)		(SvFLAGS(sv) |= SVp_SCREAM)
#define SvSCREAM_off(sv)	(SvFLAGS(sv) &= ~SVp_SCREAM)

#define SvCOMPILED(sv)		(SvFLAGS(sv) & SVpfm_COMPILED)
#define SvCOMPILED_on(sv)	(SvFLAGS(sv) |= SVpfm_COMPILED)
#define SvCOMPILED_off(sv)	(SvFLAGS(sv) &= ~SVpfm_COMPILED)

#define SvTAIL(sv)		(SvFLAGS(sv) & SVpbm_TAIL)
#define SvTAIL_on(sv)		(SvFLAGS(sv) |= SVpbm_TAIL)
#define SvTAIL_off(sv)		(SvFLAGS(sv) &= ~SVpbm_TAIL)

#define SvCASEFOLD(sv)		(SvFLAGS(sv) & SVpbm_CASEFOLD)
#define SvCASEFOLD_on(sv)	(SvFLAGS(sv) |= SVpbm_CASEFOLD)
#define SvCASEFOLD_off(sv)	(SvFLAGS(sv) &= ~SVpbm_CASEFOLD)

#define SvVALID(sv)		(SvFLAGS(sv) & SVpbm_VALID)
#define SvVALID_on(sv)		(SvFLAGS(sv) |= SVpbm_VALID)
#define SvVALID_off(sv)		(SvFLAGS(sv) &= ~SVpbm_VALID)


#define SvPVX(sv)  sv->xpv_pv
#define SvPVXx(sv) SvPVX(sv)
#define SvCUR(sv) sv->xpv_cur
#define SvLEN(sv) sv->xpv_len
#define SvLENx(sv) SvLEN(sv)
#define SvEND(sv)	(sv->xpv_pv + sv->xpv_cur)
#define SvENDx(sv) ((Sv = (sv)), SvEND(Sv))

#define BmRARE(sv)	sv->xbm_rare
#define BmUSEFUL(sv)	sv->xbm_useful
#define BmPREVIOUS(sv)	sv->xbm_previous





#define SAVEt_ITEM	0
#define SAVEt_SV	1
#define SAVEt_AV	2
#define SAVEt_HV	3
#define SAVEt_INT	4
#define SAVEt_LONG	5
#define SAVEt_I32	6
#define SAVEt_IV	7
#define SAVEt_SPTR	8
#define SAVEt_APTR	9
#define SAVEt_HPTR	10
#define SAVEt_PPTR	11
#define SAVEt_NSTAB	12
#define SAVEt_SVREF	13
#define SAVEt_GP	14
#define SAVEt_FREESV	15
#define SAVEt_FREEOP	16
#define SAVEt_FREEPV	17
#define SAVEt_CLEARSV	18
#define SAVEt_DELETE	19
#define SAVEt_DESTRUCTOR 20
#define SAVEt_REGCONTEXT 21

#define SSCHECK(need) if (savestack_ix + need > savestack_max) savestack_grow()
#define SSPUSHINT(i) (savestack[savestack_ix++].any_i32 = (int)(i))
#define SSPUSHLONG(i) (savestack[savestack_ix++].any_long = (long)(i))
#define SSPUSHIV(i) (savestack[savestack_ix++].any_iv = (IV)(i))
#define SSPUSHPTR(p) (savestack[savestack_ix++].any_ptr = (void*)(p))
#define SSPUSHDPTR(p) (savestack[savestack_ix++].any_dptr = (p))
#define SSPOPINT (savestack[--savestack_ix].any_i32)
#define SSPOPLONG (savestack[--savestack_ix].any_long)
#define SSPOPIV (savestack[--savestack_ix].any_iv)
#define SSPOPPTR (savestack[--savestack_ix].any_ptr)
#define SSPOPDPTR (savestack[--savestack_ix].any_dptr)


TCHAR *ninstr(register TCHAR *big,register TCHAR *bigend,TCHAR *little,
	TCHAR *lend,int kmode);
//TCHAR * fbm_instr(unsigned TCHAR*,register unsigned TCHAR *,SV*,int mline,int kmode);
TCHAR * fbm_instr(TBYTE*,register TBYTE *,SV*,int mline,int kmode);
BOOL sv_upgrade(register SV*, int);// sv.spp
SV *newSVpv(TCHAR*,int);//sv.cpp
void sv_catpvn(register SV*,register const TCHAR*,register int);//sv.cpp
void sv_setpvn(register SV*,register const TCHAR*,register int);//sv.cpp
void sv_setsv(SV*,SV*);//sv.cpp

void fbm_compile (SV* sv, int iflag);
void	sv_free (SV* sv);
TCHAR*	savepvn (TCHAR* sv, int len);
unsigned long scan_hex(const TCHAR*, int,int*);
unsigned long scan_oct(const TCHAR*, int,int*);

#ifndef iskanji
#include <mbstring.h>
//BOOL iskanji(int c);
#define iskanji(c)	_ismbblead(c)
#endif

} // namespace

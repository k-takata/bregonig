//
//  sv.cc
//
////////////////////////////////////////////////////////////////////////////////
//  1999.11.24   update by Tatsuo Baba
//  2006.08.29   update by K.Takata
//
//  You may distribute under the terms of either the GNU General Public
//  License or the Artistic License, as specified in the perl_license.txt file.
////////////////////////////////////////////////////////////////////////////////


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <tchar.h>
#include <new>
#include "mem_vc6.h"

#ifndef UNICODE
#define KANJI
#endif

/* return values of kpart */
#define KPART_KANJI_1 1 /* kanji 1st byte */
#define KPART_KANJI_2 2 /* kanji 2nd byte */
#define KPART_OTHER   0 /* other (ASCII) */


#include "sv.h"

//using namespace BREGONIG_NS;
namespace BREGONIG_NS {

int   kpart(TCHAR *pLim,TCHAR *pChr);


#if 0
static
unsigned char sjis_tab[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 0x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 1x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 2x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 3x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 4x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 5x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 6x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 7x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* 8x */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* 9x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* Ax */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* Bx */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* Cx */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* Dx */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* Ex */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,	/* Fx */
};
#endif


//#  define SvGROW(sv,len) (SvLEN(sv) < (len) ? sv_grow(sv,len) : SvPVX(sv))
//  g++ cause error    converting to `void' from `char *'   1999/11/22
//     so   add (void)
#  define SvGROW(sv,len) (SvLEN(sv) < (len) ? sv_grow(sv,len) : (void)SvPVX(sv))


#if 0
static TBYTE fold[] = {	/* fast case folding table */
	0,	1,	2,	3,	4,	5,	6,	7,
	8,	9,	10,	11,	12,	13,	14,	15,
	16,	17,	18,	19,	20,	21,	22,	23,
	24,	25,	26,	27,	28,	29,	30,	31,
	32,	33,	34,	35,	36,	37,	38,	39,
	40,	41,	42,	43,	44,	45,	46,	47,
	48,	49,	50,	51,	52,	53,	54,	55,
	56,	57,	58,	59,	60,	61,	62,	63,
	64,	'a',	'b',	'c',	'd',	'e',	'f',	'g',
	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
	'p',	'q',	'r',	's',	't',	'u',	'v',	'w',
	'x',	'y',	'z',	91,	92,	93,	94,	95,
	96,	'A',	'B',	'C',	'D',	'E',	'F',	'G',
	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
	'X',	'Y',	'Z',	123,	124,	125,	126,	127,
	128,	129,	130,	131,	132,	133,	134,	135,
	136,	137,	138,	139,	140,	141,	142,	143,
	144,	145,	146,	147,	148,	149,	150,	151,
	152,	153,	154,	155,	156,	157,	158,	159,
	160,	161,	162,	163,	164,	165,	166,	167,
	168,	169,	170,	171,	172,	173,	174,	175,
	176,	177,	178,	179,	180,	181,	182,	183,
	184,	185,	186,	187,	188,	189,	190,	191,
	192,	193,	194,	195,	196,	197,	198,	199,
	200,	201,	202,	203,	204,	205,	206,	207,
	208,	209,	210,	211,	212,	213,	214,	215,
	216,	217,	218,	219,	220,	221,	222,	223,	
	224,	225,	226,	227,	228,	229,	230,	231,
	232,	233,	234,	235,	236,	237,	238,	239,
	240,	241,	242,	243,	244,	245,	246,	247,
	248,	249,	250,	251,	252,	253,	254,	255
};

static TBYTE freq[] = {	/* letter frequencies for mixed English/C */
	1,	2,	84,	151,	154,	155,	156,	157,
	165,	246,	250,	3,	158,	7,	18,	29,
	40,	51,	62,	73,	85,	96,	107,	118,
	129,	140,	147,	148,	149,	150,	152,	153,
	255,	182,	224,	205,	174,	176,	180,	217,
	233,	232,	236,	187,	235,	228,	234,	226,
	222,	219,	211,	195,	188,	193,	185,	184,
	191,	183,	201,	229,	181,	220,	194,	162,
	163,	208,	186,	202,	200,	218,	198,	179,
	178,	214,	166,	170,	207,	199,	209,	206,
	204,	160,	212,	216,	215,	192,	175,	173,
	243,	172,	161,	190,	203,	189,	164,	230,
	167,	248,	227,	244,	242,	255,	241,	231,
	240,	253,	169,	210,	245,	237,	249,	247,
	239,	168,	252,	251,	254,	238,	223,	221,
	213,	225,	177,	197,	171,	196,	159,	4,
	5,	6,	8,	9,	10,	11,	12,	13,
	14,	15,	16,	17,	19,	20,	21,	22,
	23,	24,	25,	26,	27,	28,	30,	31,
	32,	33,	34,	35,	36,	37,	38,	39,
	41,	42,	43,	44,	45,	46,	47,	48,
	49,	50,	52,	53,	54,	55,	56,	57,
	58,	59,	60,	61,	63,	64,	65,	66,
	67,	68,	69,	70,	71,	72,	74,	75,
	76,	77,	78,	79,	80,	81,	82,	83,
	86,	87,	88,	89,	90,	91,	92,	93,
	94,	95,	97,	98,	99,	100,	101,	102,
	103,	104,	105,	106,	108,	109,	110,	111,
	112,	113,	114,	115,	116,	117,	119,	120,
	121,	122,	123,	124,	125,	126,	127,	128,
	130,	131,	132,	133,	134,	135,	136,	137,
	138,	139,	141,	142,	143,	144,	145,	146
};
#endif



typedef int STRLEN;

void sv_free(register SV *sv)
{
	if (sv->xpv_pv)
		delete [] sv->xpv_pv;
	delete sv;
}


#if 0
TCHAR *fbm_instr(
TBYTE *big,
register TBYTE *bigend,
SV *littlestr,int mline,int kmode)
{
    register TBYTE *s;
    register int tmp;
    register int littlelen;
    register TBYTE *little;
    register TBYTE *table;
    register TBYTE *olds;
    register TBYTE *oldlittle;
#ifdef	KANJI
    TBYTE *tops = big;
#endif

    if (SvTYPE(littlestr) != SVt_PVBM || !SvVALID(littlestr)) {
		int len = SvCUR(littlestr);
		TCHAR *l = SvPVX(littlestr);
		if (!len)
		    return (TCHAR*)big;
		return ninstr((TCHAR*)big,(TCHAR*)bigend, l, l + len,kmode);
    }





    littlelen = SvCUR(littlestr);
    if (SvTAIL(littlestr) && !mline) {	/* tail anchored? */
	if (littlelen > bigend - big)
	    return NULL;
	little = (TBYTE*)SvPVX(littlestr);
	if (SvCASEFOLD(littlestr)) {	/* oops, fake it */
	    big = bigend - littlelen;		/* just start near end */
	    if (bigend[-1] == '\n' && little[littlelen-1] != '\n')
		big--;
	}
	else {
	    s = bigend - littlelen;
#ifdef	KANJI
	    if (*s == *little
//		&& (!(hints & HINT_KANJI_STRING)
//		    || kpart((TCHAR *)tops,(TCHAR *)s)!=KPART_KANJI_2)
		&& (kpart((TCHAR *)tops,(TCHAR *)s)!=KPART_KANJI_2)
		&& memcmp(s,little,littlelen)==0)
#else
	    if (*s == *little && memcmp((TCHAR*)s,(TCHAR*)little,littlelen*sizeof(TCHAR))==0)
#endif
		return (TCHAR*)s;		/* how sweet it is */
	    else if (bigend[-1] == '\n' && little[littlelen-1] != '\n'
	      && s > big) {
		    s--;
#ifdef	KANJI
		if (*s == *little
//		    && (!(hints & HINT_KANJI_STRING)
//			|| kpart((TCHAR *)tops,(TCHAR *)s)!=KPART_KANJI_2)
		    && (
			   kpart((TCHAR *)tops,(TCHAR *)s)!=KPART_KANJI_2)
		    && memcmp(s,little,littlelen)==0)
#else
		if (*s == *little && memcmp((TCHAR*)s,(TCHAR*)little,littlelen*sizeof(TCHAR))==0)
#endif
		    return (TCHAR*)s;
	    }
	    return NULL;
	}
    }
    table = (TBYTE*)(SvPVX(littlestr) + littlelen + 1);
    if (--littlelen >= bigend - big)
	return NULL;
    s = big + littlelen;
    oldlittle = little = table - 2;
    if (SvCASEFOLD(littlestr)) {	/* case insensitive? */
	if (s < bigend) {
	  top1:
	    /*SUPPRESS 560*/
	    if (tmp = table[*s]) {
#ifdef POINTERRIGOR
		if (bigend - s > tmp) {
		    s += tmp;
		    goto top1;
		}
#else
		if ((s += tmp) < bigend)
		    goto top1;
#endif
		return NULL;
	    }
	    else {
		tmp = littlelen;	/* less expensive than calling strncmp() */
		olds = s;
		while (tmp--) {
		    if (*--s == *--little || fold[*s] == *little)
			continue;
		    s = olds + 1;	/* here we pay the price for failure */
		    little = oldlittle;
		    if (s < bigend)	/* fake up continue to outer loop */
			goto top1;
		    return NULL;
		}
#ifdef	KANJI
//		if ((hints & HINT_KANJI_STRING)
//		    && kpart((TCHAR *)tops,(TCHAR *)s) == 2) {
		if (
		       kpart((TCHAR *)tops,(TCHAR *)s) == 2) {
			s = olds + 1;
			little = oldlittle;
			if (s < bigend)
				goto top1;
			return NULL;
		}
#endif
		return (TCHAR *)s;
	    }
	}
    }
    else {
	if (s < bigend) {
	  top2:
	    /*SUPPRESS 560*/
	    if (tmp = table[*s]) {
#ifdef POINTERRIGOR
		if (bigend - s > tmp) {
		    s += tmp;
		    goto top2;
		}
#else
		if ((s += tmp) < bigend)
		    goto top2;
#endif
		return NULL;
	    }
	    else {
		tmp = littlelen;	/* less expensive than calling strncmp() */
		olds = s;
		while (tmp--) {
		    if (*--s == *--little)
			continue;
		    s = olds + 1;	/* here we pay the price for failure */
		    little = oldlittle;
		    if (s < bigend)	/* fake up continue to outer loop */
			goto top2;
		    return NULL;
		}
#ifdef	KANJI
		if (kpart((TCHAR *)tops,(TCHAR *)s) == 2) {
		    s = olds + 1;
		    little = oldlittle;
 		    if (s < bigend)
		      goto top1;
		    return NULL;
		}
#endif
		return (TCHAR *)s;
	    }
	}
    }
    return NULL;
}
#endif


////////////////////////////////////////////////////////////////////////////
/* Note: sv_setsv() should not be called with a source string that needs
 * to be reused, since it may destroy the source string if it is marked
 * as temporary.
 */

void sv_setpvn(register SV *sv, register TCHAR *ptr, register STRLEN len);



void sv_grow(SV* sv,int len)
{
	len += 512;
	TCHAR *ptr = new (std::nothrow) TCHAR[len];
	if (ptr == NULL)
		throw std::bad_alloc("lack of memory");

	memcpy(ptr,sv->xpv_pv,sv->xpv_cur*sizeof(TCHAR));
	ptr[sv->xpv_cur] = '\0';
	if (sv->xpv_pv)
		delete [] sv->xpv_pv;
	sv->xpv_pv = ptr;
	sv->xpv_len = len;
}



void
sv_setsv(SV *dstr, register SV *sstr)
{
    if (sstr == dstr)
		return;
	int len = sstr->xpv_cur;
    SvGROW(dstr, len +1);

	memcpy(dstr->xpv_pv,sstr->xpv_pv,len*sizeof(TCHAR));
	dstr->xpv_cur = len;
    *SvEND(dstr) = '\0';
    dstr->sv_flags = sstr->sv_flags;
}


void sv_catpvn(SV* sv,TCHAR*ptr,int len)
{
    SvGROW(sv, sv->xpv_cur + len +1);

	memcpy(sv->xpv_pv + sv->xpv_cur,ptr,len*sizeof(TCHAR));
	sv->xpv_cur += len;
    *SvEND(sv) = '\0';
}


void sv_setpvn(register SV *sv, register TCHAR *ptr, register STRLEN len)
{
    SvGROW(sv, len + 1 < 512 ? 512:len + 1);
    memcpy(SvPVX(sv),ptr,len*sizeof(TCHAR));
//    SvCUR_set(sv, len);
	sv->xpv_cur = len;
    *SvEND(sv) = '\0';
    (void)SvPOK_only(sv);		/* validate pointer */
}

BOOL sv_upgrade(register SV* sv, int mt)
{
    if (SvTYPE(sv) == mt)
	return TRUE;
	sv->sv_flags |= SVt_PVBM;
	return TRUE;
}



#if 0
void fbm_compile(SV *sv, int iflag)
{
    register TBYTE *s;
    register TBYTE *table;
    register int i;
    register int len = SvCUR(sv);
    int rarest = 0;
    int frequency = 256;

    if (len > 255)
	return;			/* can't have offsets that big */
    SvGROW(sv,len+258);
    table = (TBYTE*)(SvPVX(sv) + len + 1);
    s = table - 2;
    for (i = 0; i < 256; i++) {
	table[i] = len;
    }
    i = 0;
    while (s >= (TBYTE*)(SvPVX(sv)))
    {
	if (table[*s] == len) {
#ifndef pdp11
	    if (iflag)
		table[*s] = table[fold[*s]] = i;
#else
	    if (iflag) {
		int j;
		j = fold[*s];
		table[j] = i;
		table[*s] = i;
	    }
#endif /* pdp11 */
	    else
		table[*s] = i;
	}
	s--,i++;
    }
    sv_upgrade(sv, SVt_PVBM);
//baba    sv_magic(sv, Nullsv, 'B', Nullch, 0);			/* deep magic */
    SvVALID_on(sv);

    s = (TBYTE*)(SvPVX(sv));		/* deeper magic */
    if (iflag) {
	register int tmp, foldtmp;
	SvCASEFOLD_on(sv);
	if (SvCASEFOLD(sv)) {
		int ff = 1;
		ff = 3;
	}
	for (i = 0; i < len; i++) {
	    tmp=freq[s[i]];
	    foldtmp=freq[fold[s[i]]];
	    if (tmp < frequency && foldtmp < frequency) {
		rarest = i;
		/* choose most frequent among the two */
		frequency = (tmp > foldtmp) ? tmp : foldtmp;
	    }
	}
    }
    else {
	for (i = 0; i < len; i++) {
	    if (freq[s[i]] < frequency) {
		rarest = i;
		frequency = freq[s[i]];
	    }
	}
    }
    BmRARE(sv) = s[rarest];
    BmPREVIOUS(sv) = rarest;
}
#endif


SV *newSVpv(TCHAR *s, STRLEN len)
{
    register SV *sv;
    sv = new (std::nothrow) SV;
	if (sv == NULL)
		throw std::bad_alloc("lack of memory");
	memset(sv,0,sizeof(SV));
	
    SvREFCNT(sv) = 1;
    SvFLAGS(sv) = 0;
    if (!len)
	len = _tcslen(s);
    sv_setpvn(sv,s,len);
    return sv;
}

int kpart(TCHAR *pLim, TCHAR *pChr)
{
    register TCHAR *p = pChr - 1;
    register int ct = 0;

    if (NULL == pLim || NULL == pChr) return 0 ;

    while (p >= pLim && iskanji(*p)) {
	p--;
	ct++;
    }
    return (ct & 1) ? 2 : iskanji(*pChr);
}



/* same as instr but allow embedded nulls */

TCHAR *
ninstr(
register TCHAR *big,
register TCHAR *bigend,
TCHAR *little,
TCHAR *lend,int kmode)
{
    register TCHAR *s, *x;
    register int first = *little;
    register TCHAR *littleend = lend;

    if (!first && little >= littleend)
	return big;
    if (bigend - big < littleend - little)
	return NULL;
    bigend -= littleend - little++;
	if (kmode) goto kproc;
    while (big <= bigend) {
	if (*big++ != first)
	    continue;
	
	for (x=big,s=little; s < littleend; /**/ ) {
		if (*s++ != *x++) {
		s--;
		break;
	    }
	}
	if (s >= littleend)
	    return big-1;
    }
    return NULL;

kproc:
	int k = 0;
    while (big <= bigend) {
	if (k) {
		big++; k = 0;
		continue;
	}
	k = iskanji(*big);
	if (*big++ != first)
	    continue;
	
	for (x=big,s=little; s < littleend; /**/ ) {
		if (*s++ != *x++) {
		s--;
		break;
	    }
	}
	if (s >= littleend)
	    return big-1;
    }
    return NULL;


}


#if 0
BOOL iskanji(int c) {
	return sjis_tab[c & 0xff];
}
#endif

} // namespace

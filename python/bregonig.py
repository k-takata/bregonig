# -*- coding: utf-8 -*-

"""Using bregonig.dll/BREGEXP.DLL regular expression DLL.

This is a wrapper for bregonig.dll/BREGEXP.DLL regular expression DLL.
"""

from ctypes import *

__all__ = ["BREGEXP", "BRegexpVersion",
           "BMatch", "BSubst",
           "BMatchEx", "BSubstEx",
           "BTrans", "BSplit", "BRegfree",
           "LoadDLL", "LoadBregonig", "LoadBregexp"]


if sizeof(c_long) == sizeof(c_void_p):
    INT_PTR = c_long
elif sizeof(c_longlong) == sizeof(c_void_p):
    INT_PTR = c_longlong


class BREGEXP(Structure):
    """BREGEXP structure"""
    _fields_ = [
        ("outp",        c_void_p),
        ("outendp",     c_void_p),
        ("splitctr",    c_int),
        ("splitp",      POINTER(c_void_p)),
        ("rsv1",        INT_PTR),
        ("parap",       c_void_p),
        ("paraendp",    c_void_p),
        ("transtblp",   c_void_p),
        ("startp",      POINTER(c_void_p)),
        ("endp",        POINTER(c_void_p)),
        ("nparens",     c_int)
    ]

# function pointers
_BRegexpVersion = None
_BMatch = None
_BSubst = None
_BMatchEx = None
_BSubstEx = None
_BTrans = None
_BSplit = None
_BRegfree = None


def BRegexpVersion():
    """Return version string of the regular expression DLL."""
    return _BRegexpVersion()

def BMatch(str, target, targetendp, rxp, msg):
    return _BMatch(str, target, targetendp, rxp, msg)

def BMatchEx(str, targetbegp, target, targetendp, rxp, msg):
    if _BMatchEx is None:
        # fall back
        return _BMatch(str, target, targetendp, rxp, msg)
    else:
        return _BMatchEx(str, targetbegp, target, targetendp, rxp, msg)

def BSubst(str, target, targetendp, rxp, msg):
    return _BSubst(str, target, targetendp, rxp, msg)

def BSubstEx(str, targetbegp, target, targetendp, rxp, msg):
    if _BSubstEx is None:
        # fall back
        return _BSubst(str, target, targetendp, rxp, msg)
    else:
        return _BSubstEx(str, targetbegp, target, targetendp, rxp, msg)

def BTrans(str, target, targetendp, rxp, msg):
    return _BTrans(str, target, targetendp, rxp, msg)

def BSplit(str, target, targetendp, limit, rxp, msg):
    return _BSplit(str, target, targetendp, limit, rxp, msg)

def BRegfree(rxp):
    return _BRegfree(rxp)


# bregonig.dll
def LoadBregonig(unicode_func = False):
    """Load bregonig.dll.
    
    argument:
      unicode_func -- True:  Use Unicode functions.
                      False: Use ANSI functions.
    """
    LoadDLL(cdll.bregonig, unicode_func)

# BREGEXP.DLL
def LoadBregexp():
    """Load BREGEXP.DLL."""
    LoadDLL(cdll.bregexp)


def LoadDLL(regexpdll, unicode_func = False):
    """Load specified regular expression DLL.
    
    arguments:
      regexpdll -- Instance of ctypes.CDLL to load.
      unicode_func -- True:  Use Unicode functions.
                      False: Use ANSI functions.
    """
    
    if unicode_func:
        c_tchar_p = c_wchar_p
    else:
        c_tchar_p = c_char_p
    
    global _BRegexpVersion
    if unicode_func:
        _BRegexpVersion = regexpdll.BRegexpVersionW
    else:
        _BRegexpVersion = regexpdll.BRegexpVersion
    _BRegexpVersion.restype = c_tchar_p
    
    global _BMatch
    if unicode_func:
        _BMatch = regexpdll.BMatchW
    else:
        _BMatch = regexpdll.BMatch
    _BMatch.argtypes = [c_tchar_p, c_void_p, c_void_p,
            POINTER(POINTER(BREGEXP)), c_tchar_p]
    
    global _BSubst
    if unicode_func:
        _BSubst = regexpdll.BSubstW
    else:
        _BSubst = regexpdll.BSubst
    _BSubst.argtypes = [c_tchar_p, c_void_p, c_void_p,
            POINTER(POINTER(BREGEXP)), c_tchar_p]
    
    global _BMatchEx
    try:
        if unicode_func:
            _BMatchEx = regexpdll.BMatchExW
        else:
            _BMatchEx = regexpdll.BMatchEx
        _BMatchEx.argtypes = [c_tchar_p, c_void_p, c_void_p, c_void_p,
                POINTER(POINTER(BREGEXP)), c_tchar_p]
    except AttributeError:
        pass
    
    global _BSubstEx
    try:
        if unicode_func:
            _BSubstEx = regexpdll.BSubstExW
        else:
            _BSubstEx = regexpdll.BSubstEx
        _BSubstEx.argtypes = [c_tchar_p, c_void_p, c_void_p, c_void_p,
                POINTER(POINTER(BREGEXP)), c_tchar_p]
    except AttributeError:
        pass
    
    global _BTrans
    if unicode_func:
        _BTrans = regexpdll.BTransW
    else:
        _BTrans = regexpdll.BTrans
    _BTrans.argtypes = [c_tchar_p, c_void_p, c_void_p,
            POINTER(POINTER(BREGEXP)), c_tchar_p]
    
    global _BSplit
    if unicode_func:
        _BSplit = regexpdll.BSplitW
    else:
        _BSplit = regexpdll.BSplit
    _BSplit.argtypes = [c_tchar_p, c_void_p, c_void_p, c_int,
            POINTER(POINTER(BREGEXP)), c_tchar_p]
    
    global _BRegfree
    if unicode_func:
        _BRegfree = regexpdll.BRegfreeW
    else:
        _BRegfree = regexpdll.BRegfree
    _BRegfree.argtypes = [POINTER(BREGEXP)]


# -*- coding: utf-8 -*-

"""Pooling BREGEXP objects.

This is a sample of pooling BREGEXP objects.
"""

from ctypes import *
from bregonig import *

__all__ = ["BregPool"]

class BregPool:
    def __init__(self, nmax):
        self._nmax = nmax
        self._rxpool = (POINTER(BREGEXP) * nmax)()

    def __del__(self):
        self.Free()

    def Free(self):
        if self._nmax == 0:
            return
        for r in self._rxpool:
            if (r):
                BRegfree(r)
        del self._rxpool
        self._nmax = 0

    def Get(self, regstr):
        for i in xrange(self._nmax):
            r = self._rxpool[i]
            if (not r):
                break
            if (not r.contents.parap):
                break
            if (regstr == string_at(r.contents.parap,
                    r.contents.paraendp - r.contents.parap)):
                return r
        
        if (self._rxpool[i]):
            return self._rxpool[i];
        
        msg = create_string_buffer(80)
        
        dummystr = " "
        p = cast(dummystr, c_void_p)
        BMatch(regstr, p, p.value + 1, byref(self._rxpool[i]), msg)
        
        return self._rxpool[i]


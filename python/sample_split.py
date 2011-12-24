#!/usr/bin/env python
# -*- coding: cp932 -*-

from __future__ import print_function
from ctypes import *
from bregonig import *
from BregPool import *

LoadBregonig()

print(BRegexpVersion())
print()

bpool = BregPool(8)
msg = create_string_buffer(BREGEXP_MAX_ERROR_MESSAGE_LEN)

t1 = " Yokohama 045-222-1111  Osaka 06-5555-6666  Tokyo 03-1111-9999 "
t1p = cast(t1, c_void_p)
pattern1 = r"/ *\d{2,3}-\d{3,4}-\d{4} */"
rxp = bpool.Get(pattern1)
splitcnt =  BSplit(pattern1, t1p, t1p.value + len(t1), 0, byref(rxp), msg)
if splitcnt > 0:
    i = 0
    for j in xrange(splitcnt):
        length = rxp.contents.splitp[i+1] - rxp.contents.splitp[i]
        print("len=%d [%d]=%s" % (length, j, string_at(rxp.contents.splitp[i], length)))
        i += 2


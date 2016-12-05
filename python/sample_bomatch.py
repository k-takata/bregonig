#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function
from ctypes import *
from bregonig import *

LoadBregonig()

print(BRegexpVersion())
print()

rxp = POINTER(BREGEXP)()
msg = create_string_buffer(BREGEXP_MAX_ERROR_MESSAGE_LEN)

t1 = " Yokohama 045-222-1111  Osaka 06-5555-6666  Tokyo 03-1111-9999 "
t1p = cast(t1, c_void_p)
pattern1 = r"(03|045)-(\d{3,4})-(\d{4})"
pos = 0
while BoMatch(pattern1, None,
        t1p.value, t1p.value + pos, t1p.value + len(t1),
        False, byref(rxp), msg) > 0:
    print("pos: %d, '%s'" % (pos, string_at(t1p.value + pos)))
    print("nparens: %d" % rxp.contents.nparens)
    for i in xrange(rxp.contents.nparens + 1):
        print("%d = %s" % (i, string_at(rxp.contents.startp[i],
                rxp.contents.endp[i] - rxp.contents.startp[i])))
    pos = rxp.contents.endp[0] - t1p.value

if (rxp):
    BRegfree(rxp)


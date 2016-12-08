#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function, unicode_literals
from ctypes import *
from bregonig import *

LoadBregonig()

print(BRegexpVersion())
print()

rxp = POINTER(BREGEXP)()
msg = create_string_buffer(BREGEXP_MAX_ERROR_MESSAGE_LEN)

t1 = " Yokohama 045-222-1111  Osaka 06-5555-6666  Tokyo 03-1111-9999 ".encode('ASCII')
t1p = cast(t1, c_void_p)
pattern1 = r"tr/A-Z0-9/a-zx/g".encode('ASCII')
ctr = BTrans(pattern1, t1p, t1p.value + len(t1), byref(rxp), msg)
if ctr > 0:
    print("after(%d)=%s" % (ctr, string_at(rxp.contents.outp).decode('ASCII')))
    print("length=%d" % (rxp.contents.outendp - rxp.contents.outp))

if (rxp):
    BRegfree(rxp)


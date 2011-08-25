#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function
from ctypes import *
from bregonig import *


def subst_callback(type, value, index):
    print("callback: %d, %d, %d" % (type, value, index))
    return True



LoadBregonig()

print(BRegexpVersion())
print()

rxp = POINTER(BREGEXP)()
msg = create_string_buffer(BREGEXP_MAX_ERROR_MESSAGE_LEN)

t1 = " 横浜 045-222-1111  大阪 06-5555-6666  東京 03-1111-9999 "
t1p = cast(t1, c_void_p)
ctr = BoSubst(r"(\d\d)-\d{4}-\d{4}", r"$1-xxxx-xxxx", r"g8",
        t1p, t1p, t1p.value + len(t1),
        None, byref(rxp), msg)
if ctr > 0:
    print("after(%d)=%s" % (ctr, string_at(rxp.contents.outp)))
    print("length=%d" % (rxp.contents.outendp - rxp.contents.outp))



ctr = BoSubst(None, r"$1-ｘｘｘｘ-\x{FF59}\x{FF59}\x{FF59}\x{FF59}", None,
        t1p, t1p, t1p.value + len(t1),
        BCallBack(subst_callback), byref(rxp), msg)
if ctr > 0:
    print("after(%d)=%s" % (ctr, string_at(rxp.contents.outp)))
    print("length=%d" % (rxp.contents.outendp - rxp.contents.outp))


if (rxp):
    BRegfree(rxp)


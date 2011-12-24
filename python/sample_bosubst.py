#!/usr/bin/env python
# -*- coding: cp932 -*-

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

t1 = " Yokohama 045-222-1111  Osaka 06-5555-6666  Tokyo 03-1111-9999 "
t1p = cast(t1, c_void_p)
ctr = BoSubst(r"(\d\d)-\d{4}-\d{4}", r"$1-xxxx-xxxx", r"g",
        t1p, t1p, t1p.value + len(t1),
        None, byref(rxp), msg)
if ctr > 0:
    print("after(%d)=%s" % (ctr, string_at(rxp.contents.outp)))
    print("length=%d" % (rxp.contents.outendp - rxp.contents.outp))


# use same patternp and same substp -> reused
ctr = BoSubst(r"(\d\d)-\d{4}-\d{4}", r"$1-xxxx-xxxx", r"g",
        t1p, t1p, t1p.value + len(t1),
        None, byref(rxp), msg)
if ctr > 0:
    print("after(%d)=%s" % (ctr, string_at(rxp.contents.outp)))
    print("length=%d" % (rxp.contents.outendp - rxp.contents.outp))


# reuse patternp, use new substp
# use callback
ctr = BoSubst(None, r"$1-yyyy-zzzz", None,
        t1p, t1p, t1p.value + len(t1),
        BCallBack(subst_callback), byref(rxp), msg)
if ctr > 0:
    print("after(%d)=%s" % (ctr, string_at(rxp.contents.outp)))
    print("length=%d" % (rxp.contents.outendp - rxp.contents.outp))


# reuse patternp and substp
# use callback
ctr = BoSubst(None, None, None,
        t1p, t1p, t1p.value + len(t1),
        BCallBack(subst_callback), byref(rxp), msg)
if ctr > 0:
    print("after(%d)=%s" % (ctr, string_at(rxp.contents.outp)))
    print("length=%d" % (rxp.contents.outendp - rxp.contents.outp))


# use new patternp and same substp -> not reused
# use callback
ctr = BoSubst(r"(\d{3})-\d{3}-\d{4}", r"$1-yyyy-zzzz", r"g",
        t1p, t1p, t1p.value + len(t1),
        BCallBack(subst_callback), byref(rxp), msg)
if ctr > 0:
    print("after(%d)=%s" % (ctr, string_at(rxp.contents.outp)))
    print("length=%d" % (rxp.contents.outendp - rxp.contents.outp))

if (rxp):
    BRegfree(rxp)


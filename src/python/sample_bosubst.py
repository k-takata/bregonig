#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function, unicode_literals
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

t1 = " Yokohama 045-222-1111  Osaka 06-5555-6666  Tokyo 03-1111-9999 ".encode('ASCII')
t1p = cast(t1, c_void_p)
ctr = BoSubst(r"(\d\d)-\d{4}-\d{4}".encode('ASCII'), r"$1-xxxx-xxxx".encode('ASCII'), r"g".encode('ASCII'),
        t1p, t1p, t1p.value + len(t1),
        None, byref(rxp), msg)
if ctr > 0:
    if rxp.contents.outp:
        print("after(%d)=%s" % (ctr, string_at(rxp.contents.outp).decode('ASCII')))
        print("length=%d" % (rxp.contents.outendp - rxp.contents.outp))
    else:
        # Result is an empty string.
        print("after(%d)" % ctr)
        print("length=0")


# use same patternp and same substp -> reused
ctr = BoSubst(r"(\d\d)-\d{4}-\d{4}".encode('ASCII'), r"$1-xxxx-xxxx".encode('ASCII'), r"g".encode('ASCII'),
        t1p, t1p, t1p.value + len(t1),
        None, byref(rxp), msg)
if ctr > 0:
    if rxp.contents.outp:
        print("after(%d)=%s" % (ctr, string_at(rxp.contents.outp).decode('ASCII')))
        print("length=%d" % (rxp.contents.outendp - rxp.contents.outp))
    else:
        # Result is an empty string.
        print("after(%d)" % ctr)
        print("length=0")


# reuse patternp, use new substp
# use callback
ctr = BoSubst(None, r"$1-yyyy-zzzz".encode('ASCII'), None,
        t1p, t1p, t1p.value + len(t1),
        BCallBack(subst_callback), byref(rxp), msg)
if ctr > 0:
    if rxp.contents.outp:
        print("after(%d)=%s" % (ctr, string_at(rxp.contents.outp).decode('ASCII')))
        print("length=%d" % (rxp.contents.outendp - rxp.contents.outp))
    else:
        # Result is an empty string.
        print("after(%d)" % ctr)
        print("length=0")


# reuse patternp and substp
# use callback
ctr = BoSubst(None, None, None,
        t1p, t1p, t1p.value + len(t1),
        BCallBack(subst_callback), byref(rxp), msg)
if ctr > 0:
    if rxp.contents.outp:
        print("after(%d)=%s" % (ctr, string_at(rxp.contents.outp).decode('ASCII')))
        print("length=%d" % (rxp.contents.outendp - rxp.contents.outp))
    else:
        # Result is an empty string.
        print("after(%d)" % ctr)
        print("length=0")


# use new patternp and same substp -> not reused
# use callback
ctr = BoSubst(r"(\d{3})-\d{3}-\d{4}".encode('ASCII'), r"$1-yyyy-zzzz".encode('ASCII'), r"g".encode('ASCII'),
        t1p, t1p, t1p.value + len(t1),
        BCallBack(subst_callback), byref(rxp), msg)
if ctr > 0:
    if rxp.contents.outp:
        print("after(%d)=%s" % (ctr, string_at(rxp.contents.outp).decode('ASCII')))
        print("length=%d" % (rxp.contents.outendp - rxp.contents.outp))
    else:
        # Result is an empty string.
        print("after(%d)" % ctr)
        print("length=0")

if (rxp):
    BRegfree(rxp)


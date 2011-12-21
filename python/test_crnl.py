#!/usr/bin/env python
# -*- coding: cp932 -*-

from __future__ import print_function
from ctypes import *
from bregonig import *
import sys
import codecs
import locale

nerror = 0
nsucc = 0
nfail = 0

encoding = "CP932"

class strptr:
    """a helper class to get a pointer to a string"""
    def __init__(self, s):
        if not isinstance(s, bytes):
            raise TypeError
        self._str = s
        self._ptr = cast(self._str, c_void_p)

    def getptr(self, offset=0):
        if offset == -1:    # -1 means the end of the string
            offset = len(self._str)
        elif offset > len(self._str):
            raise IndexError
        return self._ptr.value + offset

def cc_to_cb(s, enc, cc):
    """convert char count to byte count
    
    arguments:
      s -- unicode string
      enc -- encoding name
      cc -- char count
    """
    if cc > len(s):
        raise IndexError
    return len(s[0:cc].encode(enc))

def xx(pattern, target, start_offset, s_from, s_to, not_match):
    global nerror
    global nsucc
    global nfail
    
    rxp = POINTER(BREGEXP)()
    msg = create_tchar_buffer(BREGEXP_MAX_ERROR_MESSAGE_LEN)
    
    pattern2 = pattern
    if not isinstance(pattern, bytes):
        pattern2 = pattern.encode(encoding)
    if encoding == "UTF-16LE":
        pattern2 = pattern2.decode(encoding)
    
    target2 = target
    if not isinstance(target, bytes):
        s_from = cc_to_cb(target, encoding, s_from)
        s_to = cc_to_cb(target, encoding, s_to)
        target2 = target.encode(encoding)
    tp = strptr(target2)
    
    if encoding == "UTF-8":
        r = BoMatch(pattern2, "R8", tp.getptr(), tp.getptr(start_offset), tp.getptr(-1),
                False, byref(rxp), msg)
    else:
        r = BoMatch(pattern2, "Rk", tp.getptr(), tp.getptr(start_offset), tp.getptr(-1),
                False, byref(rxp), msg)
    
    if r < 0:
        nerror += 1
        print("ERROR: %s (/%s/ '%s')" % (msg.value, pattern, target),
                file=sys.stderr)
        return
    
    if r == 0:
        if not_match:
            nsucc += 1
            print("OK(N): /%s/ '%s'" % (pattern, target))
        else:
            nfail += 1
            print("FAIL: /%s/ '%s'" % (pattern, target))
    else:
        if not_match:
            nfail += 1
            print("FAIL(N): /%s/ '%s'" % (pattern, target))
        else:
            start = rxp.contents.startp[0] - tp.getptr()
            end = rxp.contents.endp[0] - tp.getptr()
            if (start == s_from) and (end == s_to):
                nsucc += 1
                print("OK: /%s/ '%s'" % (pattern, target))
            else:
                nfail += 1
                print("FAIL: /%s/ '%s' %d-%d : %d-%d\n" % (pattern, target,
                        s_from, s_to, start, end))
    
    if (rxp):
        BRegfree(rxp)

def x(pattern, target, start_offset, s_from, s_to):
    xx(pattern, target, start_offset, s_from, s_to, False)

def n(pattern, target, start_offset):
    xx(pattern, target, start_offset, 0, 0, True)



def main():
    global encoding
    
    unicode_func = False
    
    # set encoding of the test target
    if len(sys.argv) > 1:
        if sys.argv[1] in ("CP932", "SJIS"):
            unicode_func = False
        elif sys.argv[1] == "UTF-8":
            unicode_func = False
        elif sys.argv[1] == "UTF-16LE":
            unicode_func = True
        else:
            print("test target encoding error")
            sys.exit()
        encoding = sys.argv[1]
    
    # set encoding of stdout/stderr
    if len(sys.argv) > 2:
        outenc = sys.argv[2]
    else:
        outenc = locale.getpreferredencoding()
    sys.stdout = codecs.getwriter(outenc)(sys.stdout)
    sys.stderr = codecs.getwriter(outenc)(sys.stderr)
    
    
    LoadBregonig(unicode_func)
    #LoadBregexp()
    
    print(BRegexpVersion())
    print()
    
    x(u"",      u"\r\n",            0,  0,  0);
    n(u".",     u"\r\n",            0);
    n(u"..",    u"\r\n",            0);
    x(u"^",     u"\r\n",            0,  0,  0);
    x(u"(?m)\\n^",  u"\r\nf",       0,  1,  2);
    x(u"(?m)\\n^a", u"\r\na",       0,  1,  3);
    x(u"$",     u"\r\n",            0,  0,  0);
    x(u"T$",    u"T\r\n",           0,  0,  1);
    x(u"T$",    u"T\raT\r\n",       0,  3,  4);
    x(u"\\z",   u"\r\n",            0,  2,  2);
    n(u"a\\z",  u"a\r\n",           0);
    x(u"\\Z",   u"\r\n",            0,  0,  0);
    x(u"\\Z",   u"\r\na",           0,  3,  3);
    x(u"\\Z",   u"\r\n\r\n\n",      0,  4,  4);
    x(u"\\Z",   u"\r\n\r\nX",       0,  5,  5);
    x(u"a\\Z",  u"a\r\n",           0,  0,  1);
    x(u"aaaaaaaaaaaaaaa\\Z",    u"aaaaaaaaaaaaaaa\r\n", 0,  0,  15);
    x(u"(?m)a|$",   u"b\r\n",       0,  1,  1);
    x(u"(?m)$|b",   u"\rb",         0,  1,  2);
    x(u"(?m)a$|ab$",u"\r\nab\r\n",  0,  2,  4);
    x(u"a|\\Z",     u"b\r\n",       0,  1,  1);
    x(u"\\Z|b",     u"\rb",         0,  1,  2);
    x(u"a\\Z|ab\\Z",u"\r\nab\r\n",  0,  2,  4);
    x(u"(?=a$).",   u"a\r\n",       0,  0,  1);
    n(u"(?=a$).",   u"a\r",         0);
    x(u"(?!a$)..",  u"a\r",         0,  0,  2);
    x(u"(?m)(?<=a$)\\r\\n", u"a\r\n",   0,  1,  3);
    n(u"(?m)(?<!a$)\\r\\n", u"a\r\n",   0);
    x(u"(?=a\\Z).", u"a\r\n",       0,  0,  1);
    n(u"(?=a\\Z).", u"a\r",         0);
    x(u"(?!a\\Z)..",u"a\r",         0,  0,  2);
    x(u"(?m).*$",   u"aa\r\n",      0,  0,  2);
    x(u"(?m).*$",   u"aa\r",        0,  0,  3);
    x(u"\\R{3}",    u"\r\r\n\n",    0,  0,  4);
    x(u"(?m)$",     u"\n",          0,  0,  0);
    x(u"(?m)T$",    u"T\n",         0,  0,  1);
    x(u"(?s).",     u"\r\n",        0,  0,  1);
    x(u"(?s)..",    u"\r\n",        0,  0,  2);
    x(u"(?m)^",     u"\n.",         1,  1,  1);
    x(u"(?m)^",     u"\r\n.",       1,  2,  2);
    x(u"(?m)^",     u"\r\n.",       2,  2,  2);
    x(u"(?m)$",     u"\n\n",        1,  1,  1);
    x(u"(?m)$",     u"\r\n\n",      1,  2,  2);
    x(u"(?m)$",     u"\r\n\n",      2,  2,  2);
    n(u"(?m)^$",    u"\n\r",        1);
    x(u"(?m)^$",    u"\n\r\n",      1,  1,  1);
    x(u"(?m)^$",    u"\r\n\n",      1,  2,  2);
    x(u"\\Z",       u"\r\n\n",      1,  2,  2);
    n(u".(?=\\Z)",  u"\r\n",        1);
    x(u"(?=\\Z)",   u"\r\n",        1,  2,  2);
    x(u"(?m)(?<=^).",   u"\r\n.",   0,  2,  3);
    x(u"(?m)(?<=^).",   u"\r\n.",   1,  2,  3);
    x(u"(?m)(?<=^).",   u"\r\n.",   2,  2,  3);
    x(u"(?m)^a",    u"\r\na",       0,  2,  3);
    x(u"(?m)^a",    u"\r\na",       1,  2,  3);
    x(u"(?ms)$.{1,2}a", u"\r\na",   0,  0,  3);
    n(u"(?ms)$.{1,2}a", u"\r\na",   1);
    x(u".*b",       u"\r\naaab\r\n",1,  2,  6);
    
    
    print("\nRESULT   SUCC: %d,  FAIL: %d,  ERROR: %d\n" % (
           nsucc, nfail, nerror))


if __name__ == '__main__':
    main()


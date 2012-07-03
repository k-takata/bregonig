#!/usr/bin/env python
# -*- coding: cp932 -*-

from __future__ import print_function, unicode_literals
from ctypes import *
from bregonig import *
import sys
import io
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
        try:
            self._ptr = cast(self._str, c_void_p)   # CPython 2.x/3.x
        except TypeError:
            self._ptr = c_void_p(self._str)         # PyPy 1.x

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
    s = s.encode('UTF-32LE')
    clen = cc * 4
    if clen > len(s):
        raise IndexError
    return len(s[:clen].decode('UTF-32LE').encode(enc))

def xx(pattern, target, start_offset, s_from, s_to, not_match):
    global nerror
    global nsucc
    global nfail
    
    rxp = POINTER(BREGEXP)()
    msg = create_tchar_buffer(BREGEXP_MAX_ERROR_MESSAGE_LEN)
    
    pattern2 = pattern
    if not isinstance(pattern, bytes):
        pattern2 = pattern.encode(encoding)
    
    target2 = target
    if not isinstance(target, bytes):
        s_from = cc_to_cb(target, encoding, s_from)
        s_to = cc_to_cb(target, encoding, s_to)
        target2 = target.encode(encoding)
    tp = strptr(target2)
    
    if encoding == "UTF-8":
        option = "R8"
    else:
        option = "Rk"
    option = option.encode(encoding)
    
    if encoding == "UTF-16LE":
        pattern2 = pattern2.decode(encoding)
        option = option.decode(encoding)
    
    r = BoMatch(pattern2, option, tp.getptr(), tp.getptr(start_offset), tp.getptr(-1),
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
        encs = {"CP932": False,
                "SJIS": False,
                "UTF-8": False,
                "UTF-16LE": True}
        try:
            unicode_func = encs[sys.argv[1]]
        except KeyError:
            print("test target encoding error")
            print("Usage: python test_crnl.py [test target encoding] [output encoding]")
            sys.exit()
        encoding = sys.argv[1]
    
    # set encoding of stdout/stderr
    if len(sys.argv) > 2:
        outenc = sys.argv[2]
    else:
        outenc = locale.getpreferredencoding()
    
    class TextWriter:
        def __init__(self, fileno, **kwargs):
            kw = dict(kwargs)
            kw.setdefault('errors', 'backslashreplace')
            kw.setdefault('closefd', False)
            self._writer = io.open(fileno, mode='w', **kw)
            
            # work around for Python 2.x
            _write = self._writer.write    # save the original write() function
            enc = locale.getpreferredencoding()
            self._writer.write = lambda s: _write(s.decode(enc)) \
                    if isinstance(s, bytes) else _write(s)  # convert to unistr
        
        def getwriter(self):
            return self._writer
    
    sys.stdout = TextWriter(sys.stdout.fileno(), encoding=outenc).getwriter()
    sys.stderr = TextWriter(sys.stderr.fileno(), encoding=outenc).getwriter()
    
    
    LoadBregonig(unicode_func)
    #LoadBregexp()
    
    print(BRegexpVersion())
    print()
    
    x("",       "\r\n",             0,  0,  0);
    n(".",      "\r\n",             0);
    n("..",     "\r\n",             0);
    x("^",      "\r\n",             0,  0,  0);
    x("(?m)\\n^",   "\r\nf",        0,  1,  2);
    x("(?m)\\n^a",  "\r\na",        0,  1,  3);
    x("$",      "\r\n",             0,  0,  0);
    x("T$",     "T\r\n",            0,  0,  1);
    x("T$",     "T\raT\r\n",        0,  3,  4);
    x("\\z",    "\r\n",             0,  2,  2);
    n("a\\z",   "a\r\n",            0);
    x("\\Z",    "\r\n",             0,  0,  0);
    x("\\Z",    "\r\na",            0,  3,  3);
    x("\\Z",    "\r\n\r\n\n",       0,  4,  4);
    x("\\Z",    "\r\n\r\nX",        0,  5,  5);
    x("a\\Z",   "a\r\n",            0,  0,  1);
    x("aaaaaaaaaaaaaaa\\Z", "aaaaaaaaaaaaaaa\r\n",  0,  0,  15);
    x("(?m)a|$",    "b\r\n",        0,  1,  1);
    x("(?m)$|b",    "\rb",          0,  1,  2);
    x("(?m)a$|ab$", "\r\nab\r\n",   0,  2,  4);
    x("a|\\Z",      "b\r\n",        0,  1,  1);
    x("\\Z|b",      "\rb",          0,  1,  2);
    x("a\\Z|ab\\Z", "\r\nab\r\n",   0,  2,  4);
    x("(?=a$).",    "a\r\n",        0,  0,  1);
    n("(?=a$).",    "a\r",          0);
    x("(?!a$)..",   "a\r",          0,  0,  2);
    x("(?m)(?<=a$)\\r\\n",  "a\r\n",    0,  1,  3);
    n("(?m)(?<!a$)\\r\\n",  "a\r\n",    0);
    x("(?=a\\Z).",  "a\r\n",        0,  0,  1);
    n("(?=a\\Z).",  "a\r",          0);
    x("(?!a\\Z)..", "a\r",          0,  0,  2);
    x("(?m).*$",    "aa\r\n",       0,  0,  2);
    x("(?m).*$",    "aa\r",         0,  0,  3);
    x("\\R{3}",     "\r\r\n\n",     0,  0,  4);
    x("(?m)$",      "\n",           0,  0,  0);
    x("(?m)T$",     "T\n",          0,  0,  1);
    x("(?s).",      "\r\n",         0,  0,  1);
    x("(?s)..",     "\r\n",         0,  0,  2);
    x("(?m)^",      "\n.",          1,  1,  1);
    x("(?m)^",      "\r\n.",        1,  2,  2);
    x("(?m)^",      "\r\n.",        2,  2,  2);
    x("(?m)$",      "\n\n",         1,  1,  1);
    x("(?m)$",      "\r\n\n",       1,  2,  2);
    x("(?m)$",      "\r\n\n",       2,  2,  2);
    n("(?m)^$",     "\n\r",         1);
    x("(?m)^$",     "\n\r\n",       1,  1,  1);
    x("(?m)^$",     "\r\n\n",       1,  2,  2);
    x("\\Z",        "\r\n\n",       1,  2,  2);
    n(".(?=\\Z)",   "\r\n",         1);
    x("(?=\\Z)",    "\r\n",         1,  2,  2);
    x("(?m)(?<=^).",    "\r\n.",    0,  2,  3);
    x("(?m)(?<=^).",    "\r\n.",    1,  2,  3);
    x("(?m)(?<=^).",    "\r\n.",    2,  2,  3);
    x("(?m)^a",     "\r\na",        0,  2,  3);
    x("(?m)^a",     "\r\na",        1,  2,  3);
    x("(?ms)$.{1,2}a",  "\r\na",    0,  0,  3);
    n("(?ms)$.{1,2}a",  "\r\na",    1);
    x(".*b",        "\r\naaab\r\n", 1,  2,  6);
    
    
    print("\nRESULT   SUCC: %d,  FAIL: %d,  ERROR: %d\n" % (
           nsucc, nfail, nerror))


if __name__ == '__main__':
    main()


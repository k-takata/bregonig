#!/usr/bin/env python
# -*- coding: cp932 -*-

from __future__ import print_function, unicode_literals
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

def print_result(result, pattern, file=None):
    if not file:
        file = sys.stdout
    print(result + ": ", end='', file=file)
    try:
        print(pattern, file=file)
    except UnicodeEncodeError as e:
        print('(' + str(e) + ')')

def xx(pattern, replacement, target, s_result, not_match=False):
    global nerror
    global nsucc
    global nfail
    
    rxp = POINTER(BREGEXP)()
    msg = create_tchar_buffer(BREGEXP_MAX_ERROR_MESSAGE_LEN)
    
    pattern2 = pattern
    if not isinstance(pattern, bytes):
        pattern2 = pattern.encode(encoding)
    
    replacement2 = replacement
    if not isinstance(replacement, bytes):
        replacement2 = replacement2.encode(encoding)
    
    target2 = target
    if not isinstance(target, bytes):
        #s_from = cc_to_cb(target, encoding, s_from)
        #s_to = cc_to_cb(target, encoding, s_to)
        target2 = target.encode(encoding)
    tp = strptr(target2)
    
    if encoding == "UTF-8":
        option = "g8"
    else:
        option = "gk"
    option = option.encode(encoding)
    
    if encoding == "UTF-16LE":
        pattern2 = pattern2.decode(encoding)
        replacement2 = replacement2.decode(encoding)
        option = option.decode(encoding)
    
    if isinstance(s_result, bytes):
        s_result = s_result.decode(encoding)
    
    
    r = BoSubst(pattern2, replacement2, option, tp.getptr(), tp.getptr(), tp.getptr(-1),
            None, byref(rxp), msg)
    
    if r < 0:
        nerror += 1
        print_result("ERROR",
                "%s (s/%s/%s/g '%s')" % (msg.value, pattern, replacement, target),
                file=sys.stderr)
        return
    
    if r == 0:
        if not_match:
            nsucc += 1
            print_result("OK(N)", "s/%s/%s/g '%s'" % (pattern, replacement, target))
        else:
            nfail += 1
            print_result("FAIL", "s/%s/%s/g '%s'" % (pattern, replacement, target))
    else:
        if not_match:
            nfail += 1
            print_result("FAIL(N)",
                    "s/%s/%s/g '%s' => '%s'" % (pattern, replacement, target, out_result))
        else:
            out_result = tstring_at(rxp.contents.outp)
            if isinstance(out_result, bytes):
                out_result = out_result.decode(encoding)
            if out_result == s_result:
                nsucc += 1
                print_result("OK",
                        "s/%s/%s/g '%s' => '%s'" % (pattern, replacement, target, out_result))
            else:
                nfail += 1
                print_result("FAIL",
                        "s/%s/%s/g '%s' => '%s'" % (pattern, replacement, target, out_result))
    
    if (rxp):
        BRegfree(rxp)

#def x2(pattern, target, s_from, s_to):
#    xx(pattern, target, s_from, s_to, 0, False)

#def x3(pattern, target, s_from, s_to, mem):
#    xx(pattern, target, s_from, s_to, mem, False)

#def n(pattern, target):
#    xx(pattern, target, 0, 0, 0, True)


def is_unicode_encoding(enc):
    return enc in ("UTF-16LE", "UTF-8")

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
            print("Usage: python test_subst.py [test target encoding] [output encoding]")
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
    
    print(BRegexpVersion())
    print()
    
    
    # fixed string
    xx("abc", "def", "abc", "def")
    xx("abc", "def", "abcabcabc", "defdefdef")
    xx("あいう", "えお", "あいうあいう", "えおえお")
    
    # variable
    xx("abc", "$&_$&_$&", "abc", "abc_abc_abc")
    xx("(.)(.)(.)(.)(.)(.)(.)(.)(.)(.)", "${10}$9\\8${7}$6\\5${4}$3\\2${1}", "abcdefghij", "jihgfedcba")
    xx("(a+)(b+)(c+).*", "$+", "aabbcc", "cc")
    xx("(a+)(b+)(c+)?.*", "$+", "aabbdd", "bb")
    xx("(a+)(b+)?(c+)?.*", "$+", "aaddee", "aa")
    
    # named group
    xx("(?<a>.*)_(?<b>.*)", "\\k<b>_\\k<a>", "abc_def", "def_abc")
    xx("(?'a'.*)_(?'b'.*)", "\\k'b'_\\k'a'", "abc_def", "def_abc")
    xx("(?<a>.*)_(?<b>.*)", "$+{b}_$+{a}", "abc_def", "def_abc")
    xx("(?<a>.*)_(?<a>.*)_(?<a>.*)", "$+{a}", "abc_def_ghi", "abc")
    xx("(?<a>.*)_(?<a>.*)_(?<a>.*)", "$-{a}[2]_$-{a}[1]_$-{a}[0]", "abc_def_ghi", "ghi_def_abc")
    xx("(?<a>.*)_(?<a>.*)_(?<a>.*)", "$-{a}[-1]_$-{a}[-2]_$-{a}[-3]", "abc_def_ghi", "ghi_def_abc")
    
    # \l, \u, \L, \U and \E
    xx("", "\\LABCDEFG\\EHIJKLMN", "", "abcdefgHIJKLMN")
    xx("", "\\Uabcdefg\\Ehijklmn", "", "ABCDEFGhijklmn")
    xx("(.*)", "\\L$1", "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "abcdefghijklmnopqrstuvwxyz")
    xx("(.*)", "\\U$1", "abcdefghijklmnopqrstuvwxyz", "ABCDEFGHIJKLMNOPQRSTUVWXYZ")
    xx("(.*)_(.*)", "\\L\\u$1\\u\\L$2", "abc_def", "AbcDef")
    xx("(.*)_(.*)", "\\L\\u$1\\u$2",    "abc_def", "AbcDef")
    xx("(.*)_(.*)", "\\l\\U$1\\U\\l$2", "abc_def", "aBCdEF")
    xx("(.*)_(.*)", "\\l\\U$1\\l$2",    "abc_def", "aBCdEF")
    xx("([a-z]+)_?", "\\L\\u$1", "abc_def_ghi_jkl", "AbcDefGhiJkl")     # snake_case => CamelCase
    xx("([A-Z]?[a-z]+)([A-Z])", "\\L$1_$2", "AbcDefGhiJkl", "abc_def_ghi_jkl")  # CamelCase => snake_case
    xx("(a*)(b*)(c*)", "\\u$1$2$3", "aabbcc", "Aabbcc")
    xx("(a*)(b*)(c*)", "\\u$1$2$3", "abbcc",  "Abbcc")
    xx("(a*)(b*)(c*)", "\\u$1$2$3", "bbcc",   "Bbcc")
    xx("(a*)(b*)(c*)", "\\u$1$2$3", "bcc",    "Bcc")
    xx("(a*)(b*)(c*)", "\\u$1$2$3", "cc",     "Cc")
    xx("(A*)(B*)(C*)", "\\l$1$2$3", "AABBCC", "aABBCC")
    xx("(A*)(B*)(C*)", "\\l$1$2$3", "ABBCC",  "aBBCC")
    xx("(A*)(B*)(C*)", "\\l$1$2$3", "BBCC",   "bBCC")
    xx("(A*)(B*)(C*)", "\\l$1$2$3", "BCC",    "bCC")
    xx("(A*)(B*)(C*)", "\\l$1$2$3", "CC",     "cC")
    xx("Abc", "\\U$&\\L$&", "Abc", "ABCabc")
    
    # nasted variable (bregonig.dll doesn't support this.)
#    xx("^([23]),(.*),(.*)$", "${$1}", "2,1234,abcd", "1234")
#    xx("^([23]),(.*),(.*)$", "${$1}", "3,1234,abcd", "abcd")
#    xx("^(?<select>[ab]),(?<a>.*?),(?<b>.*?)$", "$+{$+{select}}", "a,1234,abcd", "1234")
#    xx("^(?<select>[ab]),(?<a>.*?),(?<b>.*?)$", "$+{$+{select}}", "b,1234,abcd", "abcd")
    
    print("\nRESULT   SUCC: %d,  FAIL: %d,  ERROR: %d\n" % (
           nsucc, nfail, nerror))


if __name__ == '__main__':
    main()


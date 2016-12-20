#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function, unicode_literals
import ctypes
from bregonig import *
from test_common import *
import sys

def xx(pattern, replacement, target, s_result, not_match=False):
    rxp = ctypes.POINTER(BREGEXP)()
    msg = create_tchar_buffer(BREGEXP_MAX_ERROR_MESSAGE_LEN)
    
    encoding = get_encoding()
    
    pattern2 = pattern
    if not isinstance(pattern, bytes):
        pattern2 = pattern.encode(encoding)
    
    replacement2 = replacement
    if not isinstance(replacement, bytes):
        replacement2 = replacement2.encode(encoding)
    
    target2 = target
    if not isinstance(target, bytes):
        target2 = target.encode(encoding)
    tp = strptr(target2)
    
    if encoding == "UTF-8":
        option = "g8"
    else:
        option = "gk"
    option = option.encode(encoding)
    
    if encoding == "UTF-16LE":
        pattern2 = ctypes.c_wchar_p(pattern2.decode(encoding))
        replacement2 = ctypes.c_wchar_p(replacement2.decode(encoding))
        option = ctypes.c_wchar_p(option.decode(encoding))
    
    if isinstance(s_result, bytes):
        s_result = s_result.decode(encoding)
    
    
    r = BoSubst(pattern2, replacement2, option, tp.getptr(), tp.getptr(), tp.getptr(-1),
            None, ctypes.byref(rxp), msg)
    
    if r < 0:
        inc_nerror()
        print_result("ERROR",
                "%s (s/%s/%s/g '%s')" % (msg.value, pattern, replacement, target),
                file=sys.stderr)
        return
    
    if r == 0:
        # Not matched
        if not_match:
            inc_nsucc()
            print_result("OK(N)", "s/%s/%s/g '%s'" % (pattern, replacement, target))
        else:
            inc_nfail()
            print_result("FAIL", "s/%s/%s/g '%s'" % (pattern, replacement, target))
    else:
        # Matched
        if rxp.contents.outp:
            out_result = tstring_at(rxp.contents.outp)
        else:
            out_result = ""
        if isinstance(out_result, bytes):
            out_result = out_result.decode(encoding)

        if not_match:
            inc_nfail()
            print_result("FAIL(N)",
                    "s/%s/%s/g '%s' => '%s'" % (pattern, replacement, target, out_result))
        else:
            if out_result == s_result:
                inc_nsucc()
                print_result("OK",
                        "s/%s/%s/g '%s' => '%s'" % (pattern, replacement, target, out_result))
            else:
                inc_nfail()
                print_result("FAIL",
                        "s/%s/%s/g '%s' => '%s'" % (pattern, replacement, target, out_result))
    
    if (rxp):
        BRegfree(rxp)


def main():
    unicode_func = False
    
    # encoding of the test target
    enc = None
    if len(sys.argv) > 1:
        enc = sys.argv[1]

    # encoding of stdout/stderr
    outenc = None
    if len(sys.argv) > 2:
        outenc = sys.argv[2]

    # Initialization
    try:
        unicode_func = init(enc, outenc)
    except KeyError:
        print("test target encoding error")
        print("Usage: python test_match.py [test target encoding] [output encoding]")
        sys.exit()
    
    
    LoadBregonig(unicode_func)
    
    print(BRegexpVersion())
    print()
    
    
    # fixed string
    xx("abc", "def", "abc", "def")
    xx("abc", "def", "abcabcabc", "defdefdef")
    xx("あいう", "えお", "あいうあいう", "えおえお")
    xx("x?", "!", "abcde", "!a!b!c!d!e!")
    xx("\\Gx?", "!", "abcde", "!abcde")
    xx("abc", "\\r\\n\\t\\f\\e\\a\\b\\c[\\x30\\x{31}\\62\\o{63}", "abc", "\r\n\t\f\x1b\a\b\x1b0123")
    
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
    
    print("\nEncoding:", get_encoding())
    print("RESULT   SUCC: %d,  FAIL: %d,  ERROR: %d\n" % (
           get_nsucc(), get_nfail(), get_nerror()))

    if (get_nfail() == 0 and get_nerror() == 0):
        exit(0)
    else:
        exit(-1)

if __name__ == '__main__':
    main()


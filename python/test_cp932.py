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

def xx(pattern, target, s_from, s_to, mem, not_match):
    global nerror
    global nsucc
    global nfail
    
    rxp = POINTER(BREGEXP)()
    msg = create_tchar_buffer(BREGEXP_MAX_ERROR_MESSAGE_LEN)
    
    pattern2 = pattern
    if not isinstance(pattern, bytes):
        pattern2 = pattern.encode(encoding)
    pattern3 = "/".encode(encoding) + pattern2 + "/k".encode(encoding)
    if encoding == "UTF-16LE":
        pattern2 = pattern2.decode(encoding)
        pattern3 = pattern3.decode(encoding)
    
    target2 = target
    if not isinstance(target, bytes):
        s_from = cc_to_cb(target, encoding, s_from)
        s_to = cc_to_cb(target, encoding, s_to)
        target2 = target.encode(encoding)
    tp = strptr(target2)
    
    if encoding == "UTF-8":
        r = BoMatch(pattern2, "8", tp.getptr(), tp.getptr(), tp.getptr(-1),
                False, byref(rxp), msg)
    else:
        try:
            r = BoMatch(pattern2, "k", tp.getptr(), tp.getptr(), tp.getptr(-1),
                    False, byref(rxp), msg)
        except RuntimeError:
            r = BMatch(pattern3, tp.getptr(), tp.getptr(-1), byref(rxp), msg)
    
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
            start = rxp.contents.startp[mem] - tp.getptr()
            end = rxp.contents.endp[mem] - tp.getptr()
            if (start == s_from) and (end == s_to):
                nsucc += 1
                print("OK: /%s/ '%s'" % (pattern, target))
            else:
                nfail += 1
                print("FAIL: /%s/ '%s' %d-%d : %d-%d\n" % (pattern, target,
                        s_from, s_to, start, end))
    
    if (rxp):
        BRegfree(rxp)

def x2(pattern, target, s_from, s_to):
    xx(pattern, target, s_from, s_to, 0, False)

def x3(pattern, target, s_from, s_to, mem):
    xx(pattern, target, s_from, s_to, mem, False)

def n(pattern, target):
    xx(pattern, target, 0, 0, 0, True)



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
    
    
    # onig-5.9.2/win32/testc.c ����R�s�[
    #   trigraph �΍�� ?\? �� ?? �ɒu������
    #   (?m) �� (?s) �ɒu������
    #   \C-x �̓T�|�[�g���Ă��Ȃ����߃R�����g�A�E�g
    #   ONIG_OPTION_CAPTURE_GROUP ��L���ɂ��Ă��邽�߁A�o�b�t�@�ԍ���ύX
    #   �}�b�`�ʒu�̎w����o�C�g�P�ʂ��當�����P�ʂɕύX
    
    x2(u"", u"", 0, 0);
    x2(u"^", u"", 0, 0);
    x2(u"$", u"", 0, 0);
    x2(u"\\G", u"", 0, 0);
    x2(u"\\A", u"", 0, 0);
    x2(u"\\Z", u"", 0, 0);
    x2(u"\\z", u"", 0, 0);
    x2(u"^$", u"", 0, 0);
    x2(u"\\ca", u"\001", 0, 1);
    #x2(u"\\C-b", u"\002", 0, 1);  #XXX: \C-x is not supported
    x2(u"\\c\\\\", u"\034", 0, 1);
    x2(u"q[\\c\\\\]", u"q\034", 0, 2);
    x2(u"", u"a", 0, 0);
    x2(u"a", u"a", 0, 1);
    if encoding == "UTF-16LE":
        x2(u"\\x61\\x00", u"a", 0, 1);
    else:
        x2(u"\\x61", u"a", 0, 1);
    x2(u"aa", u"aa", 0, 2);
    x2(u"aaa", u"aaa", 0, 3);
    x2(u"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", u"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 0, 35);
    x2(u"ab", u"ab", 0, 2);
    x2(u"b", u"ab", 1, 2);
    x2(u"bc", u"abc", 1, 3);
    x2(u"(?i:#RET#)", u"#INS##RET#", 5, 10);
    if encoding == "UTF-16LE":
        x2(u"\\17\\00", u"\017", 0, 1);
        x2(u"\\x1f\\x00", u"\x1f", 0, 1);
    else:
        x2(u"\\17", u"\017", 0, 1);
        x2(u"\\x1f", u"\x1f", 0, 1);
    x2(u"a(?#....\\\\JJJJ)b", u"ab", 0, 2);
    x2(u"(?x)  G (o O(?-x)oO) g L", u"GoOoOgLe", 0, 7);
    x2(u".", u"a", 0, 1);
    n(u".", u"");
    x2(u"..", u"ab", 0, 2);
    x2(u"\\w", u"e", 0, 1);
    n(u"\\W", u"e");
    x2(u"\\s", u" ", 0, 1);
    x2(u"\\S", u"b", 0, 1);
    x2(u"\\d", u"4", 0, 1);
    n(u"\\D", u"4");
    x2(u"\\b", u"z ", 0, 0);
    x2(u"\\b", u" z", 1, 1);
    x2(u"\\B", u"zz ", 1, 1);
    x2(u"\\B", u"z ", 2, 2);
    x2(u"\\B", u" z", 0, 0);
    x2(u"[ab]", u"b", 0, 1);
    n(u"[ab]", u"c");
    x2(u"[a-z]", u"t", 0, 1);
    n(u"[^a]", u"a");
    x2(u"[^a]", u"\n", 0, 1);
    x2(u"[]]", u"]", 0, 1);
    n(u"[^]]", u"]");
    x2(u"[\\^]+", u"0^^1", 1, 3);
    x2(u"[b-]", u"b", 0, 1);
    x2(u"[b-]", u"-", 0, 1);
    x2(u"[\\w]", u"z", 0, 1);
    n(u"[\\w]", u" ");
    x2(u"[\\W]", u"b$", 1, 2);
    x2(u"[\\d]", u"5", 0, 1);
    n(u"[\\d]", u"e");
    x2(u"[\\D]", u"t", 0, 1);
    n(u"[\\D]", u"3");
    x2(u"[\\s]", u" ", 0, 1);
    n(u"[\\s]", u"a");
    x2(u"[\\S]", u"b", 0, 1);
    n(u"[\\S]", u" ");
    x2(u"[\\w\\d]", u"2", 0, 1);
    n(u"[\\w\\d]", u" ");
    x2(u"[[:upper:]]", u"B", 0, 1);
    x2(u"[*[:xdigit:]+]", u"+", 0, 1);
    x2(u"[*[:xdigit:]+]", u"GHIKK-9+*", 6, 7);
    x2(u"[*[:xdigit:]+]", u"-@^+", 3, 4);
    n(u"[[:upper]]", u"A");
    x2(u"[[:upper]]", u":", 0, 1);
    if encoding == "UTF-16LE":
        x2(u"[\\044\\000-\\047\\000]", u"\046", 0, 1);
        x2(u"[\\x5a\\x00-\\x5c\\x00]", u"\x5b", 0, 1);
        x2(u"[\\x6A\\x00-\\x6D\\x00]", u"\x6c", 0, 1);
        n(u"[\\x6A\\x00-\\x6D\\x00]", u"\x6E");
    else:
        x2(u"[\\044-\\047]", u"\046", 0, 1);
        x2(u"[\\x5a-\\x5c]", u"\x5b", 0, 1);
        x2(u"[\\x6A-\\x6D]", u"\x6c", 0, 1);
        n(u"[\\x6A-\\x6D]", u"\x6E");
    n(u"^[0-9A-F]+ 0+ UNDEF ", u"75F 00000000 SECT14A notype ()    External    | _rb_apply");
    x2(u"[\\[]", u"[", 0, 1);
    x2(u"[\\]]", u"]", 0, 1);
    x2(u"[&]", u"&", 0, 1);
    x2(u"[[ab]]", u"b", 0, 1);
    x2(u"[[ab]c]", u"c", 0, 1);
    n(u"[[^a]]", u"a");
    n(u"[^[a]]", u"a");
    x2(u"[[ab]&&bc]", u"b", 0, 1);
    n(u"[[ab]&&bc]", u"a");
    n(u"[[ab]&&bc]", u"c");
    x2(u"[a-z&&b-y&&c-x]", u"w", 0, 1);
    n(u"[^a-z&&b-y&&c-x]", u"w");
    x2(u"[[^a&&a]&&a-z]", u"b", 0, 1);
    n(u"[[^a&&a]&&a-z]", u"a");
    x2(u"[[^a-z&&bcdef]&&[^c-g]]", u"h", 0, 1);
    n(u"[[^a-z&&bcdef]&&[^c-g]]", u"c");
    x2(u"[^[^abc]&&[^cde]]", u"c", 0, 1);
    x2(u"[^[^abc]&&[^cde]]", u"e", 0, 1);
    n(u"[^[^abc]&&[^cde]]", u"f");
    x2(u"[a-&&-a]", u"-", 0, 1);
    n(u"[a\\-&&\\-a]", u"&");
    n(u"\\wabc", u" abc");
    x2(u"a\\Wbc", u"a bc", 0, 4);
    x2(u"a.b.c", u"aabbc", 0, 5);
    x2(u".\\wb\\W..c", u"abb bcc", 0, 7);
    x2(u"\\s\\wzzz", u" zzzz", 0, 5);
    x2(u"aa.b", u"aabb", 0, 4);
    n(u".a", u"ab");
    x2(u".a", u"aa", 0, 2);
    x2(u"^a", u"a", 0, 1);
    x2(u"^a$", u"a", 0, 1);
    x2(u"^\\w$", u"a", 0, 1);
    n(u"^\\w$", u" ");
    x2(u"^\\wab$", u"zab", 0, 3);
    x2(u"^\\wabcdef$", u"zabcdef", 0, 7);
    x2(u"^\\w...def$", u"zabcdef", 0, 7);
    x2(u"\\w\\w\\s\\Waaa\\d", u"aa  aaa4", 0, 8);
    x2(u"\\A\\Z", u"", 0, 0);
    x2(u"\\Axyz", u"xyz", 0, 3);
    x2(u"xyz\\Z", u"xyz", 0, 3);
    x2(u"xyz\\z", u"xyz", 0, 3);
    x2(u"a\\Z", u"a", 0, 1);
    x2(u"\\Gaz", u"az", 0, 2);
    n(u"\\Gz", u"bza");
    n(u"az\\G", u"az");
    n(u"az\\A", u"az");
    n(u"a\\Az", u"az");
    x2(u"\\^\\$", u"^$", 0, 2);
    x2(u"^x?y", u"xy", 0, 2);
    x2(u"^(x?y)", u"xy", 0, 2);
    x2(u"\\w", u"_", 0, 1);
    n(u"\\W", u"_");
    x2(u"(?=z)z", u"z", 0, 1);
    n(u"(?=z).", u"a");
    x2(u"(?!z)a", u"a", 0, 1);
    n(u"(?!z)a", u"z");
    x2(u"(?i:a)", u"a", 0, 1);
    x2(u"(?i:a)", u"A", 0, 1);
    x2(u"(?i:A)", u"a", 0, 1);
    n(u"(?i:A)", u"b");
    x2(u"(?i:[A-Z])", u"a", 0, 1);
    x2(u"(?i:[f-m])", u"H", 0, 1);
    x2(u"(?i:[f-m])", u"h", 0, 1);
    n(u"(?i:[f-m])", u"e");
    x2(u"(?i:[A-c])", u"D", 0, 1);
    n(u"(?i:[^a-z])", u"A");
    n(u"(?i:[^a-z])", u"a");
    x2(u"(?i:[!-k])", u"Z", 0, 1);
    x2(u"(?i:[!-k])", u"7", 0, 1);
    x2(u"(?i:[T-}])", u"b", 0, 1);
    x2(u"(?i:[T-}])", u"{", 0, 1);
    x2(u"(?i:\\?a)", u"?A", 0, 2);
    x2(u"(?i:\\*A)", u"*a", 0, 2);
    n(u".", u"\n");
    x2(u"(?s:.)", u"\n", 0, 1);           #XXX: m -> s
    x2(u"(?s:a.)", u"a\n", 0, 2);         #XXX: m -> s
    x2(u"(?s:.b)", u"a\nb", 1, 3);        #XXX: m -> s
    x2(u".*abc", u"dddabdd\nddabc", 8, 13);
    x2(u"(?s:.*abc)", u"dddabddabc", 0, 10);  #XXX: m -> s
    n(u"(?i)(?-i)a", u"A");
    n(u"(?i)(?-i:a)", u"A");
    x2(u"a?", u"", 0, 0);
    x2(u"a?", u"b", 0, 0);
    x2(u"a?", u"a", 0, 1);
    x2(u"a*", u"", 0, 0);
    x2(u"a*", u"a", 0, 1);
    x2(u"a*", u"aaa", 0, 3);
    x2(u"a*", u"baaaa", 0, 0);
    n(u"a+", u"");
    x2(u"a+", u"a", 0, 1);
    x2(u"a+", u"aaaa", 0, 4);
    x2(u"a+", u"aabbb", 0, 2);
    x2(u"a+", u"baaaa", 1, 5);
    x2(u".?", u"", 0, 0);
    x2(u".?", u"f", 0, 1);
    x2(u".?", u"\n", 0, 0);
    x2(u".*", u"", 0, 0);
    x2(u".*", u"abcde", 0, 5);
    x2(u".+", u"z", 0, 1);
    x2(u".+", u"zdswer\n", 0, 6);
    x2(u"(.*)a\\1f", u"babfbac", 0, 4);
    x2(u"(.*)a\\1f", u"bacbabf", 3, 7);
    x2(u"((.*)a\\2f)", u"bacbabf", 3, 7);
    x2(u"(.*)a\\1f", u"baczzzzzz\nbazz\nzzzzbabf", 19, 23);
    x2(u"a|b", u"a", 0, 1);
    x2(u"a|b", u"b", 0, 1);
    x2(u"|a", u"a", 0, 0);
    x2(u"(|a)", u"a", 0, 0);
    x2(u"ab|bc", u"ab", 0, 2);
    x2(u"ab|bc", u"bc", 0, 2);
    x2(u"z(?:ab|bc)", u"zbc", 0, 3);
    x2(u"a(?:ab|bc)c", u"aabc", 0, 4);
    x2(u"ab|(?:ac|az)", u"az", 0, 2);
    x2(u"a|b|c", u"dc", 1, 2);
    x2(u"a|b|cd|efg|h|ijk|lmn|o|pq|rstuvwx|yz", u"pqr", 0, 2);
    n(u"a|b|cd|efg|h|ijk|lmn|o|pq|rstuvwx|yz", u"mn");
    x2(u"a|^z", u"ba", 1, 2);
    x2(u"a|^z", u"za", 0, 1);
    x2(u"a|\\Gz", u"bza", 2, 3);
    x2(u"a|\\Gz", u"za", 0, 1);
    x2(u"a|\\Az", u"bza", 2, 3);
    x2(u"a|\\Az", u"za", 0, 1);
    x2(u"a|b\\Z", u"ba", 1, 2);
    x2(u"a|b\\Z", u"b", 0, 1);
    x2(u"a|b\\z", u"ba", 1, 2);
    x2(u"a|b\\z", u"b", 0, 1);
    x2(u"\\w|\\s", u" ", 0, 1);
    n(u"\\w|\\w", u" ");
    x2(u"\\w|%", u"%", 0, 1);
    x2(u"\\w|[&$]", u"&", 0, 1);
    x2(u"[b-d]|[^e-z]", u"a", 0, 1);
    x2(u"(?:a|[c-f])|bz", u"dz", 0, 1);
    x2(u"(?:a|[c-f])|bz", u"bz", 0, 2);
    x2(u"abc|(?=zz)..f", u"zzf", 0, 3);
    x2(u"abc|(?!zz)..f", u"abf", 0, 3);
    x2(u"(?=za)..a|(?=zz)..a", u"zza", 0, 3);
    n(u"(?>a|abd)c", u"abdc");
    x2(u"(?>abd|a)c", u"abdc", 0, 4);
    x2(u"a?|b", u"a", 0, 1);
    x2(u"a?|b", u"b", 0, 0);
    x2(u"a?|b", u"", 0, 0);
    x2(u"a*|b", u"aa", 0, 2);
    x2(u"a*|b*", u"ba", 0, 0);
    x2(u"a*|b*", u"ab", 0, 1);
    x2(u"a+|b*", u"", 0, 0);
    x2(u"a+|b*", u"bbb", 0, 3);
    x2(u"a+|b*", u"abbb", 0, 1);
    n(u"a+|b+", u"");
    x2(u"(a|b)?", u"b", 0, 1);
    x2(u"(a|b)*", u"ba", 0, 2);
    x2(u"(a|b)+", u"bab", 0, 3);
    x2(u"(ab|ca)+", u"caabbc", 0, 4);
    x2(u"(ab|ca)+", u"aabca", 1, 5);
    x2(u"(ab|ca)+", u"abzca", 0, 2);
    x2(u"(a|bab)+", u"ababa", 0, 5);
    x2(u"(a|bab)+", u"ba", 1, 2);
    x2(u"(a|bab)+", u"baaaba", 1, 4);
    x2(u"(?:a|b)(?:a|b)", u"ab", 0, 2);
    x2(u"(?:a*|b*)(?:a*|b*)", u"aaabbb", 0, 3);
    x2(u"(?:a*|b*)(?:a+|b+)", u"aaabbb", 0, 6);
    x2(u"(?:a+|b+){2}", u"aaabbb", 0, 6);
    x2(u"h{0,}", u"hhhh", 0, 4);
    x2(u"(?:a+|b+){1,2}", u"aaabbb", 0, 6);
    n(u"ax{2}*a", u"0axxxa1");
    n(u"a.{0,2}a", u"0aXXXa0");
    n(u"a.{0,2}?a", u"0aXXXa0");
    n(u"a.{0,2}?a", u"0aXXXXa0");
    x2(u"^a{2,}?a$", u"aaa", 0, 3);
    x2(u"^[a-z]{2,}?$", u"aaa", 0, 3);
    x2(u"(?:a+|\\Ab*)cc", u"cc", 0, 2);
    n(u"(?:a+|\\Ab*)cc", u"abcc");
    x2(u"(?:^a+|b+)*c", u"aabbbabc", 6, 8);
    x2(u"(?:^a+|b+)*c", u"aabbbbc", 0, 7);
    x2(u"a|(?i)c", u"C", 0, 1);
    x2(u"(?i)c|a", u"C", 0, 1);
    x2(u"(?i)c|a", u"A", 0, 1);
    x2(u"(?i:c)|a", u"C", 0, 1);
    n(u"(?i:c)|a", u"A");
    x2(u"[abc]?", u"abc", 0, 1);
    x2(u"[abc]*", u"abc", 0, 3);
    x2(u"[^abc]*", u"abc", 0, 0);
    n(u"[^abc]+", u"abc");
    x2(u"a??", u"aaa", 0, 0);
    x2(u"ba??b", u"bab", 0, 3);
    x2(u"a*?", u"aaa", 0, 0);
    x2(u"ba*?", u"baa", 0, 1);
    x2(u"ba*?b", u"baab", 0, 4);
    x2(u"a+?", u"aaa", 0, 1);
    x2(u"ba+?", u"baa", 0, 2);
    x2(u"ba+?b", u"baab", 0, 4);
    x2(u"(?:a?)??", u"a", 0, 0);
    x2(u"(?:a??)?", u"a", 0, 0);
    x2(u"(?:a?)+?", u"aaa", 0, 1);
    x2(u"(?:a+)??", u"aaa", 0, 0);
    x2(u"(?:a+)??b", u"aaab", 0, 4);
    x2(u"(?:ab)?{2}", u"", 0, 0);
    x2(u"(?:ab)?{2}", u"ababa", 0, 4);
    x2(u"(?:ab)*{0}", u"ababa", 0, 0);
    x2(u"(?:ab){3,}", u"abababab", 0, 8);
    n(u"(?:ab){3,}", u"abab");
    x2(u"(?:ab){2,4}", u"ababab", 0, 6);
    x2(u"(?:ab){2,4}", u"ababababab", 0, 8);
    x2(u"(?:ab){2,4}?", u"ababababab", 0, 4);
    x2(u"(?:ab){,}", u"ab{,}", 0, 5);
    x2(u"(?:abc)+?{2}", u"abcabcabc", 0, 6);
    x2(u"(?:X*)(?i:xa)", u"XXXa", 0, 4);
    x2(u"(d+)([^abc]z)", u"dddz", 0, 4);
    x2(u"([^abc]*)([^abc]z)", u"dddz", 0, 4);
    x2(u"(\\w+)(\\wz)", u"dddz", 0, 4);
    x3(u"(a)", u"a", 0, 1, 1);
    x3(u"(ab)", u"ab", 0, 2, 1);
    x2(u"((ab))", u"ab", 0, 2);
    x3(u"((ab))", u"ab", 0, 2, 1);
    x3(u"((ab))", u"ab", 0, 2, 2);
    x3(u"((((((((((((((((((((ab))))))))))))))))))))", u"ab", 0, 2, 20);
    x3(u"(ab)(cd)", u"abcd", 0, 2, 1);
    x3(u"(ab)(cd)", u"abcd", 2, 4, 2);
    x3(u"()(a)bc(def)ghijk", u"abcdefghijk", 3, 6, 3);
    x3(u"(()(a)bc(def)ghijk)", u"abcdefghijk", 3, 6, 4);
    x2(u"(^a)", u"a", 0, 1);
    x3(u"(a)|(a)", u"ba", 1, 2, 1);
    x3(u"(^a)|(a)", u"ba", 1, 2, 2);
    x3(u"(a?)", u"aaa", 0, 1, 1);
    x3(u"(a*)", u"aaa", 0, 3, 1);
    x3(u"(a*)", u"", 0, 0, 1);
    x3(u"(a+)", u"aaaaaaa", 0, 7, 1);
    x3(u"(a+|b*)", u"bbbaa", 0, 3, 1);
    x3(u"(a+|b?)", u"bbbaa", 0, 1, 1);
    x3(u"(abc)?", u"abc", 0, 3, 1);
    x3(u"(abc)*", u"abc", 0, 3, 1);
    x3(u"(abc)+", u"abc", 0, 3, 1);
    x3(u"(xyz|abc)+", u"abc", 0, 3, 1);
    x3(u"([xyz][abc]|abc)+", u"abc", 0, 3, 1);
    x3(u"((?i:abc))", u"AbC", 0, 3, 1);
    x2(u"(abc)(?i:\\1)", u"abcABC", 0, 6);
    x3(u"((?s:a.c))", u"a\nc", 0, 3, 1);  #XXX m -> s
    x3(u"((?=az)a)", u"azb", 0, 1, 1);
    x3(u"abc|(.abd)", u"zabd", 0, 4, 1);
    x2(u"(?:abc)|(ABC)", u"abc", 0, 3);
    x3(u"(?i:(abc))|(zzz)", u"ABC", 0, 3, 1);
    x3(u"a*(.)", u"aaaaz", 4, 5, 1);
    x3(u"a*?(.)", u"aaaaz", 0, 1, 1);
    x3(u"a*?(c)", u"aaaac", 4, 5, 1);
    x3(u"[bcd]a*(.)", u"caaaaz", 5, 6, 1);
    x3(u"(\\Abb)cc", u"bbcc", 0, 2, 1);
    n(u"(\\Abb)cc", u"zbbcc");
    x3(u"(^bb)cc", u"bbcc", 0, 2, 1);
    n(u"(^bb)cc", u"zbbcc");
    x3(u"cc(bb$)", u"ccbb", 2, 4, 1);
    n(u"cc(bb$)", u"ccbbb");
    n(u"(\\1)", u"");
    n(u"\\1(a)", u"aa");
    n(u"(a(b)\\1)\\2+", u"ababb");
    n(u"(?:(?:\\1|z)(a))+$", u"zaa");
    x2(u"(?:(?:\\1|z)(a))+$", u"zaaa", 0, 4);
    x2(u"(a)(?=\\1)", u"aa", 0, 1);
    n(u"(a)$|\\1", u"az");
    x2(u"(a)\\1", u"aa", 0, 2);
    n(u"(a)\\1", u"ab");
    x2(u"(a?)\\1", u"aa", 0, 2);
    x2(u"(a??)\\1", u"aa", 0, 0);
    x2(u"(a*)\\1", u"aaaaa", 0, 4);
    x3(u"(a*)\\1", u"aaaaa", 0, 2, 1);
    x2(u"a(b*)\\1", u"abbbb", 0, 5);
    x2(u"a(b*)\\1", u"ab", 0, 1);
    x2(u"(a*)(b*)\\1\\2", u"aaabbaaabb", 0, 10);
    x2(u"(a*)(b*)\\2", u"aaabbbb", 0, 7);
    x2(u"(((((((a*)b))))))c\\7", u"aaabcaaa", 0, 8);
    x3(u"(((((((a*)b))))))c\\7", u"aaabcaaa", 0, 3, 7);
    x2(u"(a)(b)(c)\\2\\1\\3", u"abcbac", 0, 6);
    x2(u"([a-d])\\1", u"cc", 0, 2);
    x2(u"(\\w\\d\\s)\\1", u"f5 f5 ", 0, 6);
    n(u"(\\w\\d\\s)\\1", u"f5 f5");
    x2(u"(who|[a-c]{3})\\1", u"whowho", 0, 6);
    x2(u"...(who|[a-c]{3})\\1", u"abcwhowho", 0, 9);
    x2(u"(who|[a-c]{3})\\1", u"cbccbc", 0, 6);
    x2(u"(^a)\\1", u"aa", 0, 2);
    n(u"(^a)\\1", u"baa");
    n(u"(a$)\\1", u"aa");
    n(u"(ab\\Z)\\1", u"ab");
    x2(u"(a*\\Z)\\1", u"a", 1, 1);
    x2(u".(a*\\Z)\\1", u"ba", 1, 2);
    x3(u"(.(abc)\\2)", u"zabcabc", 0, 7, 1);
    x3(u"(.(..\\d.)\\2)", u"z12341234", 0, 9, 1);
    x2(u"((?i:az))\\1", u"AzAz", 0, 4);
    n(u"((?i:az))\\1", u"Azaz");
    x2(u"(?<=a)b", u"ab", 1, 2);
    n(u"(?<=a)b", u"bb");
    x2(u"(?<=a|b)b", u"bb", 1, 2);
    x2(u"(?<=a|bc)b", u"bcb", 2, 3);
    x2(u"(?<=a|bc)b", u"ab", 1, 2);
    x2(u"(?<=a|bc||defghij|klmnopq|r)z", u"rz", 1, 2);
    x2(u"(a)\\g<1>", u"aa", 0, 2);
    x2(u"(?<!a)b", u"cb", 1, 2);
    n(u"(?<!a)b", u"ab");
    x2(u"(?<!a|bc)b", u"bbb", 0, 1);
    n(u"(?<!a|bc)z", u"bcz");
    x2(u"(?<name1>a)", u"a", 0, 1);
    x2(u"(?<name_2>ab)\\g<name_2>", u"abab", 0, 4);
    x2(u"(?<name_3>.zv.)\\k<name_3>", u"azvbazvb", 0, 8);
    x2(u"(?<=\\g<ab>)|-\\zEND (?<ab>XyZ)", u"XyZ", 3, 3);
    x2(u"(?<n>|a\\g<n>)+", u"", 0, 0);
    x2(u"(?<n>|\\(\\g<n>\\))+$", u"()(())", 0, 6);
    x3(u"\\g<n>(?<n>.){0}", u"X", 0, 1, 1);
    x2(u"\\g<n>(abc|df(?<n>.YZ){2,8}){0}", u"XYZ", 0, 3);
    x2(u"\\A(?<n>(a\\g<n>)|)\\z", u"aaaa", 0, 4);
    x2(u"(?<n>|\\g<m>\\g<n>)\\z|\\zEND (?<m>a|(b)\\g<m>)", u"bbbbabba", 0, 8);
    x2(u"(?<name1240>\\w+\\sx)a+\\k<name1240>", u"  fg xaaaaaaaafg x", 2, 18);
    x3(u"(z)()()(?<_9>a)\\g<_9>", u"zaa", 2, 3, 4);   #XXX: memory number 1 -> 4
    x2(u"(.)(((?<_>a)))\\k<_>", u"zaa", 0, 3);
    x2(u"((?<name1>\\d)|(?<name2>\\w))(\\k<name1>|\\k<name2>)", u"ff", 0, 2);
    x2(u"(?:(?<x>)|(?<x>efg))\\k<x>", u"", 0, 0);
    x2(u"(?:(?<x>abc)|(?<x>efg))\\k<x>", u"abcefgefg", 3, 9);
    n(u"(?:(?<x>abc)|(?<x>efg))\\k<x>", u"abcefg");
    x2(u"(?:(?<n1>.)|(?<n1>..)|(?<n1>...)|(?<n1>....)|(?<n1>.....)|(?<n1>......)|(?<n1>.......)|(?<n1>........)|(?<n1>.........)|(?<n1>..........)|(?<n1>...........)|(?<n1>............)|(?<n1>.............)|(?<n1>..............))\\k<n1>$", u"a-pyumpyum", 2, 10);
    x3(u"(?:(?<n1>.)|(?<n1>..)|(?<n1>...)|(?<n1>....)|(?<n1>.....)|(?<n1>......)|(?<n1>.......)|(?<n1>........)|(?<n1>.........)|(?<n1>..........)|(?<n1>...........)|(?<n1>............)|(?<n1>.............)|(?<n1>..............))\\k<n1>$", u"xxxxabcdefghijklmnabcdefghijklmn", 4, 18, 14);
    x3(u"(?<name1>)(?<name2>)(?<name3>)(?<name4>)(?<name5>)(?<name6>)(?<name7>)(?<name8>)(?<name9>)(?<name10>)(?<name11>)(?<name12>)(?<name13>)(?<name14>)(?<name15>)(?<name16>aaa)(?<name17>)$", u"aaa", 0, 3, 16);
    x2(u"(?<foo>a|\\(\\g<foo>\\))", u"a", 0, 1);
    x2(u"(?<foo>a|\\(\\g<foo>\\))", u"((((((a))))))", 0, 13);
    x3(u"(?<foo>a|\\(\\g<foo>\\))", u"((((((((a))))))))", 0, 17, 1);
    x2(u"\\g<bar>|\\zEND(?<bar>.*abc$)", u"abcxxxabc", 0, 9);
    x2(u"\\g<1>|\\zEND(.a.)", u"bac", 0, 3);
    x3(u"\\g<_A>\\g<_A>|\\zEND(.a.)(?<_A>.b.)", u"xbxyby", 3, 6, 2);  #XXX: memory number 1 -> 2
    x2(u"\\A(?:\\g<pon>|\\g<pan>|\\zEND  (?<pan>a|c\\g<pon>c)(?<pon>b|d\\g<pan>d))$", u"cdcbcdc", 0, 7);
    x2(u"\\A(?<n>|a\\g<m>)\\z|\\zEND (?<m>\\g<n>)", u"aaaa", 0, 4);
    x2(u"(?<n>(a|b\\g<n>c){3,5})", u"baaaaca", 1, 5);
    x2(u"(?<n>(a|b\\g<n>c){3,5})", u"baaaacaaaaa", 0, 10);
    x2(u"(?<pare>\\(([^\\(\\)]++|\\g<pare>)*+\\))", u"((a))", 0, 5);
    x2(u"()*\\1", u"", 0, 0);
    x2(u"(?:()|())*\\1\\2", u"", 0, 0);
    x3(u"(?:\\1a|())*", u"a", 0, 0, 1);
    x2(u"x((.)*)*x", u"0x1x2x3", 1, 6);
    x2(u"x((.)*)*x(?i:\\1)\\Z", u"0x1x2x1X2", 1, 9);
    x2(u"(?:()|()|()|()|()|())*\\2\\5", u"", 0, 0);
    x2(u"(?:()|()|()|(x)|()|())*\\2b\\5", u"b", 0, 1);
    if encoding == "UTF-16LE":
        x2(u"\\xFA\\x8F", u"\u8ffa", 0, 1);
    elif encoding == "UTF-8":
        x2(u"\\xE8\\xBF\\xBA", u"\u8ffa", 0, 1);
    else:
        x2(u"\\xE7\\x92", u"\u8ffa", 0, 1); # "�"
    x2(u"", u"��", 0, 0);
    x2(u"��", u"��", 0, 1);
    n(u"��", u"��");
    x2(u"����", u"����", 0, 2);
    x2(u"������", u"������", 0, 3);
    x2(u"����������������������������������������������������������������������", u"����������������������������������������������������������������������", 0, 35);
    x2(u"��", u"����", 1, 2);
    x2(u"����", u"������", 1, 3);
#    x2(b"\\xca\\xb8", b"\xca\xb8", 0, 2);   # "��" (EUC-JP)
    x2(u".", u"��", 0, 1);
    x2(u"..", u"����", 0, 2);
    x2(u"\\w", u"��", 0, 1);
    n(u"\\W", u"��");
    x2(u"[\\W]", u"��$", 1, 2);
    x2(u"\\S", u"��", 0, 1);
    x2(u"\\S", u"��", 0, 1);
    x2(u"\\b", u"�C ", 0, 0);
    x2(u"\\b", u" ��", 1, 1);
    x2(u"\\B", u"���� ", 1, 1);
    x2(u"\\B", u"�� ", 2, 2);
    x2(u"\\B", u" ��", 0, 0);
    x2(u"[����]", u"��", 0, 1);
    n(u"[�Ȃ�]", u"��");
    x2(u"[��-��]", u"��", 0, 1);
    n(u"[^��]", u"��");
    x2(u"[\\w]", u"��", 0, 1);
    n(u"[\\d]", u"��");
    x2(u"[\\D]", u"��", 0, 1);
    n(u"[\\s]", u"��");
    x2(u"[\\S]", u"��", 0, 1);
    x2(u"[\\w\\d]", u"��", 0, 1);
    x2(u"[\\w\\d]", u"   ��", 3, 4);
    n(u"\\w�S��", u" �S��");
    x2(u"�S\\W��", u"�S ��", 0, 3);
    x2(u"��.��.��", u"����������", 0, 5);
    x2(u".\\w��\\W..��", u"������ ������", 0, 7);
    x2(u"\\s\\w������", u" ��������", 0, 5);
    x2(u"����.��", u"��������", 0, 4);
    n(u".��", u"����");
    x2(u".��", u"����", 0, 2);
    x2(u"^��", u"��", 0, 1);
    x2(u"^��$", u"��", 0, 1);
    x2(u"^\\w$", u"��", 0, 1);
    x2(u"^\\w����������$", u"z����������", 0, 6);
    x2(u"^\\w...������$", u"z������������", 0, 7);
    x2(u"\\w\\w\\s\\W������\\d", u"a��  ������4", 0, 8);
    x2(u"\\A������", u"������", 0, 3);
    x2(u"�ނ߂�\\Z", u"�ނ߂�", 0, 3);
    x2(u"������\\z", u"������", 0, 3);
    x2(u"������\\Z", u"������\n", 0, 3);
    x2(u"\\G�ۂ�", u"�ۂ�", 0, 2);
    n(u"\\G��", u"������");
    n(u"�Ƃ�\\G", u"�Ƃ�");
    n(u"�܂�\\A", u"�܂�");
    n(u"��\\A��", u"�܂�");
    x2(u"(?=��)��", u"��", 0, 1);
    n(u"(?=��).", u"��");
    x2(u"(?!��)��", u"��", 0, 1);
    n(u"(?!��)��", u"��");
    x2(u"(?i:��)", u"��", 0, 1);
    x2(u"(?i:�Ԃ�)", u"�Ԃ�", 0, 2);
    n(u"(?i:��)", u"��");
    x2(u"(?s:��.)", u"��\n", 0, 2);     #XXX: m -> s
    x2(u"(?s:.��)", u"��\n��", 1, 3);   #XXX: m -> s
    x2(u"��?", u"", 0, 0);
    x2(u"��?", u"��", 0, 0);
    x2(u"��?", u"��", 0, 1);
    x2(u"��*", u"", 0, 0);
    x2(u"��*", u"��", 0, 1);
    x2(u"�q*", u"�q�q�q", 0, 3);
    x2(u"�n*", u"���n�n�n�n", 0, 0);
    n(u"�R+", u"");
    x2(u"��+", u"��", 0, 1);
    x2(u"��+", u"��������", 0, 4);
    x2(u"��+", u"����������", 0, 2);
    x2(u"��+", u"����������", 1, 5);
    x2(u".?", u"��", 0, 1);
    x2(u".*", u"�ς҂Ղ�", 0, 4);
    x2(u".+", u"��", 0, 1);
    x2(u".+", u"��������\n", 0, 4);
    x2(u"��|��", u"��", 0, 1);
    x2(u"��|��", u"��", 0, 1);
    x2(u"����|����", u"����", 0, 2);
    x2(u"����|����", u"����", 0, 2);
    x2(u"��(?:����|����)", u"������", 0, 3);
    x2(u"��(?:����|����)��", u"��������", 0, 4);
    x2(u"����|(?:����|����)", u"����", 0, 2);
    x2(u"��|��|��", u"����", 1, 2);
    x2(u"��|��|����|������|��|������|������|��|����|�ĂƂȂ�|�ʂ�", u"������", 0, 3);
    n(u"��|��|����|������|��|������|������|��|����|�ĂƂȂ�|�ʂ�", u"����");
    x2(u"��|^��", u"�Ԃ�", 1, 2);
    x2(u"��|^��", u"����", 0, 1);
    x2(u"�S|\\G��", u"���ԋS", 2, 3);
    x2(u"�S|\\G��", u"�ԋS", 0, 1);
    x2(u"�S|\\A��", u"b�ԋS", 2, 3);
    x2(u"�S|\\A��", u"��", 0, 1);
    x2(u"�S|��\\Z", u"�ԋS", 1, 2);
    x2(u"�S|��\\Z", u"��", 0, 1);
    x2(u"�S|��\\Z", u"��\n", 0, 1);
    x2(u"�S|��\\z", u"�ԋS", 1, 2);
    x2(u"�S|��\\z", u"��", 0, 1);
    x2(u"\\w|\\s", u"��", 0, 1);
    x2(u"\\w|%", u"%��", 0, 1);
    x2(u"\\w|[&$]", u"��&", 0, 1);
    x2(u"[��-��]", u"��", 0, 1);
    x2(u"[��-��]|[^��-��]", u"��", 0, 1);
    x2(u"[��-��]|[^��-��]", u"��", 0, 1);
    x2(u"[^��]", u"\n", 0, 1);
    x2(u"(?:��|[��-��])|����", u"����", 0, 1);
    x2(u"(?:��|[��-��])|����", u"����", 0, 2);
    x2(u"������|(?=����)..��", u"������", 0, 3);
    x2(u"������|(?!����)..��", u"������", 0, 3);
    x2(u"(?=����)..��|(?=����)..��", u"������", 0, 3);
    x2(u"(?<=��|����)��", u"������", 2, 3);
    n(u"(?>��|������)��", u"��������");
    x2(u"(?>������|��)��", u"��������", 0, 4);
    x2(u"��?|��", u"��", 0, 1);
    x2(u"��?|��", u"��", 0, 0);
    x2(u"��?|��", u"", 0, 0);
    x2(u"��*|��", u"����", 0, 2);
    x2(u"��*|��*", u"����", 0, 0);
    x2(u"��*|��*", u"����", 0, 1);
    x2(u"[a��]*|��*", u"a��������", 0, 2);
    x2(u"��+|��*", u"", 0, 0);
    x2(u"��+|��*", u"������", 0, 3);
    x2(u"��+|��*", u"��������", 0, 1);
    x2(u"��+|��*", u"a��������", 0, 0);
    n(u"��+|��+", u"");
    x2(u"(��|��)?", u"��", 0, 1);
    x2(u"(��|��)*", u"����", 0, 2);
    x2(u"(��|��)+", u"������", 0, 3);
    x2(u"(����|����)+", u"������������", 0, 4);
    x2(u"(����|����)+", u"������������", 2, 6);
    x2(u"(����|����)+", u"����������", 1, 5);
    x2(u"(����|����)+", u"����������", 0, 2);
    x2(u"(����|����)+", u"$$zzzz����������", 6, 8);
    x2(u"(��|������)+", u"����������", 0, 5);
    x2(u"(��|������)+", u"����", 1, 2);
    x2(u"(��|������)+", u"������������", 1, 4);
    x2(u"(?:��|��)(?:��|��)", u"����", 0, 2);
    x2(u"(?:��*|��*)(?:��*|��*)", u"������������", 0, 3);
    x2(u"(?:��*|��*)(?:��+|��+)", u"������������", 0, 6);
    x2(u"(?:��+|��+){2}", u"������������", 0, 6);
    x2(u"(?:��+|��+){1,2}", u"������������", 0, 6);
    x2(u"(?:��+|\\A��*)����", u"����", 0, 2);
    n(u"(?:��+|\\A��*)����", u"��������");
    x2(u"(?:^��+|��+)*��", u"����������������", 6, 8);
    x2(u"(?:^��+|��+)*��", u"��������������", 0, 7);
    x2(u"��{0,}", u"��������", 0, 4);
    x2(u"��|(?i)c", u"C", 0, 1);
    x2(u"(?i)c|��", u"C", 0, 1);
    x2(u"(?i:��)|a", u"a", 0, 1);
    n(u"(?i:��)|a", u"A");
    x2(u"[������]?", u"������", 0, 1);
    x2(u"[������]*", u"������", 0, 3);
    x2(u"[^������]*", u"������", 0, 0);
    n(u"[^������]+", u"������");
    x2(u"��??", u"������", 0, 0);
    x2(u"����??��", u"������", 0, 3);
    x2(u"��*?", u"������", 0, 0);
    x2(u"����*?", u"������", 0, 1);
    x2(u"����*?��", u"��������", 0, 4);
    x2(u"��+?", u"������", 0, 1);
    x2(u"����+?", u"������", 0, 2);
    x2(u"����+?��", u"��������", 0, 4);
    x2(u"(?:�V?)??", u"�V", 0, 0);
    x2(u"(?:�V??)?", u"�V", 0, 0);
    x2(u"(?:��?)+?", u"������", 0, 1);
    x2(u"(?:��+)??", u"������", 0, 0);
    x2(u"(?:��+)??��", u"���ᑚ", 0, 4);
    x2(u"(?:����)?{2}", u"", 0, 0);
    x2(u"(?:�S��)?{2}", u"�S�ԋS�ԋS", 0, 4);
    x2(u"(?:�S��)*{0}", u"�S�ԋS�ԋS", 0, 0);
    x2(u"(?:�S��){3,}", u"�S�ԋS�ԋS�ԋS��", 0, 8);
    n(u"(?:�S��){3,}", u"�S�ԋS��");
    x2(u"(?:�S��){2,4}", u"�S�ԋS�ԋS��", 0, 6);
    x2(u"(?:�S��){2,4}", u"�S�ԋS�ԋS�ԋS�ԋS��", 0, 8);
    x2(u"(?:�S��){2,4}?", u"�S�ԋS�ԋS�ԋS�ԋS��", 0, 4);
    x2(u"(?:�S��){,}", u"�S��{,}", 0, 5);
    x2(u"(?:������)+?{2}", u"������������������", 0, 6);
    x3(u"(��)", u"��", 0, 1, 1);
    x3(u"(�ΐ�)", u"�ΐ�", 0, 2, 1);
    x2(u"((����))", u"����", 0, 2);
    x3(u"((����))", u"����", 0, 2, 1);
    x3(u"((���))", u"���", 0, 2, 2);
    x3(u"((((((((((((((((((((�ʎq))))))))))))))))))))", u"�ʎq", 0, 2, 20);
    x3(u"(����)(����)", u"��������", 0, 2, 1);
    x3(u"(����)(����)", u"��������", 2, 4, 2);
    x3(u"()(��)����(������)��������", u"��������������������", 3, 6, 3);
    x3(u"(()(��)����(������)��������)", u"��������������������", 3, 6, 4);
    x3(u".*(�t�H)���E�}(��()�V���^)�C��", u"�t�H���E�}���V���^�C��", 5, 9, 2);
    x2(u"(^��)", u"��", 0, 1);
    x3(u"(��)|(��)", u"����", 1, 2, 1);
    x3(u"(^��)|(��)", u"����", 1, 2, 2);
    x3(u"(��?)", u"������", 0, 1, 1);
    x3(u"(��*)", u"�܂܂�", 0, 3, 1);
    x3(u"(��*)", u"", 0, 0, 1);
    x3(u"(��+)", u"��������", 0, 7, 1);
    x3(u"(��+|��*)", u"�ӂӂӂւ�", 0, 3, 1);
    x3(u"(��+|��?)", u"����������", 0, 1, 1);
    x3(u"(������)?", u"������", 0, 3, 1);
    x3(u"(������)*", u"������", 0, 3, 1);
    x3(u"(������)+", u"������", 0, 3, 1);
    x3(u"(������|������)+", u"������", 0, 3, 1);
    x3(u"([�Ȃɂ�][������]|������)+", u"������", 0, 3, 1);
    x3(u"((?i:������))", u"������", 0, 3, 1);
    x3(u"((?s:��.��))", u"��\n��", 0, 3, 1);    #XXX: m -> s
    x3(u"((?=����)��)", u"����", 0, 1, 1);
    x3(u"������|(.������)", u"�񂠂���", 0, 4, 1);
    x3(u"��*(.)", u"����������", 4, 5, 1);
    x3(u"��*?(.)", u"����������", 0, 1, 1);
    x3(u"��*?(��)", u"����������", 4, 5, 1);
    x3(u"[������]��*(.)", u"������������", 5, 6, 1);
    x3(u"(\\A����)����", u"��������", 0, 2, 1);
    n(u"(\\A����)����", u"�񂢂�����");
    x3(u"(^����)����", u"��������", 0, 2, 1);
    n(u"(^����)����", u"�񂢂�����");
    x3(u"���(���$)", u"�����", 2, 4, 1);
    n(u"���(���$)", u"������");
    x2(u"(��)\\1", u"����", 0, 2);
    n(u"(��)\\1", u"����");
    x2(u"(��?)\\1", u"���", 0, 2);
    x2(u"(��??)\\1", u"���", 0, 0);
    x2(u"(��*)\\1", u"������", 0, 4);
    x3(u"(��*)\\1", u"������", 0, 2, 1);
    x2(u"��(��*)\\1", u"����������", 0, 5);
    x2(u"��(��*)\\1", u"����", 0, 1);
    x2(u"(��*)(��*)\\1\\2", u"��������������������", 0, 10);
    x2(u"(��*)(��*)\\2", u"��������������", 0, 7);
    x3(u"(��*)(��*)\\2", u"��������������", 3, 5, 2);
    x2(u"(((((((��*)��))))))��\\7", u"�ۂۂۂ؂҂ۂۂ�", 0, 8);
    x3(u"(((((((��*)��))))))��\\7", u"�ۂۂۂ؂҂ۂۂ�", 0, 3, 7);
    x2(u"(��)(��)(��)\\2\\1\\3", u"�͂ЂӂЂ͂�", 0, 6);
    x2(u"([��-��])\\1", u"����", 0, 2);
    x2(u"(\\w\\d\\s)\\1", u"��5 ��5 ", 0, 6);
    n(u"(\\w\\d\\s)\\1", u"��5 ��5");
    x2(u"(�N�H|[��-��]{3})\\1", u"�N�H�N�H", 0, 4);
    x2(u"...(�N�H|[��-��]{3})\\1", u"��a���N�H�N�H", 0, 7);
    x2(u"(�N�H|[��-��]{3})\\1", u"������������", 0, 6);
    x2(u"(^��)\\1", u"����", 0, 2);
    n(u"(^��)\\1", u"�߂ނ�");
    n(u"(��$)\\1", u"����");
    n(u"(����\\Z)\\1", u"����");
    x2(u"(��*\\Z)\\1", u"��", 1, 1);
    x2(u".(��*\\Z)\\1", u"����", 1, 2);
    x3(u"(.(�₢��)\\2)", u"z�₢��₢��", 0, 7, 1);
    x3(u"(.(..\\d.)\\2)", u"��12341234", 0, 9, 1);
    x2(u"((?i:��v��))\\1", u"��v����v��", 0, 6);
    x2(u"(?<����>��|\\(\\g<����>\\))", u"((((((��))))))", 0, 13);
    x2(u"\\A(?:\\g<��_1>|\\g<�]_2>|\\z�I��  (?<��_1>��|��\\g<�]_2>��)(?<�]_2>��|��F\\g<��_1>��F))$", u"��F����F���ݎ���F����F", 0, 13);
    x2(u"[[�Ђ�]]", u"��", 0, 1);
    x2(u"[[������]��]", u"��", 0, 1);
    n(u"[[^��]]", u"��");
    n(u"[^[��]]", u"��");
    x2(u"[^[^��]]", u"��", 0, 1);
    x2(u"[[������]&&����]", u"��", 0, 1);
    n(u"[[������]&&����]", u"��");
    n(u"[[������]&&����]", u"��");
    x2(u"[��-��&&��-��&&��-��]", u"��", 0, 1);
    n(u"[^��-��&&��-��&&��-��]", u"��");
    x2(u"[[^��&&��]&&��-��]", u"��", 0, 1);
    n(u"[[^��&&��]&&��-��]", u"��");
    x2(u"[[^��-��&&��������]&&[^��-��]]", u"��", 0, 1);
    n(u"[[^��-��&&��������]&&[^��-��]]", u"��");
    x2(u"[^[^������]&&[^������]]", u"��", 0, 1);
    x2(u"[^[^������]&&[^������]]", u"��", 0, 1);
    n(u"[^[^������]&&[^������]]", u"��");
    x2(u"[��-&&-��]", u"-", 0, 1);
    x2(u"[^[^a-z������]&&[^bcdefg������]q-w]", u"��", 0, 1);
    x2(u"[^[^a-z������]&&[^bcdefg������]g-w]", u"f", 0, 1);
    x2(u"[^[^a-z������]&&[^bcdefg������]g-w]", u"g", 0, 1);
    n(u"[^[^a-z������]&&[^bcdefg������]g-w]", u"2");
    x2(u"a<b>�o�[�W�����̃_�E�����[�h<\\/b>", u"a<b>�o�[�W�����̃_�E�����[�h</b>", 0, 20);
    x2(u".<b>�o�[�W�����̃_�E�����[�h<\\/b>", u"a<b>�o�[�W�����̃_�E�����[�h</b>", 0, 20);
    
    
    # additional test patterns
    if encoding in ("UTF-16LE", "UTF-8"):
        x2(u"\\x{3042}\\x{3044}", u"����", 0, 2)
    else:
        x2(u"\\x{82a0}\\x{82A2}", u"����", 0, 2)
    x2(u"\\p{Hiragana}\\p{Katakana}", u"���C", 0, 2)
    x2(u"(?ms)^A.B$", u"X\nA\nB\nZ", 2, 5)  # (?ms)
    n(u"(?<!(?<=a)b|c)d", u"abd")
    n(u"(?<!(?<=a)b|c)d", u"cd")
    x2(u"(?<!(?<=a)b|c)d", u"bd", 1, 2)
    x2(u"(a){2}z", u"aaz", 0, 3)
    x2(u"(?<=a).*b", u"aab", 1, 3)
    x2(u"(?<=(?<!A)B)C", u"BBC", 2, 3)
    n(u"(?<=(?<!A)B)C", u"ABC")
    n(u"(?i)(?<!aa|b)c", u"Aac")
    n(u"(?i)(?<!b|aa)c", u"Aac")
    x2(u"a\\b?a", u"aa", 0, 2)
    x2(u"[^x]*x", u"aaax", 0, 4)
    
    # character classes (tests for character class optimization)
    x2(u"[@][a]", u"@a", 0, 2);
    x2(u".*[a][b][c][d][e]", u"abcde", 0, 5);
    x2(u"(?i)[A\\x{41}]", u"a", 0, 1);
    x2(u"[abA]", u"a", 0, 1);
    x2(u"[[ab]&&[ac]]+", u"aaa", 0, 3);
    x2(u"[[����]&&[����]]+", u"������", 0, 3);
    
    # possessive quantifiers
    n(u"a?+a", u"a")          # Ver.1.xx fails
    n(u"a*+a", u"aaaa")       # Ver.1.xx fails
    n(u"a++a", u"aaaa")       # Ver.1.xx fails
    n(u"a{2,3}+a", u"aaa")    # Ver.1.xx fails
    
    # linebreak
    x2(u"\\R", u"\n", 0, 1)
    x2(u"\\R", u"\r", 0, 1)
    x2(u"\\R{3}", u"\r\r\n\n", 0, 4)
    
    # extended grapheme cluster
    x2(u"\\X{5}", u"����ab\n", 0, 5)
    if encoding in ("UTF-16LE", "UTF-8"):
        try:
            x2(u"\\X", u"\u306F\u309A\n", 0, 2)
        except UnicodeEncodeError:
            # "\u309A" can not be encoded by some encodings
            pass
    
    # keep
    x2(u"ab\\Kcd", u"abcd", 2, 4)
    x2(u"ab\\Kc(\\Kd|z)", u"abcd", 3, 4)
    x2(u"ab\\Kc(\\Kz|d)", u"abcd", 2, 4)
    x2(u"(a\\K)*", u"aaab", 3, 3)
    x3(u"(a\\K)*", u"aaab", 2, 3, 1)
#    x2(u"a\\K?a", u"aa", 0, 2)        # error: differ from perl
    x2(u"ab(?=c\Kd)", u"abcd", 2, 2)          # This behaviour is currently not well defined. (see: perlre)
    x2(u"(?<=a\\Kb|aa)cd", u"abcd", 1, 4)     # This behaviour is currently not well defined. (see: perlre)
    x2(u"(?<=ab|a\\Ka)cd", u"abcd", 2, 4)     # This behaviour is currently not well defined. (see: perlre)
    
    # named group and subroutine call
    x2(u"(?<name_2>ab)(?&name_2)", u"abab", 0, 4);
    x2(u"(?<name_2>ab)(?1)", u"abab", 0, 4);
    x2(u"(?<n>|\\((?&n)\\))+$", u"()(())", 0, 6);
    x2(u"(a|x(?-1)x)", u"xax", 0, 3);
    x2(u"(a|(x(?-2)x))", u"xax", 0, 3);
    x2(u"a|x(?0)x", u"xax", 0, 3);
    x2(u"a|x(?R)x", u"xax", 0, 3);
    x2(u"(a|x\g<0>x)", u"xax", 0, 3);
    x2(u"(a|x\g'0'x)", u"xax", 0, 3);
    x2(u"(?-i:(?+1))(?i:(a)){0}", u"A", 0, 1);
    x2(u"(?-i:\g<+1>)(?i:(a)){0}", u"A", 0, 1);
    x2(u"(?-i:\g'+1')(?i:(a)){0}", u"A", 0, 1);
    
    # character set modifiers
    x2(u"(?u)\\w+", u"��a#", 0, 2);
    x2(u"(?a)\\w+", u"��a#", 1, 2);
    x2(u"(?u)\\W+", u"��a#", 2, 3);
    x2(u"(?a)\\W+", u"��a#", 0, 1);
    
    x2(u"(?a)\\b", u"��a", 1, 1);
    x2(u"(?a)\\w\\b", u"a��", 0, 1);
    x2(u"(?a)\\B", u"a ���� ", 2, 2);
    
    x2(u"(?u)\\B", u"�� ", 2, 2);
    x2(u"(?a)\\B", u"�� ", 0, 0);
    x2(u"(?a)\\B", u"a�� ", 2, 2);
    
    x2(u"(?a)\\p{Alpha}\\P{Alpha}", u"a�B", 0, 2);
    x2(u"(?u)\\p{Alpha}\\P{Alpha}", u"a�B", 0, 2);
    x2(u"(?a)[[:word:]]+", u"a��", 0, 1);
    x2(u"(?a)[[:^word:]]+", u"a��", 1, 2);
    x2(u"(?u)[[:word:]]+", u"a��", 0, 2);
    n(u"(?u)[[:^word:]]+", u"a��");
    
    # \g{} backref
    x2(u"((?<name1>\\d)|(?<name2>\\w))(\\g{name1}|\\g{name2})", u"ff", 0, 2);
    x2(u"(?:(?<x>)|(?<x>efg))\\g{x}", u"", 0, 0);
    x2(u"(?:(?<x>abc)|(?<x>efg))\\g{x}", u"abcefgefg", 3, 9);
    n(u"(?:(?<x>abc)|(?<x>efg))\\g{x}", u"abcefg");
    x2(u"((.*)a\\g{2}f)", u"bacbabf", 3, 7);
    x2(u"(.*)a\\g{1}f", u"baczzzzzz\nbazz\nzzzzbabf", 19, 23);
    x2(u"((.*)a\\g{-1}f)", u"bacbabf", 3, 7);
    x2(u"(.*)a\\g{-1}f", u"baczzzzzz\nbazz\nzzzzbabf", 19, 23);
    x2(u"(��*)(��*)\\g{-2}\\g{-1}", u"��������������������", 0, 10);
    
    # Python/PCRE compatible named group
    x2(u"(?P<name_2>ab)(?P>name_2)", u"abab", 0, 4);
    x2(u"(?P<n>|\\((?P>n)\\))+$", u"()(())", 0, 6);
    x2(u"((?P<name1>\\d)|(?P<name2>\\w))((?P=name1)|(?P=name2))", u"ff", 0, 2);
    
    # Fullwidth Alphabet
    n(u"����������������������������������������������������", u"�`�a�b�c�d�e�f�g�h�i�j�k�l�m�n�o�p�q�r�s�t�u�v�w�x�y");
    x2(u"(?i)����������������������������������������������������", u"����������������������������������������������������", 0, 26);
    x2(u"(?i)����������������������������������������������������", u"�`�a�b�c�d�e�f�g�h�i�j�k�l�m�n�o�p�q�r�s�t�u�v�w�x�y", 0, 26);
    x2(u"(?i)�`�a�b�c�d�e�f�g�h�i�j�k�l�m�n�o�p�q�r�s�t�u�v�w�x�y", u"����������������������������������������������������", 0, 26);
    x2(u"(?i)�`�a�b�c�d�e�f�g�h�i�j�k�l�m�n�o�p�q�r�s�t�u�v�w�x�y", u"�`�a�b�c�d�e�f�g�h�i�j�k�l�m�n�o�p�q�r�s�t�u�v�w�x�y", 0, 26);
    
    # Greek
    n(u"�������ÃăŃƃǃȃɃʃ˃̃̓΃σЃу҃ӃԃՃ�", u"������������������������������������������������");
    x2(u"(?i)�������ÃăŃƃǃȃɃʃ˃̃̓΃σЃу҃ӃԃՃ�", u"�������ÃăŃƃǃȃɃʃ˃̃̓΃σЃу҃ӃԃՃ�", 0, 24);
    x2(u"(?i)�������ÃăŃƃǃȃɃʃ˃̃̓΃σЃу҃ӃԃՃ�", u"������������������������������������������������", 0, 24);
    x2(u"(?i)������������������������������������������������", u"�������ÃăŃƃǃȃɃʃ˃̃̓΃σЃу҃ӃԃՃ�", 0, 24);
    x2(u"(?i)������������������������������������������������", u"������������������������������������������������", 0, 24);
    
    # Cyrillic
    n(u"�p�q�r�s�t�u�v�w�x�y�z�{�|�}�~������������������������������������", u"�@�A�B�C�D�E�F�G�H�I�J�K�L�M�N�O�P�Q�R�S�T�U�V�W�X�Y�Z�[�\�]�^�_�`");
    x2(u"(?i)�p�q�r�s�t�u�v�w�x�y�z�{�|�}�~������������������������������������", u"�p�q�r�s�t�u�v�w�x�y�z�{�|�}�~������������������������������������", 0, 33);
    x2(u"(?i)�p�q�r�s�t�u�v�w�x�y�z�{�|�}�~������������������������������������", u"�@�A�B�C�D�E�F�G�H�I�J�K�L�M�N�O�P�Q�R�S�T�U�V�W�X�Y�Z�[�\�]�^�_�`", 0, 33);
    x2(u"(?i)�@�A�B�C�D�E�F�G�H�I�J�K�L�M�N�O�P�Q�R�S�T�U�V�W�X�Y�Z�[�\�]�^�_�`", u"�p�q�r�s�t�u�v�w�x�y�z�{�|�}�~������������������������������������", 0, 33);
    x2(u"(?i)�@�A�B�C�D�E�F�G�H�I�J�K�L�M�N�O�P�Q�R�S�T�U�V�W�X�Y�Z�[�\�]�^�_�`", u"�@�A�B�C�D�E�F�G�H�I�J�K�L�M�N�O�P�Q�R�S�T�U�V�W�X�Y�Z�[�\�]�^�_�`", 0, 33);
    
    # multiple name definition
    x2(u"(?<a>a)(?<a>b)\\k<a>", u"aba", 0, 3)
    x2(u"(?<a>a)(?<a>b)(?&a)", u"aba", 0, 3)
    x2(u"(?<a>(a|.)(?<a>b))(?&a)", u"abcb", 0, 4)
    
    # branch reset
#    x3(u"(?|(c)|(?:(b)|(a)))", u"a", 0, 1, 2)
#    x3(u"(?|(c)|(?|(b)|(a)))", u"a", 0, 1, 1)
    
    # conditional expression
    x2(u"(?:(a)|(b))(?(1)cd)e", u"acde", 0, 4)
    n(u"(?:(a)|(b))(?(1)cd)e", u"ae")
    x2(u"(?:(a)|(b))(?(2)cd)e", u"ae", 0, 2)
    n(u"(?:(a)|(b))(?(2)cd)e", u"acde")
    x2(u"(?:(a)|(b))(?(1)c|d)", u"ac", 0, 2)
    x2(u"(?:(a)|(b))(?(1)c|d)", u"bd", 0, 2)
    n(u"(?:(a)|(b))(?(1)c|d)", u"ad")
    n(u"(?:(a)|(b))(?(1)c|d)", u"bc")
    x2(u"(?:(a)|(b))(?:(?(1)cd)e|fg)", u"acde", 0, 4)
    x2(u"(?:(a)|(b))(?:(?(1)cd|x)e|fg)", u"bxe", 0, 3)
    n(u"(?:(a)|(b))(?:(?(2)cd|x)e|fg)", u"bxe")
    x2(u"(?:(?<x>a)|(?<y>b))(?:(?(<x>)cd|x)e|fg)", u"bxe", 0, 3)
    n(u"(?:(?<x>a)|(?<y>b))(?:(?(<y>)cd|x)e|fg)", u"bxe")
    x2(u"((?<=a))?(?(1)b|c)", u"abc", 1, 2)
    x2(u"((?<=a))?(?(1)b|c)", u"bc", 1, 2)
    
    # Implicit-anchor optimization
    x2(u"(?s:.*abc)", u"dddabdd\nddabc", 0, 13)   # optimized /(?s:.*abc)/ ==> /\A(?s:.*abc)/
    x2(u"(?s:.+abc)", u"dddabdd\nddabc", 0, 13)   # optimized
    x2(u"(.*)X\\1", u"1234X2345", 1, 8)           # not optimized
    
    
    print("\nRESULT   SUCC: %d,  FAIL: %d,  ERROR: %d\n" % (
           nsucc, nfail, nerror))


if __name__ == '__main__':
    main()


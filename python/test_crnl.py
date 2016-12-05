#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function, unicode_literals
from bregonig import *
from test_common import *
import sys

def x(pattern, target, start_offset, s_from, s_to):
    xx(pattern, target, s_from, s_to, 0, False, start_offset=start_offset, opt="R")

def n(pattern, target, start_offset):
    xx(pattern, target, 0, 0, 0, True, start_offset=start_offset, opt="R")


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
           get_nsucc(), get_nfail(), get_nerror()))

    if (get_nfail() == 0 and get_nerror() == 0):
        exit(0)
    else:
        exit(-1)

if __name__ == '__main__':
    main()


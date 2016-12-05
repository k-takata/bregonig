# -*- coding: utf-8 -*-

from __future__ import print_function, unicode_literals
import ctypes
from bregonig import *
import sys
import io
import locale

__all__ = ["get_nerror", "get_nsucc", "get_nfail",
        "inc_nerror", "inc_nsucc", "inc_nfail",
        "strptr", "cc_to_cb", "print_result",
        "xx", "x2", "x3", "n",
        "is_unicode_encoding", "is_wide_encoding",
        "set_encoding", "get_encoding", "set_output_encoding", "init"];

nerror = 0
nsucc = 0
nfail = 0

# default encoding
encoding = "CP932"


def get_nerror():
    return nerror
def get_nsucc():
    return nsucc
def get_nfail():
    return nfail

def inc_nerror():
    global nerror
    nerror += 1
def inc_nsucc():
    global nsucc
    nsucc += 1
def inc_nfail():
    global nfail
    nfail += 1


class strptr:
    """a helper class to get a pointer to a string"""
    def __init__(self, s):
        if not isinstance(s, bytes):
            raise TypeError
        self._str = s
        try:
            # CPython 2.x/3.x
            self._ptr = ctypes.cast(self._str, ctypes.c_void_p)
        except TypeError:
            # PyPy 1.x
            self._ptr = ctypes.c_void_p(self._str)

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
    if cc == -1:
        return -1
    s = s.encode('UTF-32LE')
    clen = cc * 4
    if clen > len(s):
        raise IndexError
    return len(s[:clen].decode('UTF-32LE').encode(enc))

def print_result(result, pattern, file=None):
    if not file:
        file = sys.stdout
    print(result + ": ", end='', file=file)
    try:
        print(pattern, file=file)
    except UnicodeEncodeError as e:
        print('(' + str(e) + ')')

def xx(pattern, target, s_from, s_to, mem, not_match, opt="", err=False,
        start_offset=0):
    global nerror
    global nsucc
    global nfail
    
    rxp = ctypes.POINTER(BREGEXP)()
    msg = create_tchar_buffer(BREGEXP_MAX_ERROR_MESSAGE_LEN)
    
    pattern2 = pattern
    if not isinstance(pattern, bytes):
        pattern2 = pattern.encode(encoding)
    pattern3 = "/".encode(encoding) + pattern2 + ("/k" + opt).encode(encoding)
    
    target2 = target
    if not isinstance(target, bytes):
        s_from = cc_to_cb(target, encoding, s_from)
        s_to = cc_to_cb(target, encoding, s_to)
        start_offset = cc_to_cb(target, encoding, start_offset)
        target2 = target.encode(encoding)
    tp = strptr(target2)
    
    # cut very long outputs
    limit = 100
    if len(target) > limit:
        target = target[:limit] + "..."
    if len(pattern) > limit:
        pattern = pattern[:limit] + "..."

    if encoding == "UTF-8":
        option = "8"
    else:
        option = "k"
    option = (option + opt).encode(encoding)
    
    if encoding == "UTF-16LE":
        pattern2 = ctypes.c_wchar_p(pattern2.decode(encoding))
        pattern3 = ctypes.c_wchar_p(pattern3.decode(encoding))
        option = ctypes.c_wchar_p(option.decode(encoding))
    
    try:
        r = BoMatch(pattern2, option, tp.getptr(), tp.getptr(start_offset), tp.getptr(-1),
                False, ctypes.byref(rxp), msg)
    except RuntimeError:
        r = BMatch(pattern3, tp.getptr(), tp.getptr(-1), ctypes.byref(rxp), msg)
    
    if r < 0:
        # Error
        if err:
            nsucc += 1
            print_result("OK(E)", "%s (/%s/ '%s')" % (msg.value, pattern, target))
        else:
            nerror += 1
            print_result("ERROR", "%s (/%s/ '%s')" % (msg.value, pattern, target),
                    file=sys.stderr)
        return

    if err:
        nfail += 1
        print_result("FAIL(E)", "/%s/ '%s'" % (pattern, target))

    elif r == 0:
        # Not matched
        if not_match:
            nsucc += 1
            print_result("OK(N)", "/%s/ '%s'" % (pattern, target))
        else:
            nfail += 1
            print_result("FAIL", "/%s/ '%s'" % (pattern, target))
    else:
        # Matched
        if not_match:
            nfail += 1
            print_result("FAIL(N)", "/%s/ '%s'" % (pattern, target))
        else:
            start = rxp.contents.startp[mem] - tp.getptr()
            end = rxp.contents.endp[mem] - tp.getptr()
            if (start == s_from) and (end == s_to):
                nsucc += 1
                print_result("OK", "/%s/ '%s'" % (pattern, target))
            else:
                nfail += 1
                print_result("FAIL", "/%s/ '%s' %d-%d : %d-%d" % (pattern, target,
                        s_from, s_to, start, end))
    
    if (rxp):
        BRegfree(rxp)

def x2(pattern, target, s_from, s_to, **kwargs):
    xx(pattern, target, s_from, s_to, 0, False, **kwargs)

def x3(pattern, target, s_from, s_to, mem, **kwargs):
    xx(pattern, target, s_from, s_to, mem, False, **kwargs)

def n(pattern, target, **kwargs):
    xx(pattern, target, 0, 0, 0, True, **kwargs)


def is_unicode_encoding(enc):
    return enc in ("UTF-16LE", "UTF-8")

def is_wide_encoding(enc):
    encs = {"CP932": False,
            "SJIS": False,
            "UTF-8": False,
            "UTF-16LE": True}
    return encs[enc]


def set_encoding(enc):
    """Set the encoding used for testing.

    arguments:
      enc -- encoding name
    """
    global encoding

    if enc == None:
        return False
    encoding = enc

    return is_wide_encoding(enc)


def get_encoding():
    return encoding


def set_output_encoding(enc=None):
    """Set the encoding used for showing the results.

    arguments:
      enc -- Encoding name.
             If omitted, locale.getpreferredencoding() is used.
    """
    if enc is None:
        enc = locale.getpreferredencoding()

    def get_text_writer(fo, **kwargs):
        kw = dict(kwargs)
        kw.setdefault('errors', 'backslashreplace') # use \uXXXX style
        kw.setdefault('closefd', False)

        if sys.version_info[0] < 3:
            # Work around for Python 2.x
            # New line conversion isn't needed here. Done in somewhere else.
            writer = io.open(fo.fileno(), mode='w', newline='', **kw)
            write = writer.write    # save the original write() function
            enc = locale.getpreferredencoding()
            def convwrite(s):
                if isinstance(s, bytes):
                    write(s.decode(enc))    # convert to unistr
                else:
                    write(s)
                try:
                    writer.flush()  # needed on Windows
                except IOError:
                    pass
            writer.write = convwrite
        else:
            writer = io.open(fo.fileno(), mode='w', **kw)
        return writer

    sys.stdout = get_text_writer(sys.stdout, encoding=enc)
    sys.stderr = get_text_writer(sys.stderr, encoding=enc)


def init(enc, outenc=None):
    """Setup test target encoding, output encoding and warning function.

    arguments:
      enc    -- Encoding used for testing.
      outenc -- Encoding used for showing messages.
    """
    ret = set_encoding(enc)
    set_output_encoding(outenc)
    return ret

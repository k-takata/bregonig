#
# Makefile for bregonig.dll
#
#  Copyright (C) 2006-2012  K.Takata
#

#VER1 = 1
USE_LTCG = 1
#USE_MSVCRT = 1
#USE_ONIG_DLL = 1

!ifndef TARGET_CPU
!if ("$(CPU)"=="AMD64" && !DEFINED(386)) || DEFINED(AMD64) || "$(Platform)"=="x64"
TARGET_CPU = x64
!elseif DEFINED(IA64)
TARGET_CPU = ia64
!else
TARGET_CPU = x86
!endif
!endif

BASEADDR = 0x60500000

ONIG_DIR = ../onigmo-5.13.2
!ifdef USE_ONIG_DLL
ONIG_LIB = $(ONIG_DIR)/onig.lib
!else
ONIG_LIB = $(ONIG_DIR)/onig_s.lib
!endif

CPPFLAGS = /O2 /W3 /EHsc /LD /nologo /I$(ONIG_DIR)
!ifdef VER1
CPPFLAGS = $(CPPFLAGS) /DUSE_VTAB /DPERL_5_8_COMPAT /DNAMEGROUP_RIGHTMOST
!endif
LD = link
LDFLAGS = /DLL /nologo /MAP /BASE:$(BASEADDR) /merge:.rdata=.text

!ifdef USE_MSVCRT
CPPFLAGS = $(CPPFLAGS) /MD
!else
CPPFLAGS = $(CPPFLAGS) /MT
!endif

!ifndef USE_ONIG_DLL
CPPFLAGS = $(CPPFLAGS) /DONIG_EXTERN=extern
!endif

# Get the version of cl.exe.
#  1. Write the version to a work file (mscver$(_NMAKE_VER).~).
!if [(echo _MSC_VER>mscver$(_NMAKE_VER).c) && ($(CC) /EP mscver$(_NMAKE_VER).c 2>nul > mscver$(_NMAKE_VER).~)]
!endif
#  2. Command string to get the version.
_MSC_VER = [for /f %i in (mscver$(_NMAKE_VER).~) do @exit %i]

!if DEFINED(USE_LTCG) && $(USE_LTCG)
# Use LTCG (Link Time Code Generation).
# Check if cl.exe is newer than VC++ 7.0 (_MSC_VER >= 1300).
!if $(_MSC_VER) >= 1300
CPPFLAGS = $(CPPFLAGS) /GL
LDFLAGS = $(LDFLAGS) /LTCG
!endif
!endif

!if $(_MSC_VER) < 1500
LDFLAGS = $(LDFLAGS) /opt:nowin98
!endif

!ifdef DEBUG
CPPFLAGS = $(CPPFLAGS) /D_DEBUG
RFLAGS = $(RFLAGS) /D_DEBUG
!endif

OBJDIR = obj
!ifdef DEBUG
OBJDIR = $(OBJDIR)d
!endif
OBJDIR = $(OBJDIR)$(TARGET_CPU)

OBJS = $(OBJDIR)\subst.obj $(OBJDIR)\bsplit.obj $(OBJDIR)\btrans.obj $(OBJDIR)\sv.obj
WOBJS = $(OBJDIR)\substw.obj $(OBJDIR)\bsplitw.obj $(OBJDIR)\btransw.obj $(OBJDIR)\svw.obj
!ifdef VER1
BROBJS = $(OBJDIR)\bregonig.obj $(OBJDIR)\bregonig.res $(OBJS)
!else
BROBJS = $(OBJDIR)\bregonig.obj $(OBJDIR)\bregonigw.obj $(OBJDIR)\bregonig.res $(OBJS) $(WOBJS)
!endif
K2OBJS = $(OBJDIR)\k2regexp.obj $(OBJDIR)\k2regexp.res $(OBJS)


all: $(OBJDIR)\bregonig.dll $(OBJDIR)\k2regexp.dll


$(OBJDIR)\bregonig.dll: $(OBJDIR) $(BROBJS) $(ONIG_LIB)
	$(LD) $(BROBJS) $(ONIG_LIB) /out:$@ $(LDFLAGS)

$(OBJDIR)\k2regexp.dll: $(OBJDIR) $(K2OBJS) $(ONIG_LIB)
	$(LD) $(K2OBJS) $(ONIG_LIB) /out:$@ $(LDFLAGS)


$(OBJDIR):
	if not exist $(OBJDIR)\nul  mkdir $(OBJDIR)


.cpp{$(OBJDIR)\}.obj::
	$(CPP) $(CPPFLAGS) /Fo$(OBJDIR)\ /c $<

.rc{$(OBJDIR)\}.res:
	$(RC) $(RFLAGS) /Fo$@ /r $<

$(OBJDIR)\bregonig.obj: bregonig.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h version.h $(ONIG_DIR)/oniguruma.h

$(OBJDIR)\bregonigw.obj: bregonig.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h version.h $(ONIG_DIR)/oniguruma.h
	$(CPP) $(CPPFLAGS) /c /DUNICODE /D_UNICODE /Fo$@ bregonig.cpp

$(OBJDIR)\bregonig.res: bregonig.rc version.h

$(OBJDIR)\k2regexp.obj: bregonig.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h version.h $(ONIG_DIR)/oniguruma.h
	$(CPP) $(CPPFLAGS) /c /D_K2REGEXP_ /Fo$@ bregonig.cpp

#$(OBJDIR)\k2regexpw.obj: bregonig.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h version.h $(ONIG_DIR)/oniguruma.h
#	$(CPP) $(CPPFLAGS) /c /D_K2REGEXP_ /DUNICODE /D_UNICODE /Fo$@ bregonig.cpp

$(OBJDIR)\k2regexp.res: bregonig.rc version.h
	$(RC) $(RFLAGS) /D_K2REGEXP_ /Fo$@ /r bregonig.rc


$(OBJDIR)\subst.obj: subst.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h $(ONIG_DIR)/oniguruma.h

$(OBJDIR)\substw.obj: subst.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h $(ONIG_DIR)/oniguruma.h
	$(CPP) $(CPPFLAGS) /c /DUNICODE /D_UNICODE /Fo$@ subst.cpp

$(OBJDIR)\bsplit.obj: bsplit.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h $(ONIG_DIR)/oniguruma.h

$(OBJDIR)\bsplitw.obj: bsplit.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h $(ONIG_DIR)/oniguruma.h
	$(CPP) $(CPPFLAGS) /c /DUNICODE /D_UNICODE /Fo$@ bsplit.cpp

$(OBJDIR)\btrans.obj: btrans.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h sv.h $(ONIG_DIR)/oniguruma.h

$(OBJDIR)\btransw.obj: btrans.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h sv.h $(ONIG_DIR)/oniguruma.h
	$(CPP) $(CPPFLAGS) /c /DUNICODE /D_UNICODE /Fo$@ btrans.cpp

$(OBJDIR)\sv.obj: sv.cpp sv.h

$(OBJDIR)\svw.obj: sv.cpp sv.h
	$(CPP) $(CPPFLAGS) /c /DUNICODE /D_UNICODE /Fo$@ sv.cpp


clean:
	del $(BROBJS) $(OBJDIR)\bregonig.lib $(OBJDIR)\bregonig.dll $(OBJDIR)\bregonig.exp $(OBJDIR)\bregonig.map \
		$(OBJDIR)\k2regexp.obj $(OBJDIR)\k2regexp.res $(OBJDIR)\k2regexp.lib $(OBJDIR)\k2regexp.dll $(OBJDIR)\k2regexp.exp $(OBJDIR)\k2regexp.map


# clean up
!if [del mscver$(_NMAKE_VER).~ mscver$(_NMAKE_VER).c]
!endif

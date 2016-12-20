#
# Makefile for bregonig.dll
#
#  Copyright (C) 2006-2014  K.Takata
#

#VER1 = 1
USE_LTCG = 1
#USE_MSVCRT = 1
#USE_ONIG_DLL = 1
USE_ONIGMO_6 = 1

!ifndef TARGET_CPU
!if ("$(CPU)"=="AMD64" && !DEFINED(386)) || DEFINED(AMD64) || "$(PLATFORM)"=="x64" || "$(PLATFORM)"=="X64"
TARGET_CPU = x64
!elseif DEFINED(IA64)
TARGET_CPU = ia64
!else
TARGET_CPU = x86
!endif
!endif

BASEADDR = 0x60500000

!ifdef DEBUG
DBGFLG = d
!else
DBGFLG =
!endif

!ifdef USE_ONIGMO_6
# Onigmo 6.0 or later
ONIG_DIR = ../onigmo-6.0.0
!ifdef USE_ONIG_DLL
ONIG_LIB = $(ONIG_DIR)/build_$(TARGET_CPU)$(DBGFLG)/onigmo.lib
!else
ONIG_LIB = $(ONIG_DIR)/build_$(TARGET_CPU)$(DBGFLG)/onigmo_s.lib
!endif
ONIG_H = $(ONIG_DIR)/onigmo.h

!else
# Onigmo 5.15 or earlier
ONIG_DIR = ../onig-5.15.0
!ifdef USE_ONIG_DLL
ONIG_LIB = $(ONIG_DIR)/onig.lib
!else
ONIG_LIB = $(ONIG_DIR)/onig_s.lib
!endif
ONIG_H = $(ONIG_DIR)/oniguruma.h

!endif

CPPFLAGS = /O2 /W3 /EHsc /LD /nologo /I$(ONIG_DIR)
!ifdef VER1
CPPFLAGS = $(CPPFLAGS) /DUSE_VTAB /DPERL_5_8_COMPAT /DNAMEGROUP_RIGHTMOST
!endif
!ifdef USE_ONIGMO_6
CPPFLAGS = $(CPPFLAGS) /DUSE_ONIGMO_6
!endif
LD = link
LDFLAGS = /DLL /nologo /MAP /BASE:$(BASEADDR) /merge:.rdata=.text

!ifdef USE_MSVCRT
CPPFLAGS = $(CPPFLAGS) /MD
!else
!ifdef DEBUG
CPPFLAGS = $(CPPFLAGS) /MTd
!else
CPPFLAGS = $(CPPFLAGS) /MT
!endif
!endif

!ifndef USE_ONIG_DLL
CPPFLAGS = $(CPPFLAGS) /DONIG_EXTERN=extern
!endif

# Get the version of cl.exe.
#  1. Write the version to a work file (mscver$(_NMAKE_VER).~).
!if ![(echo _MSC_VER>mscver$(_NMAKE_VER).c) && \
	(for /f %I in ('"$(CC) /EP mscver$(_NMAKE_VER).c 2>nul"') do @echo _MSC_VER=%I> mscver$(_NMAKE_VER).~)]
#  2. Include it.
!include mscver$(_NMAKE_VER).~
#  3. Clean up.
!if [del mscver$(_NMAKE_VER).~ mscver$(_NMAKE_VER).c]
!endif
!endif

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
CPPFLAGS = $(CPPFLAGS) /D_DEBUG /Zi
RFLAGS = $(RFLAGS) /D_DEBUG
!endif

OBJDIR = obj$(TARGET_CPU)$(DBGFLG)
WOBJDIR = $(OBJDIR)\unicode

OBJS = $(OBJDIR)\subst.obj $(OBJDIR)\bsplit.obj $(OBJDIR)\btrans.obj $(OBJDIR)\sv.obj
WOBJS = $(WOBJDIR)\subst.obj $(WOBJDIR)\bsplit.obj $(WOBJDIR)\btrans.obj $(WOBJDIR)\sv.obj
!ifdef VER1
BROBJS = $(OBJDIR)\bregonig.obj $(OBJDIR)\bregonig.res $(OBJS)
!else
BROBJS = $(OBJDIR)\bregonig.obj $(WOBJDIR)\bregonig.obj $(OBJDIR)\bregonig.res $(OBJS) $(WOBJS)
!endif
K2OBJS = $(OBJDIR)\k2regexp.obj $(OBJDIR)\k2regexp.res $(OBJS)


all: $(OBJDIR)\bregonig.dll $(OBJDIR)\k2regexp.dll


$(OBJDIR)\bregonig.dll: $(WOBJDIR) $(BROBJS) $(ONIG_LIB)
	$(LD) $(BROBJS) $(ONIG_LIB) /out:$@ $(LDFLAGS)

$(OBJDIR)\k2regexp.dll: $(WOBJDIR) $(K2OBJS) $(ONIG_LIB)
	$(LD) $(K2OBJS) $(ONIG_LIB) /out:$@ $(LDFLAGS)


$(WOBJDIR):
	if not exist $(OBJDIR)\nul  mkdir $(OBJDIR)
	if not exist $(WOBJDIR)\nul mkdir $(WOBJDIR)


.cpp{$(OBJDIR)\}.obj::
	$(CPP) $(CPPFLAGS) /Fo$(OBJDIR)\ /c $<
.cpp{$(WOBJDIR)\}.obj::
	$(CPP) $(CPPFLAGS) /DUNICODE /D_UNICODE /Fo$(WOBJDIR)\ /c $<

.rc{$(OBJDIR)\}.res:
	$(RC) $(RFLAGS) /Fo$@ /r $<

$(OBJDIR)\bregonig.obj: bregonig.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h version.h $(ONIG_H)

$(WOBJDIR)\bregonig.obj: bregonig.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h version.h $(ONIG_H)

$(OBJDIR)\bregonig.res: bregonig.rc version.h

$(OBJDIR)\k2regexp.obj: bregonig.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h version.h $(ONIG_H)
	$(CPP) $(CPPFLAGS) /c /D_K2REGEXP_ /Fo$@ bregonig.cpp

#$(WOBJDIR)\k2regexp.obj: bregonig.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h version.h $(ONIG_H)
#	$(CPP) $(CPPFLAGS) /c /D_K2REGEXP_ /DUNICODE /D_UNICODE /Fo$@ bregonig.cpp

$(OBJDIR)\k2regexp.res: bregonig.rc version.h
	$(RC) $(RFLAGS) /D_K2REGEXP_ /Fo$@ /r bregonig.rc


$(OBJDIR)\subst.obj: subst.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h $(ONIG_H)

$(WOBJDIR)\subst.obj: subst.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h $(ONIG_H)

$(OBJDIR)\bsplit.obj: bsplit.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h $(ONIG_H)

$(WOBJDIR)\bsplit.obj: bsplit.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h $(ONIG_H)

$(OBJDIR)\btrans.obj: btrans.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h sv.h $(ONIG_H)

$(WOBJDIR)\btrans.obj: btrans.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h sv.h $(ONIG_H)

$(OBJDIR)\sv.obj: sv.cpp sv.h

$(WOBJDIR)\sv.obj: sv.cpp sv.h


clean:
	del $(BROBJS) $(OBJDIR)\bregonig.lib $(OBJDIR)\bregonig.dll $(OBJDIR)\bregonig.exp $(OBJDIR)\bregonig.map \
		$(OBJDIR)\k2regexp.obj $(OBJDIR)\k2regexp.res $(OBJDIR)\k2regexp.lib $(OBJDIR)\k2regexp.dll $(OBJDIR)\k2regexp.exp $(OBJDIR)\k2regexp.map

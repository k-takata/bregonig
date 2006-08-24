#
# Makefile for bregonig.dll
#
#  Copyright (C) 2006  K.Takata
#

ONIG_DIR = ../onig-4.4.0-mt
ONIG_LIB = $(ONIG_DIR)/onig_s.lib

#CPPFLAGS = /O2 /W3 /GX /LD /nologo /I$(ONIG_DIR) /DONIG_EXTERN=extern
CPPFLAGS = /O2 /W3 /EHsc /LD /nologo /I$(ONIG_DIR) /DONIG_EXTERN=extern /MT
LD = link
LDFLAGS = /DLL /nologo


!ifdef DEBUG
CPPFLAGS = $(CPPFLAGS) /D_DEBUG
!endif


all : bregonig.dll k2regexp.dll


bregonig.dll : bregonig.obj subst.obj $(ONIG_LIB)
	$(LD) $** /out:$@ $(LDFLAGS)

k2regexp.dll : k2regexp.obj subst.obj $(ONIG_LIB)
	$(LD) $** /out:$@ $(LDFLAGS)


bregonig.obj : bregonig.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h global.h $(ONIG_DIR)/oniguruma.h

k2regexp.obj : bregonig.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h global.h $(ONIG_DIR)/oniguruma.h
	$(CC) $(CPPFLAGS) /c /D_K2REGEXP_ /Fo$@ bregonig.cpp


subst.obj : subst.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h global.h $(ONIG_DIR)/oniguruma.h


clean :
	del bregonig.obj subst.obj bregonig.lib bregonig.dll bregonig.exp
	del k2regexp.obj k2regexp.lib k2regexp.dll k2regexp.exp


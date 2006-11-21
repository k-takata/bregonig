#
# Makefile for bregonig.dll
#
#  Copyright (C) 2006  K.Takata
#

ONIG_DIR = ../onig-5.3.0-mt
ONIG_LIB = $(ONIG_DIR)/onig_s.lib

#CPPFLAGS = /O2 /W3 /GX /LD /nologo /I$(ONIG_DIR) /DONIG_EXTERN=extern /MT
#CPPFLAGS = /O2 /W3 /EHsc /LD /nologo /I$(ONIG_DIR) /DONIG_EXTERN=extern /MT
CPPFLAGS = /O2 /W3 /EHac /LD /nologo /I$(ONIG_DIR) /DONIG_EXTERN=extern /MT
LD = link
LDFLAGS = /DLL /nologo /MAP /opt:nowin98


!ifdef DEBUG
CPPFLAGS = $(CPPFLAGS) /D_DEBUG
RFLAGS = $(RFLAGS) /D_DEBUG
!endif

OBJS = subst.obj bsplit.obj btrans.obj sv.obj
BROBJS = bregonig.obj bregonig.res $(OBJS)
K2OBJS = k2regexp.obj k2regexp.res $(OBJS)


all : bregonig.dll k2regexp.dll


bregonig.dll : $(BROBJS) $(ONIG_LIB)
	$(LD) $** /out:$@ $(LDFLAGS)

k2regexp.dll : $(K2OBJS) $(ONIG_LIB)
	$(LD) $** /out:$@ $(LDFLAGS)


bregonig.obj : bregonig.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h version.h $(ONIG_DIR)/oniguruma.h

bregonig.res : bregonig.rc

k2regexp.obj : bregonig.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h version.h $(ONIG_DIR)/oniguruma.h
	$(CC) $(CPPFLAGS) /c /D_K2REGEXP_ /Fo$@ bregonig.cpp

k2regexp.res : bregonig.rc
	$(RC) $(RFLAGS) /D_K2REGEXP_ /Fo$@ /r $**


subst.obj : subst.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h $(ONIG_DIR)/oniguruma.h

bsplit.obj : bsplit.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h $(ONIG_DIR)/oniguruma.h

btrans.obj : btrans.cpp bregexp.h bregonig.h mem_vc6.h dbgtrace.h sv.h $(ONIG_DIR)/oniguruma.h

sv.obj : sv.cpp sv.h


clean :
	del $(BROBJS) bregonig.lib bregonig.dll bregonig.exp bregonig.map
	del k2regexp.obj k2regexp.res k2regexp.lib k2regexp.dll k2regexp.exp k2regexp.map


#**********************************************************************#
# C/S-Antragssytem standard makefile                                   #
#                                                                      #
#   fÅr einzelne Projekte muss angegeben werden:                       #
#      P=<projektname>                                                 #
#      SRCDIR=<vollstÑndiger pfad des produktiven Quell-Verzeichnisses #
#                                                                      #
#   exportierte Header-Dateien                                         #
#      HEADERS=<header1> <header2> ...                                 #
#                                                                      #
#   zusÑtzlich benîtigte Module: (neben <projektname>.obj)             #
#      OBJ=<obj1> <obj2> ...                                           #
#                                                                      #
#   benîtigte Libraries:                                               #
#      ADDLIB=<libfile1>+<libfile2>+...                                #
#                                                                      #
#   optional:                                                          #
#      DLL=YES, PM=PM, DEB=YES                                         #
#                                                                      #
#**********************************************************************#
SRCDIR=E:\CPROG\MemStat
P=memstat
OBJ=loadstr.obj
DLL=NO
PM=PM
DEB=YES
UTIL=NO
NODEF=YES
NOPORTLIB=YES
ADDLIB=USER32.LIB GDI32.LIB ADVAPI32.LIB COMDLG32.LIB
DESTBIN=C:\UTIL

ADDINC=.

!IF "$(NOUNICODE)" != "YES"
ADDEF=-DUNICODE
!ENDIF

!IF "$(DEB)" == "YES"
# IXTRA=YES
# ADDRC=$(ADDRC) /D_DEBUG=1
!ENDIF

#**********************************************************************#
#    !!!!! Die folgenden Zeilen dÅrfen nicht verÑndert werden !!!!!    #
#**********************************************************************#
!IF "$(ENV_SYSTEMROOT)" == ""
ENV_SYSTEMROOT=$(ENV_ROOT)
!ENDIF
!IF "$(ENV_SYSTEMROOT)" == ""
ENV_OS=WINNT
ENV_SYSTEMROOT=P:\ENV\env200\std
PATH=$(ENV_SYSTEMROOT)\ALPHA\$(ENV_OS)\BIN;$(PATH)
# LIB=$(ENV_SYSTEMROOT)\ALPHA\$(ENV_OS)\LIB;$(LIB)
!INCLUDE "$(ENV_SYSTEMROOT)\ALPHA\ALL\INC\MAN\aragstd.mak"
!ELSE
!INCLUDE <ARAGSTD.MAK>
!ENDIF

install: make.nt\memstat.exe

test:
	ewhich ADVAPI32.LIB
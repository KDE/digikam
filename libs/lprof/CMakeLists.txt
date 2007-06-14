## Gilles Caulier 12/01/06: lprof implementation is in C not C++ and do not 
## support 'nofinal' compilation option.
KDE4_NO_ENABLE_FINAL(lprof)

ADD_DEFINITIONS("-w -fomit-frame-pointer")

SET(lprof_STAT_SRCS
    cmshull.cpp 
    cmslm.cpp 
    cmslnr.cpp 
    cmsmatn.cpp 
    cmsmkmsh.cpp 
    cmsmntr.cpp 
    cmsoutl.cpp 
    cmspcoll.cpp
    cmsprf.cpp 
    cmsreg.cpp 
    cmsscn.cpp 
    cmssheet.cpp
   )

KDE4_ADD_LIBRARY(lprof STATIC ${lprof_STAT_SRCS})


#original Makefile.am contents follow:

## Gilles Caulier 12/01/06: lprof implementation is in C not C++ and do not 
## support 'nofinal' compilation option.
#KDE_OPTIONS = nofinal
#
#noinst_LTLIBRARIES = liblprof.la
#
#noinst_HEADERS = lcmsprf.h
#
#liblprof_la_CXXFLAGS = -w -fomit-frame-pointer 
#
#liblprof_la_SOURCES = cmshull.cpp cmslm.cpp cmslnr.cpp cmsmatn.cpp \
#	                  cmsmkmsh.cpp cmsmntr.cpp cmsoutl.cpp cmspcoll.cpp \
#	                  cmsprf.cpp cmsreg.cpp cmsscn.cpp cmssheet.cpp

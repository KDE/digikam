## Gilles Caulier 12/01/06: lprof implementation is in C not C++ and do not 
## support 'nofinal' compilation option.
KDE4_NO_ENABLE_FINAL(liblprof)

ADD_DEFINITIONS("-w")
ADD_DEFINITIONS("-fomit-frame-pointer")

SET(liblprof_STAT_SRCS
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

KDE4_ADD_LIBRARY(liblprof STATIC ${liblprof_STAT_SRCS})


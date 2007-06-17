
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} 
                     ${CMAKE_CURRENT_BINARY_DIR}) 

ADD_DEFINITIONS("-w")

SET(liblapack_STAT_SRCS abort_.c
                        dgesv.c 
                        dlaswp.c
                        endfile.c 
                        idamax.c 
                        open.c 
                        sig_die.c 
                        wref.c 
                        close.c 
                        dgetf2.c 
                        dscal.c 
                        err.c 
                        ieeeck.c 
                        s_cmp.c 
                        s_stop.c 
                        wrtfmt.c 
                        dgemm.c 
                        dgetrf.c 
                        dswap.c 
                        fmt.c 
                        ilaenv.c 
                        s_copy.c 
                        wsfe.c 
                        dger.c 
                        dgetrs.c 
                        dtrsm.c 
                        fmtlib.c 
                        lsame.c 
                        sfe.c 
                        util.c 
                        xerbla.c
   )

KDE4_ADD_LIBRARY(liblapack STATIC ${liblapack_STAT_SRCS})

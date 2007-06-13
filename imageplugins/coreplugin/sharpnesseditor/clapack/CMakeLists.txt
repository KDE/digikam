INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(lapack_STAT_SRCS
   abort_.c
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

KDE3_AUTOMOC(${lapack_STAT_SRCS})

ADD_LIBRARY(lapack STATIC ${lapack_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#noinst_LTLIBRARIES = liblapack.la
#
#liblapack_la_CFLAGS = -w
#
#noinst_HEADERS = blaswrap.h clapack.h f2c.h fio.h fmt.h fp.h
#
#liblapack_la_SOURCES = abort_.c dgesv.c dlaswp.c endfile.c idamax.c open.c sig_die.c wref.c close.c dgetf2.c dscal.c err.c ieeeck.c s_cmp.c s_stop.c wrtfmt.c dgemm.c dgetrf.c dswap.c fmt.c ilaenv.c s_copy.c wsfe.c dger.c dgetrs.c dtrsm.c fmtlib.c lsame.c sfe.c util.c xerbla.c

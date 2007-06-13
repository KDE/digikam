INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/thumbbar ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/database ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(batch_STAT_SRCS
   batchthumbsgenerator.cpp
   batchalbumssyncmetadata.cpp
   imageinfojob.cpp
   imageinfoalbumsjob.cpp
   batchsyncmetadata.cpp
   )

KDE3_AUTOMOC(${batch_STAT_SRCS})

ADD_LIBRARY(batch STATIC ${batch_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir)/digikam/digikam \
#	   -I$(top_srcdir)/digikam/libs/dialogs \
#	   -I$(top_srcdir)/digikam/libs/dimg \
#	   -I$(top_srcdir)/digikam/libs/dmetadata \
#	   -I$(top_srcdir)/digikam/libs/thumbbar \
#	   -I$(top_srcdir)/digikam/libs/widgets/common \
#	   -I$(top_srcdir)/digikam/libs/database \
#	   $(LIBKEXIV2_CFLAGS) \
#	   $(all_includes)
#
#noinst_LTLIBRARIES = libbatch.la 
#
#libbatch_la_SOURCES = batchthumbsgenerator.cpp batchalbumssyncmetadata.cpp \
#	              imageinfojob.cpp imageinfoalbumsjob.cpp batchsyncmetadata.cpp 
#
#libbatch_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#

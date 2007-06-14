
include_directories( ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/metadata ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/lprof ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/digikam ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############


########### install files ###############

install( FILES  cietonguewidget.h iccprofilewidget.h iccpreviewwidget.h  DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libiccprofileswidgets.la
#
#libiccprofileswidgets_la_SOURCES = iccprofilewidget.cpp cietonguewidget.cpp iccpreviewwidget.cpp 
#
#libiccprofileswidgets_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#libiccprofileswidgets_la_LIBADD = $(top_builddir)/digikam/libs/lprof/liblprof.la 
#                 
#INCLUDES = -I$(top_srcdir)/digikam/libs/widgets/metadata \
#	       -I$(top_srcdir)/digikam/libs/widgets/common \
#	       -I$(top_srcdir)/digikam/libs/lprof \
#	       -I$(top_srcdir)/digikam/libs/dimg \
#	       -I$(top_srcdir)/digikam/libs/dmetadata \
#	       -I$(top_srcdir)/digikam/digikam \
#	       $(all_includes)
#           
#digikaminclude_HEADERS = cietonguewidget.h iccprofilewidget.h iccpreviewwidget.h 
#
#digikamincludedir = $(includedir)/digikam


include_directories( ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/digikam ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############


########### install files ###############

install( FILES  imageregionwidget.h imagepaniconwidget.h  	                     imagepannelwidget.h imageguidewidget.h  	                     listboxpreviewitem.h imagewidget.h  DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libimagepluginswidgets.la
#
#libimagepluginswidgets_la_SOURCES = imageregionwidget.cpp imagepaniconwidget.cpp \
#	                                imageguidewidget.cpp imagepannelwidget.cpp \
#	                                imagewidget.cpp listboxpreviewitem.cpp 
#
#libimagepluginswidgets_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#                 
#INCLUDES = -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
#	       -I$(top_srcdir)/digikam/libs/dimg \
#	       -I$(top_srcdir)/digikam/libs/dmetadata \
#	       -I$(top_srcdir)/digikam/libs/widgets/common \
#	       -I$(top_srcdir)/digikam/digikam \
#	       $(all_includes)
#
#digikaminclude_HEADERS = imageregionwidget.h imagepaniconwidget.h \
#	                     imagepannelwidget.h imageguidewidget.h \
#	                     listboxpreviewitem.h imagewidget.h 
#
#digikamincludedir = $(includedir)/digikam

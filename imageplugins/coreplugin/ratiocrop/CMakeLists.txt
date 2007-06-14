
include_directories( ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas ${CMAKE_SOURCE_DIR}/digikam/libs/histogram ${CMAKE_SOURCE_DIR}/digikam/libs/levels ${CMAKE_SOURCE_DIR}/digikam/libs/curves ${CMAKE_SOURCE_DIR}/digikam/libs/whitebalance ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/digikam ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############


########### install files ###############




#original Makefile.am contents follow:

#noinst_LTLIBRARIES = libratiocrop.la
#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
#	   -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
#	   -I$(top_srcdir)/digikam/libs/histogram \
#	   -I$(top_srcdir)/digikam/libs/levels \
#	   -I$(top_srcdir)/digikam/libs/curves \
#	   -I$(top_srcdir)/digikam/libs/whitebalance \
#	   -I$(top_srcdir)/digikam/libs/widgets/common \
#	   -I$(top_srcdir)/digikam/libs/widgets/iccprofiles \
#	   -I$(top_srcdir)/digikam/libs/widgets/imageplugins \
#	   -I$(top_srcdir)/digikam/libs/dialogs \
#	   -I$(top_srcdir)/digikam/libs/dimg \
#	   -I$(top_srcdir)/digikam/libs/dmetadata \
#	   -I$(top_srcdir)/digikam/libs/dimg/filters \
#	   -I$(top_srcdir)/digikam/digikam \
#	   $(all_includes) 
#
#libratiocrop_la_SOURCES = imageeffect_ratiocrop.cpp imageselectionwidget.cpp 
#
#libratiocrop_la_LDFLAGS = $(all_libraries)
#
#noinst_HEADERS = imageeffect_ratiocrop.h imageselectionwidget.h 
#

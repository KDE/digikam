INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/metadata ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(dialog_STAT_SRCS
   ctrlpaneldlg.cpp
   imagedlgbase.cpp
   imageguidedlg.cpp
   iccprofileinfodlg.cpp
   deletedialog.cpp
   dprogressdlg.cpp
   )

KDE3_AUTOMOC(${dialog_STAT_SRCS})

SET( dialog_UI
   deletedialogbase.ui
   )

KDE3_ADD_UI_FILES(dialog_STAT_SRCS ${dialog_UI} )

ADD_LIBRARY(dialog STATIC ${dialog_STAT_SRCS})


########### next target ###############

SET(dialogshowfoto_STAT_SRCS
   iccprofileinfodlg.cpp
   )

KDE3_AUTOMOC(${dialogshowfoto_STAT_SRCS})

ADD_LIBRARY(dialogshowfoto STATIC ${dialogshowfoto_STAT_SRCS})


########### install files ###############

INSTALL_FILES( /include/digikam FILES  ctrlpaneldlg.h imagedlgbase.h imageguidedlg.h  	                     iccprofileinfodlg.h dprogressdlg.h   )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libdialog.la libdialogshowfoto.la
#
## Dialogs collection used by Showfoto.
#
#libdialogshowfoto_la_SOURCES = iccprofileinfodlg.cpp  
#
#libdialogshowfoto_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
## Dialogs collection used by digiKam.
#
#libdialog_la_SOURCES = deletedialogbase.ui \
#	                   ctrlpaneldlg.cpp imagedlgbase.cpp imageguidedlg.cpp \
#	                   iccprofileinfodlg.cpp deletedialog.cpp dprogressdlg.cpp 
#
#libdialog_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
#	       -I$(top_srcdir)/digikam/libs/dmetadata \
#	       -I$(top_srcdir)/digikam/digikam \
#	       -I$(top_srcdir)/digikam/libs/dimg/filters \
#	       -I$(top_srcdir)/digikam/libs/widgets/common \
#	       -I$(top_srcdir)/digikam/libs/widgets/metadata \
#	       -I$(top_srcdir)/digikam/libs/widgets/iccprofiles \
#	       -I$(top_srcdir)/digikam/libs/widgets/imageplugins \
#	       -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
#	       $(all_includes)
#
#digikaminclude_HEADERS = ctrlpaneldlg.h imagedlgbase.h imageguidedlg.h \
#	                     iccprofileinfodlg.h dprogressdlg.h  
#digikamincludedir = $(includedir)/digikam
#

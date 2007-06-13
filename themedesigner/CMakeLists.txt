INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/themeengine ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/imageproperties ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(digikamthemedesigner_SRCS
   main.cpp
   mainwindow.cpp
   themedicongroupitem.cpp
   themediconitem.cpp
   themediconview.cpp
   )

KDE3_AUTOMOC(${digikamthemedesigner_SRCS})

KDE3_ADD_EXECUTABLE(digikamthemedesigner ${digikamthemedesigner_SRCS})

TARGET_LINK_LIBRARIES(digikamthemedesigner  ${QT_AND_KDECORE_LIBS} kdeui digikam kutils )

INSTALL( TARGETS digikamthemedesigner DESTINATION bin)


########### install files ###############

INSTALL_FILES( /share/applications/kde FILES  digikamthemedesigner.desktop )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir)/digikam/digikam \
#	       -I$(top_srcdir)/digikam/libs/widgets/common \
#	       -I$(top_srcdir)/digikam/libs/dimg \
#	       -I$(top_srcdir)/digikam/libs/themeengine \
#	       -I$(top_srcdir)/digikam/libs/dmetadata \
#	       -I$(top_srcdir)/digikam/libs/imageproperties \
#	       $(LIBKEXIV2_CFLAGS) $(LIBKDCRAW_CFLAGS) $(all_includes) 
#
#bin_PROGRAMS = digikamthemedesigner
#
#digikamthemedesigner_SOURCES = main.cpp mainwindow.cpp themedicongroupitem.cpp \
#                               themediconitem.cpp themediconview.cpp 
#
#digikamthemedesigner_LDADD = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) \
#	                     $(top_builddir)/digikam/digikam/libdigikam.la 
#
#digikamthemedesigner_LDFLAGS = $(KDE_RPATH) $(all_libraries) $(LIB_KUTILS) 
#
#xdg_apps_DATA = digikamthemedesigner.desktop

PROJECT(digikam)

FIND_PACKAGE(KDE4 REQUIRED)
FIND_PACKAGE(LCMS REQUIRED)
FIND_PACKAGE(JPEG REQUIRED)
FIND_PACKAGE(PNG REQUIRED)
FIND_PACKAGE(TIFF REQUIRED)
FIND_PACKAGE(Jasper REQUIRED)
find_package(Kdcraw REQUIRED)
find_package(Kexiv2 REQUIRED)
#find_package(Kipi REQUIRED)
#FIND_PACKAGE(Sqlite REQUIRED)

SET(CMAKE_VERBOSE_MAKEFILE ON)

ADD_DEFINITIONS(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

INCLUDE_DIRECTORIES(${KEXIV2_INCLUDE_DIR} ${KDCRAW_INCLUDE_DIR})


include_directories( ${KDE4_INCLUDE_DIR}
                     ${QT_INCLUDES}
                   )


ADD_SUBDIRECTORY( sqlite )
ADD_SUBDIRECTORY( libs )
#ADD_SUBDIRECTORY( utilities )
#ADD_SUBDIRECTORY( digikam )
#ADD_SUBDIRECTORY( showfoto )
#ADD_SUBDIRECTORY( data )
#ADD_SUBDIRECTORY( kioslave )
#ADD_SUBDIRECTORY( imageplugins )
#ADD_SUBDIRECTORY( themedesigner )

install( FILES  tips DESTINATION ${DATA_INSTALL_DIR}/digikam )

#kde4_create_doxygen_docs(  )



#original Makefile.am contents follow:

#COMPILE_FIRST = sqlite
#
#SUBDIRS = sqlite libs utilities digikam showfoto data kioslave imageplugins themedesigner
#
#EXTRA_DIST = AUTHORS COPYING ChangeLog INSTALL README TODO digikam.lsm 
#
#AUTOMAKE_OPTIONS = foreign 
#
#MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 
#
#
#tip_DATA = tips
#tipdir = $(kde_datadir)/digikam
#
#messages: rc.cpp
#	$(EXTRACTRC) `find . -name "*.ui"` >> rc.cpp
#	$(EXTRACTRC) `find . -name "*.rc"` >> rc.cpp
#	$(PREPARETIPS) > tips.cpp
#	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
#	if test -n "$$LIST"; then \
#		$(XGETTEXT) $$LIST -o $(podir)/digikam.pot; \
#	fi
#	rm -f tips.cpp
#
#include ../admin/Doxyfile.am
#
#fastcompile:
#	for i in libs utilities digikam kioslave; \
#	do \
#		cd $$i; \
#		make || exit 1; \
#		cd ..; \
#	done
#
#fastinstall:
#	for i in libs utilities digikam kioslave; \
#	do \
#		cd $$i; \
#		make install || exit 1; \
#		cd ..; \
#	done

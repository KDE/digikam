project(digikam)

find_package(KDE4 REQUIRED)
find_package(LCMS REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(TIFF REQUIRED)
find_package(Jasper REQUIRED)
find_package(Kdcraw REQUIRED)
#find_package(Kexiv2 REQUIRED)
#find_package(Kipi REQUIRED)
#find_package(Sqlite REQUIRED)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include(ConfigureChecks.cmake)

add_subdirectory( sqlite )
#add_subdirectory( libs )
#add_subdirectory( utilities )
#add_subdirectory( digikam )
#add_subdirectory( showfoto )
#add_subdirectory( data )
#add_subdirectory( kioslave )
#add_subdirectory( imageplugins )
#add_subdirectory( themedesigner )

########### install files ###############

install( FILES  tips DESTINATION ${DATA_INSTALL_DIR}/digikam )

kde4_create_doxygen_docs(  )



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

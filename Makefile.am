COMPILE_FIRST = libs

SUBDIRS = libs utilities digikam showfoto data kioslave imageplugins themedesigner

EXTRA_DIST = AUTHORS COPYING ChangeLog INSTALL README TODO digikam.lsm 

AUTOMAKE_OPTIONS = foreign 

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 


tip_DATA = tips
tipdir = $(kde_datadir)/digikam

messages: rc.cpp
	$(EXTRACTRC) `find . -name "*.ui"` >> rc.cpp
	$(EXTRACTRC) `find . -name "*.rc"` >> rc.cpp
	$(PREPARETIPS) > tips.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
		$(XGETTEXT) $$LIST -o $(podir)/digikam.pot; \
	fi
	rm -f tips.cpp

include ../admin/Doxyfile.am

SUBDIRS = adjustlevels solarize despeckle oilpaint emboss charcoal unsharp raindrop filmgrain 

AUTOMAKE_OPTIONS = foreign 

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 

picsdir = $(kde_datadir)/digikamimageplugins/data
pics_DATA = digikamimageplugins_banner_left.png

iconsdir = $(kde_datadir)/digikamimageplugins/icons/hicolor/32x32/actions

KDE_ICON = digikamimageplugins

tip_DATA = tips
tipdir = $(kde_datadir)/digikamimageplugins

messages: rc.cpp
	$(PREPARETIPS) > tips.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
		$(XGETTEXT) $$LIST -o $(podir)/digikamimageplugins.pot; \
	fi
	rm -f tips.cpp


SUBDIRS = adjustlevels solarize despeckle oilpaint emboss charcoal unsharp \
	  raindrop filmgrain freerotation adjustcurves sheartool antivignetting \
	  lensdistortion superimpose perspective border blurfx distortionfx \
	  inserttext infrared channelmixer texture whitebalance common 

AUTOMAKE_OPTIONS = foreign 

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 

tip_DATA = tips
tipdir = $(kde_datadir)/digikamimageplugins

messages: rc.cpp
	$(PREPARETIPS) >> tips.cpp
	LIST=`find . -name \tips.cpp`; \
	if test -n "$$LIST"; then \
		$(XGETTEXT) $$LIST -o $(podir)/digikamimageplugins.pot; \
	fi
	rm -f tips.cpp


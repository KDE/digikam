tip_DATA = tips
tipdir = $(kde_datadir)/digikamimageplugins

messages: rc.cpp
	$(PREPARETIPS) >> tips.cpp
	LIST=`find . -name \tips.cpp`; \
	if test -n "$$LIST"; then \
		$(XGETTEXT) $$LIST -o $(podir)/digikamimageplugins.pot; \
	fi
	rm -f tips.cpp


METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/digikam/digikam \
	       -I$(top_srcdir)/digikam/libs/widgets/common \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/themeengine \
	       -I$(top_srcdir)/digikam/libs/dmetadata \
	       -I$(top_srcdir)/digikam/libs/imageproperties \
	       $(LIBKEXIV2_CFLAGS) $(LIBKDCRAW_CFLAGS) $(all_includes) 

bin_PROGRAMS = digikamthemedesigner

digikamthemedesigner_SOURCES = main.cpp mainwindow.cpp themedicongroupitem.cpp \
                               themediconitem.cpp themediconview.cpp 

digikamthemedesigner_LDADD = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) \
	                     $(top_builddir)/digikam/digikam/libdigikam.la 

digikamthemedesigner_LDFLAGS = $(KDE_RPATH) $(all_libraries) $(LIB_KUTILS) 

METASOURCES = AUTO

# CImg.h version 1.2.8 do not compile fine with gcc 4.3.x 
# See B.K.O #163118: digikam-0.9.4_beta5 compilation hangs with gcc 4.3
# Using -fno-tree-pre is work around this problem.
KDE_CXXFLAGS = $(USE_EXCEPTIONS) -w -fno-tree-pre

noinst_LTLIBRARIES = libgreycstoration.la

libgreycstoration_la_SOURCES = greycstorationiface.cpp greycstorationwidget.cpp

libgreycstoration_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/dimg/filters \
	       -I$(top_srcdir)/digikam/digikam \
		$(LIBKDCRAW_CFLAGS) \
		$(all_includes)

digikaminclude_HEADERS = greycstorationiface.h greycstorationwidget.h greycstorationsettings.h 

digikamincludedir = $(includedir)/digikam


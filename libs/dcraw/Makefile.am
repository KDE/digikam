INCLUDES = -I$(top_srcdir)/digikam/digikam $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libdcraw.la

libdcraw_la_SOURCES = dcraw_parse.cpp dcrawbinary.cpp dcrawpreview.cpp

libdcraw_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

#-- dcraw binary -------------------------------------------------

bin_PROGRAMS = digikamdcraw

# This line is require to prevent broken compilation with -std=iso9899:1990 gcc option.
AM_CFLAGS = -w

digikamdcraw_SOURCES = dcraw.c

digikamdcraw_LDFLAGS = $(all_libraries) $(LIBJPEG) $(LCMS_LIBS) 

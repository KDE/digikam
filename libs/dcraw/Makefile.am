INCLUDES = -I$(top_srcdir)/digikam/digikam $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libdcraw.la

libdcraw_la_SOURCES = dcraw_parse.cpp dcrawbinary.cpp dcrawpreview.cpp

libdcraw_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

#-- dcraw binary -------------------------------------------------
# digiKam use a dedicaced dcraw binary program to run. This is mandatory 
# since dcraw author have broken the command line option 
# compatibility with the 8.x serie.
# Also, dcraw is not available like a library.
# The dcraw.c source code is just a copy of official implementation, but 
# this one have been completly tested with the digiKam core.

bin_PROGRAMS = digikamdcraw

# This line is require to prevent broken compilation with -std=iso9899:1990 gcc option.
KDE_CFLAGS = -std=c99

digikamdcraw_SOURCES = dcraw.c

digikamdcraw_LDFLAGS = $(all_libraries) $(LIBJPEG) $(LCMS_LIBS) 

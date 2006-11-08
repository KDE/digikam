# Requires by dcraw.c implementation.
KDE_OPTIONS = nofinal

METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/digikam/digikam \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       $(all_includes)

noinst_LTLIBRARIES = libdcraw.la

libdcraw_la_SOURCES = dcraw_parse.cpp dcrawbinary.cpp dcrawpreview.cpp

libdcraw_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

#-- dcraw binary -------------------------------------------------

# digiKam use a dedicaced dcraw binary program to run. This is mandatory 
# since dcraw author have broken the command line options 
# compatibility with the 8.x serie.
# Also, dcraw is not available like a library.
# The dcraw.c source code is just a copy of official implementation, but 
# this one have been completly tested with digiKam core. 
# Note: dcraw project page is http://cybercom.net/~dcoffin

bin_PROGRAMS = digikamdcraw

# This line is require to prevent broken compilation with -std=iso9899:1990 gcc option.
# In fact, dcraw.c use lcms.h witch use the "inline" keyword witch do not exist in pure C 
# implementation. The -Dinline option please compiler to see inline with this option.
# Note : using -std-c99 instead -std=iso9899:1990 is not possible because some compiler 
# do not support it.
# Thanks to Andras Mantia for this tip.
digikamdcraw_CFLAGS = -Dinline= -w

digikamdcraw_SOURCES = dcraw.c

digikamdcraw_LDFLAGS = $(all_libraries) $(LIBJPEG) $(LCMS_LIBS) 

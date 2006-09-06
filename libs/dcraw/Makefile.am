INCLUDES = -I$(top_srcdir)/digikam/digikam $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libdcraw.la

libdcraw_la_SOURCES = dcraw_parse.cpp dcrawbinary.cpp dcrawpreview.cpp

libdcraw_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# binary
#bin_PROGRAMS = digikamdcraw

#digikamdcraw_CFLAGS = -w

#digikamdcraw_SOURCES = dcraw.c

#digikamdcraw_LDFLAGS = -O4 -lm -ljpeg -lcms 
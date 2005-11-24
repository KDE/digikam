METASOURCES = AUTO

noinst_LTLIBRARIES = libdimg.la

libdimg_la_SOURCES = dimg.cpp dimgloader.cpp dimgscale.cpp colormodifier.cpp \
	             jpegloader.cpp tiffloader.cpp rawloader.cpp pngloader.cpp \
	             ppmloader.cpp qimageloader.cpp iccjpeg.c icctransform.cpp 
             
libdimg_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) \
		     $(LIBJPEG) $(LIB_TIFF) $(LCMS_LIBS) \
	             $(top_builddir)/digikam/libs/dcraw/libdcraw.la 
	                    
INCLUDES = $(all_includes) 

digikaminclude_HEADERS = dimg.h dimgloader.h colormodifier.h icctransform.h

digikamincludedir = $(includedir)/digikam

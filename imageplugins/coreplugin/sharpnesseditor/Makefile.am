SUBDIRS = clapack 
COMPILE_FIRST = clapack 

noinst_LTLIBRARIES = libsharpnesseditor.la
METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	   -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
	   -I$(top_srcdir)/digikam/libs/histogram \
	   -I$(top_srcdir)/digikam/libs/levels \
	   -I$(top_srcdir)/digikam/libs/curves \
	   -I$(top_srcdir)/digikam/libs/whitebalance \
	   -I$(top_srcdir)/digikam/libs/widgets/common \
	   -I$(top_srcdir)/digikam/libs/widgets/iccprofiles \
	   -I$(top_srcdir)/digikam/libs/widgets/imageplugins \
	   -I$(top_srcdir)/digikam/libs/dialogs \
	   -I$(top_srcdir)/digikam/libs/dimg \
	   -I$(top_srcdir)/digikam/libs/dmetadata \
	   -I$(top_srcdir)/digikam/libs/dimg/filters \
	   -I$(top_srcdir)/digikam/digikam \
	   -I$(top_srcdir)/digikam/imageplugins/coreplugin/sharpnesseditor/clapack \
	   $(all_includes) 

libsharpnesseditor_la_LIBADD = $(top_builddir)/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/liblapack.la 

libsharpnesseditor_la_SOURCES = imageeffect_sharpen.cpp unsharp.cpp matrix.cpp refocus.cpp 

libsharpnesseditor_la_LDFLAGS = $(all_libraries)

noinst_HEADERS = imageeffect_sharpen.h unsharp.h matrix.h refocus.h 


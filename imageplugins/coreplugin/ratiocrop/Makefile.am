noinst_LTLIBRARIES = libratiocrop.la
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
	   $(LIBKDCRAW_CFLAGS) \
	   $(all_includes) 

libratiocrop_la_SOURCES = ratiocroptool.cpp imageselectionwidget.cpp 

libratiocrop_la_LDFLAGS = $(all_libraries)

noinst_HEADERS = ratiocroptool.h imageselectionwidget.h 


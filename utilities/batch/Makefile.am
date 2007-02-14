METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/digikam/digikam \
	   -I$(top_srcdir)/digikam/libs/dialogs \
	   -I$(top_srcdir)/digikam/libs/dimg \
	   -I$(top_srcdir)/digikam/libs/dmetadata \
	   -I$(top_srcdir)/digikam/libs/thumbbar \
	   -I$(top_srcdir)/digikam/libs/widgets/common \
	   $(all_includes)

noinst_LTLIBRARIES = libbatch.la 

libbatch_la_SOURCES = batchthumbsgenerator.cpp batchalbumssyncmetadata.cpp \
	              imageinfojob.cpp imageinfoalbumsjob.cpp batchsyncmetadata.cpp 

libbatch_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)



METASOURCES = AUTO

noinst_LTLIBRARIES = libdigikamimagepluginsdialog.la

libdigikamimagepluginsdialog_la_SOURCES = ctrlpaneldialog.cpp imageguidedialog.cpp \
                                          imagetabdialog.cpp 

libdigikamimagepluginsdialog_la_LIBADD = \
	              $(top_builddir)/digikamimageplugins/common/widgets/libdigikamimagepluginswidget.la 
                      
libdigikamimagepluginsdialog_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikamimageplugins/common/include \
	   -I$(top_srcdir)/digikamimageplugins/common/widgets \
	   $(all_includes) $(LIBDIGIKAM_CFLAGS) 


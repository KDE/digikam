METASOURCES = AUTO

noinst_LTLIBRARIES = libcameragui.la

libcameragui_la_SOURCES = cameraui.cpp   cameraguiclient.cpp \
			cameraiconview.cpp  cameraiconitem.cpp \
			cameracontroller.cpp cameradownloaddlg.cpp  \
			camerathumbjob.cpp dirselectdialog.cpp

libcameragui_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) 

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	-I$(top_srcdir)/digikam/utilities/imageeditor \
	$(all_includes) 

digikamcameraguiicondir = $(kde_datadir)/digikam/icons
digikamcameraguiicon_ICON = AUTO


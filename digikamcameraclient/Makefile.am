## Makefile.am for digikamcameraclient

SUBDIRS = cameraui pics

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = digikamcameraclient

# set the include path for X, qt and KDE
INCLUDES = \
	-I$(top_srcdir)/digikam/digikamcameraclient/cameraui \
	-I$(top_srcdir)/digikam/digikam \
	$(all_includes)


# the library search path. 
digikamcameraclient_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# the libraries to link against.
digikamcameraclient_LDADD   = $(LIB_KDEUI) $(LIB_KFILE) \
	$(top_builddir)/digikam/digikamcameraclient/cameraui/libcameraui.la \
	$(top_builddir)/digikam/digikam/libdigikam.la

# which sources should be compiled for digikamcameraclient
digikamcameraclient_SOURCES =	main.cpp digikamcameraclient.cpp \
				digikamcameraclientiface.skel 

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO




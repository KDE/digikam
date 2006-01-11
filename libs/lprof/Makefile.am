noinst_LTLIBRARIES = liblprof.la

# FIXME: i using this line to disable all main compilation options and using "-fomit-frame-pointer" option instead,
# else we cannot compile these files. If anybody have a better solution (11/01/06-Gilles Caulier <caulier dot gilles at free dot fr>)
CFLAGS = -fomit-frame-pointer

noinst_HEADERS = lcmsprf.h

liblprof_la_SOURCES = cmshull.c cmslm.c cmslnr.c cmsmatn.c cmsmkmsh.c cmsmntr.c cmsoutl.c \
	                  cmspcoll.c cmsprf.c cmsreg.c cmsscn.c cmssheet.c

INCLUDES = $(all_includes)

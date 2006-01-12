noinst_LTLIBRARIES = liblprof.la

# FIXME: i using CFLAGS to disable all main compilation options and using "-fomit-frame-pointer" option instead,
# else we cannot compile these files. The problem is relevant of using '-std=iso9899:1990' compilation option by default.
# How to disable only this option here ? (12/01/06-Gilles Caulier <caulier dot gilles at free dot fr>)
#KDE_CFLAGS = -fomit-frame-pointer
CFLAGS = -fomit-frame-pointer

noinst_HEADERS = lcmsprf.h

liblprof_la_SOURCES = cmshull.c cmslm.c cmslnr.c cmsmatn.c cmsmkmsh.c cmsmntr.c cmsoutl.c \
	                  cmspcoll.c cmsprf.c cmsreg.c cmsscn.c cmssheet.c

INCLUDES = $(all_includes)

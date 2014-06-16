#
#  File        : Makefile
#                ( Makefile for GNU 'make' utility )
#
#  Description : Makefile for compiling :
#
#                 . the G'MIC command line tool (type 'make').
#                 . the G'MIC plug-in for GIMP (type 'make gimp').
#                 . the G'MIC interpreter, as a C++ library (type 'make lib').
#
#                ( http://gmic.sourceforge.net )
#
#  Copyright   : David Tschumperle
#                ( http://tschumperle.users.greyc.fr/ )
#
#  License     : CeCILL v2.0
#                ( http://www.cecill.info/licences/Licence_CeCILL_V2-en.html )
#
#  This software is governed by the CeCILL  license under French law and
#  abiding by the rules of distribution of free software.  You can  use,
#  modify and/ or redistribute the software under the terms of the CeCILL
#  license as circulated by CEA, CNRS and INRIA at the following URL
#  "http://www.cecill.info".
#
#  As a counterpart to the access to the source code and  rights to copy,
#  modify and redistribute granted by the license, users are provided only
#  with a limited warranty  and the software's author,  the holder of the
#  economic rights,  and the successive licensors  have only  limited
#  liability.
#
#  In this respect, the user's attention is drawn to the risks associated
#  with loading,  using,  modifying and/or developing or reproducing the
#  software by the user in light of its specific status of free software,
#  that may mean  that it is complicated to manipulate,  and  that  also
#  therefore means  that it is reserved for developers  and  experienced
#  professionals having in-depth computer knowledge. Users are therefore
#  encouraged to load and test the software's suitability as regards their
#  requirements in conditions enabling the security of their systems and/or
#  data to be ensured and,  more generally, to use and operate it in the
#  same conditions as regards security.
#
#  The fact that you are presently reading this means that you have had
#  knowledge of the CeCILL license and that you accept its terms.
#

#----------------------------------
# Set correct variables and paths.
#----------------------------------
OS := $(shell uname)
CC = g++
ifeq ($(OS),Linux)
OS = Unix
endif
ifeq ($(OS),GNU/kFreeBSD)
OS = Unix
endif
ifeq ($(OS),GNU)
OS = Unix
endif
ifeq ($(OS),Darwin)
PLUGINDIR = ~/Library/Application\ Support/Gimp/plug-ins
USR = /opt/local
else
PLUGINDIR = `gimptool-2.0 --gimpplugindir`/plug-ins
USR = /usr
endif
ifeq ($(OSTYPE),msys)
EXE = .exe
endif

#------------------------------------------------
# Set compilation flags for build customization.
#------------------------------------------------

# Flags to enable 'beta' version.
IS_BETA_CFLAGS = 

# Flags that are mandatory to compile 'gmic'.
MANDATORY_CFLAGS += -Dgmic_build -I$(USR)/include
MANDATORY_LDFLAGS += -L$(USR)/lib
ifeq ($(CC),g++)
MANDATORY_CFLAGS += -Wall -W
MANDATORY_LDFLAGS += -lm
endif
MANDATORY_CFLAGS+=$(IS_BETA_CFLAGS)
ifeq ($(OS),Unix)
MANDATORY_CFLAGS+= -Dcimg_use_vt100
endif
ifeq ($(OSTYPE),msys)
MANDATORY_LDFLAGS = -Wl,--stack,16777216
endif

# Flags to enable debugging.
DEBUG_CFLAGS = -Dcimg_verbosity=3 -g

# Flags to enable optimizations.
ifeq ($(CC),g++)
OPT_CFLAGS = -O2 -fno-ipa-sra -mtune=generic # -fno-tree-pre # -ffast-math
endif
ifeq ($(CC),clang++)
OPT_CFLAGS = -O1
endif
ifeq ($(CC),icc)
OPT_CFLAGS = -fast
endif

# Flags to enable parallel processing.
ifneq ($(OS),Darwin)
PARALLEL_CFLAGS = -Dgmic_is_parallel
ifneq ($(OSTYPE),msys)
PARALLEL_LDFLAGS = -lpthread
endif
endif

# Flags to enable extra checking of image buffer pointers.
ifneq ($(OS),Darwin)
CHECKIMAGE_CFLAGS = -Dgmic_check_image
endif

# Flags to enable image display, using X11
# (keep /usr/ dirname here since X11 is located in /usr/ on Mac too).
# This requires the presence of the X11 include and library files.
# (package 'libx11-dev' on Debian).
X11_CFLAGS = -Dcimg_display=1 -Dcimg_appname=\\\"gmic\\\" -I/usr/X11R6/include #-Dcimg_use_xrandr
X11_LDFLAGS = -L/usr/X11R6/lib -lX11 -lpthread #-lXrandr

# Flags to enable fast display, using XShm.
# This requires the presence of the X11 extension include and library files.
# (package 'libx11-dev' on Debian).
XSHM_CFLAGS = -Dcimg_use_xshm
XSHM_LDFLAGS = -L$(USR)/X11R6/lib -lXext

# Flags to enable image display, using GDI32.
# This requires the presence of the GDI32 include and library files.
GDI32_CFLAGS = -Dcimg_display=2 -Dcimg_appname=\\\"gmic\\\"
GDI32_LDFLAGS = -lgdi32

# Flags to enable native support for PNG image files, using the PNG library.
# This requires the presence of the libpng include and library files.
# (package 'libpng12-dev' on Debian).
PNG_CFLAGS = -Dcimg_use_png
PNG_LDFLAGS = -lpng -lz

# Flags to enable native support for JPEG image files, using the JPEG library.
# This requires the presence of the libjpeg include and library files.
# (package 'libjpeg62-dev' on Debian).
JPEG_CFLAGS = -Dcimg_use_jpeg
JPEG_LDFLAGS = -ljpeg

# Flags to enable native support for TIFF image files, using the TIFF library.
# This requires the presence of the libtiff include and library files.
# (package 'libtiff4-dev' on Debian).
TIFF_CFLAGS = -Dcimg_use_tiff
TIFF_LDFLAGS = -ltiff

# Flags to enable native support for MINC2 image files, using the MINC2 library.
# ( http://en.wikibooks.org/wiki/MINC/Reference/MINC2.0_Users_Guide )
# (package 'libminc-dev' on Debian).
MINC2_CFLAGS = -Dcimg_use_minc2 -I${HOME}/local/include
MINC2_LDFLAGS = -lminc_io -lvolume_io2 -lminc2 -lnetcdf -lhdf5 -lz -L${HOME}/local/lib

# Flags to enable native support for various video files, using the FFMPEG library.
# This requires the presence of the FFMPEG include and library files.
# (packages 'libavcodec-dev', 'libavutil-dev', 'libavformat-dev' and 'libswscale-dev' on Debian).
FFMPEG_CFLAGS = -Dcimg_use_ffmpeg -D__STDC_CONSTANT_MACROS -I$(USR)/include/libavcodec -I$(USR)/include/libavformat -I$(USR)/include/libswscale -I$(USR)/include/ffmpeg
FFMPEG_LDFLAGS = -lavcodec -lavformat -lswscale

# Flags to enable native support for compressed .cimgz files, using the Zlib library.
# This requires the presence of the Zlib include and library files.
# (package 'zlib1g-dev' on Debian).
ZLIB_CFLAGS = -Dcimg_use_zlib
ZLIB_LDFLAGS = -lz

# Flags to enable native support of webcams, using the OpenCV library.
# This requires the presence of the OpenCV include and library files.
# (package 'libcv3-2-dev' on Debian).
ifeq ($(OS),Darwin)
OPENCV_CFLAGS = -Dcimg_use_opencv -I$(USR)/include -I$(USR)/include/opencv
OPENCV_LDFLAGS = `pkg-config opencv --libs`   #-> Use this for OpenCV 2.2.0 !
else
OPENCV_CFLAGS = -Dcimg_use_opencv -I$(USR)/include -I$(USR)/include/opencv
# OPENCV_LDFLAGS = -lcv -lhighgui
OPENCV_LDFLAGS = -lopencv_core -lopencv_highgui   #-> Use this for OpenCV >= 2.2.0 !
endif

# Flags to enable native support of most classical image file formats, using the GraphicsMagick++ library.
# This requires the presence of the GraphicsMagick++ include and library files.
# (package 'libgraphicsmagick++1-dev' on Debian).
MAGICK_CFLAGS = -Dcimg_use_magick -I$(USR)/include/GraphicsMagick
ifeq ($(OS),Darwin)
MAGICK_LDFLAGS = -L$(USR)/lib -lGraphicsMagick++ -lGraphicsMagick -llcms -ltiff -lfreetype -ljpeg -lpng -lbz2 -lxml2 -lz -lm -lltdl
else
MAGICK_LDFLAGS = -lGraphicsMagick++
endif

# Flags to enable native support of EXR file format, using the OpenEXR library/
# This requires the presence of the OpenEXR include and library files.
# (package 'libopenexr-dev' on Debian).
EXR_CFLAGS = -Dcimg_use_openexr -I$(USR)/include/OpenEXR
EXR_LDFLAGS = -lIlmImf -lHalf

# Flags to enable the use of the FFTW3 library.
# This requires the presence of the FFTW3 include and library files.
# (package 'libfftw3-dev' on Debian).
FFTW_CFLAGS = -Dcimg_use_fftw3
FFTW_LDFLAGS = -lfftw3 -lfftw3_threads
ifeq ($(OSTYPE),msys)
FFTW_LDFLAGS = -lfftw3-3
endif

# Flags to enable the use of the BOARD library.
# This requires the presence of the BOARD include and library files.
# (no packages exist for Debian at this time).
BOARD_CFLAGS = -Dcimg_use_board
BOARD_LDFLAGS = -lboard

#----------------------------------------------------------------
# Predefined sets of flags for different default configurations.
#----------------------------------------------------------------

# Unix : Standard build.
STD_UNIX_CFLAGS = $(MANDATORY_CFLAGS) $(PARALLEL_CFLAGS) $(X11_CFLAGS) $(PNG_CFLAGS) \
                  $(JPEG_CFLAGS) $(TIFF_CFLAGS) $(ZLIB_CFLAGS) \
	  	  $(EXR_CFLAGS) $(FFTW_CFLAGS) # $(XSHM_CFLAGS) $(OPENCV_CFLAGS) $(FFMPEG_CFLAGS) # $(MAGICK_CFLAGS)
STD_UNIX_LDFLAGS = $(MANDATORY_LDFLAGS) $(PARALLEL_LDFLAGS) $(X11_LDFLAGS) $(PNG_LDFLAGS) \
		   $(JPEG_LDFLAGS) $(TIFF_LDFLAGS) $(ZLIB_LDFLAGS) \
	           $(EXR_LDFLAGS) $(FFTW_LDFLAGS) # $(XSHM_LDFLAGS) $(OPENCV_LDFLAGS) $(FFMPEG_LDFLAGS) # $(MAGICK_LDFLAGS)

# Unix : Minimal build.
MINIMAL_UNIX_CFLAGS = $(MANDATORY_CFLAGS) $(PARALLEL_CFLAGS) $(DEBUG_CFLAGS) $(X11_CFLAGS) \
	              ${TIFF_CFLAGS} ${PNG_CFLAGS} $(ZLIB_CFLAGS) $(FFTW_CFLAGS) $(CHECKIMAGE_CFLAGS) # $(OPENCV_CFLAGS)
MINIMAL_UNIX_LDFLAGS = $(MANDATORY_LDFLAGS) $(PARALLEL_LDFLAGS) $(X11_LDFLAGS) \
	               ${TIFF_LDFLAGS} ${PNG_LDFLAGS} $(ZLIB_LDFLAGS) $(FFTW_LDFLAGS) # $(OPENCV_LDFLAGS)

# Unix : Static build.
STATIC_PATH = /usr/lib/
STATIC_EXTRA =
STATIC_UNIX_CFLAGS = $(MANDATORY_CFLAGS) $(PARALLEL_CFLAGS) $(PNG_CFLAGS) $(JPEG_CFLAGS) $(ZLIB_CFLAGS) $(FFTW_CFLAGS) -Dcimg_display=0
STATIC_UNIX_LDFLAGS = $(PARALLEL_LDFLAGS) \
	              $(STATIC_PATH)/libpng.a \
	              $(STATIC_PATH)/libjpeg.a \
                      $(STATIC_PATH)/libz.a \
                      $(STATIC_PATH)/libfftw3.a $(STATIC_PATH)/libfftw3_threads.a $(STATIC_EXTRA)

# Unix : Custom build.
FULL_UNIX_CFLAGS = $(MANDATORY_CFLAGS) $(PARALLEL_CFLAGS) $(OPT_CFLAGS) $(DEBUG_CFLAGS) $(X11_CFLAGS) $(PNG_CFLAGS) \
                     $(JPEG_CFLAGS) $(TIFF_CFLAGS) $(ZLIB_CFLAGS) \
	             $(FFTW_CFLAGS) $(EXR_CFLAGS) $(OPENCV_CFLAGS) # $(XSHM_CFLAGS) $(FFMPEG_CFLAGS) $(MAGICK_CFLAGS)  $(BOARD_CFLAGS) $(MINC2_CFLAGS)
FULL_UNIX_LDFLAGS = $(MANDATORY_LDFLAGS) $(PARALLEL_LDFLAGS) $(X11_LDFLAGS) $(PNG_LDFLAGS) \
                      $(JPEG_LDFLAGS) $(TIFF_LDFLAGS) $(ZLIB_LDFLAGS) \
	       	      $(FFTW_LDFLAGS) $(EXR_LDFLAGS) $(OPENCV_LDFLAGS) # $(XSHM_LDFLAGS) $(FFMPEG_LDFLAGS) $(MAGICK_LDFLAGS) $(BOARD_LDFLAGS) $(MINC2_LDFLAGS)

# Windows : Standard build.
STD_WINDOWS_CFLAGS= $(MANDATORY_CFLAGS) $(PARALLEL_CFLAGS) $(GDI32_CFLAGS) $(ZLIB_CFLAGS) $(PNG_CFLAGS) $(JPEG_CFLAGS) $(TIFF_CFLAGS) $(FFTW_CFLAGS)
STD_WINDOWS_LDFLAGS = $(MANDATORY_LDFLAGS) $(PARALLEL_LDFLAGS) $(GDI32_LDFLAGS) $(ZLIB_LDFLAGS) $(PNG_LDFLAGS) $(JPEG_LDFLAGS) $(TIFF_LDFLAGS) $(FFTW_LDFLAGS)

# Windows : Minimal build.
MINIMAL_WINDOWS_CFLAGS = $(MANDATORY_CFLAGS) $(PARALLEL_CFLAGS) $(DEBUG_CFLAGS) $(GDI32_CFLAGS) $(CHECKIMAGE_CFLAGS)
MINIMAL_WINDOWS_LDFLAGS = $(MANDATORY_LDFLAGS) $(PARALLEL_LDFLAGS) $(GDI32_LDFLAGS)

# Mac-OSX : Standard build.
STD_MACOSX_CFLAGS= $(MANDATORY_CFLAGS) $(X11_CFLAGS) $(ZLIB_CFLAGS) $(PNG_CFLAGS) \
                   $(JPEG_CFLAGS) $(TIFF_CFLAGS) \
		   $(EXR_CFLAGS) $(FFTW_CFLAGS) # $(MAGICK_CFLAGS)
STD_MACOSX_LDFLAGS = $(MANDATORY_LDFLAGS) $(X11_LDFLAGS) $(ZLIB_LDFLAGS) \
		     $(PNG_LDFLAGS) $(JPEG_LDFLAGS) $(TIFF_LDFLAGS) \
		     $(EXR_LDFLAGS) $(FFTW_LDFLAGS) # $(MAGICK_LDFLAGS)

# GIMP plug-in : Standard build.
STD_GIMP_CFLAGS = $(MANDATORY_CFLAGS) $(PARALLEL_CFLAGS) $(FFTW_CFLAGS) $(PNG_CFLAGS) $(ZLIB_CFLAGS) ${IS_BETA_CFLAGS} -Dcimg_display=0 -Dcimg_use_rng
STD_GIMP_LDFLAGS = $(MANDATORY_LDFLAGS) $(PARALLEL_LDFLAGS) $(FFTW_LDFLAGS) $(PNG_LDFLAGS) $(ZLIB_LDFLAGS)
ifeq ($(OSTYPE),msys)
STD_GIMP_LDFLAGS += -mwindows -lpthread
endif

# Libgmic : Standard build.
STD_LIB_CFLAGS = $(MANDATORY_CFLAGS) $(PARALLEL_CFLAGS) $(FFTW_CFLAGS) -Dcimg_display=0 # -fno-ipa-sra
STD_LIB_LDFLAGS = $(MANDATORY_LDFLAGS) $(PARALLEL_LDFLAGS) $(FFTW_LDFLAGS)

# GMICol : Standard build.
GMICOL_UNIX_CFLAGS = $(MANDATORY_CFLAGS) $(PARALLEL_CFLAGS) $(PNG_CFLAGS) $(JPEG_CFLAGS) $(ZLIB_CFLAGS) $(FFTW_CFLAGS) -Dcimg_display=0
GMICOL_UNIX_LDFLAGS = $(MANDATORY_LDFLAGS) $(PARALLEL_LDFLAGS) $(PNG_LDFLAGS) $(JPEG_LDFLAGS) $(ZLIB_LDFLAGS) /usr/lib/x86_64-linux-gnu/libfftw3.a /usr/lib/x86_64-linux-gnu/libfftw3_threads.a

#--------------------------
# Define Makefile entries.
#--------------------------

# Main entries
all:
ifeq ($(OS),Unix)
	@echo "**"
	@echo "** Generic unix configuration."
	@echo "**"
	$(MAKE) linux gimp lib  # zart
else
ifeq ($(OS),Darwin)
	@echo "**"
	@echo "** MacOSX configuration."
	@echo "**"
	$(MAKE) macosx gimp  # zart
else
	@echo "**"
	@echo "** Windows configuration."
	@echo "**"
	$(MAKE) windows gimp
endif
endif

gimp:
	$(MAKE) "CFLAGS=$(STD_GIMP_CFLAGS) $(OPT_CFLAGS)" "LDFLAGS+=$(STD_GIMP_LDFLAGS) $(OPT_LDFLAGS)" "STRIP_EXE=1" gmic_gimp

lib:
	$(MAKE) "CFLAGS=$(STD_LIB_CFLAGS) $(OPT_CFLAGS)" "LDFLAGS+=$(STD_LIB_LDFLAGS) $(OPT_LDFLAGS)" gmic_lib

zart: lib
ifneq ($(OS),Darwin)
	cd ../zart && qmake-qt4 zart.pro && $(MAKE) && strip zart
else
	cd ../zart && qmake zart.pro && $(MAKE) "CFLAGS=$(STD_MACOSX_CFLAGS) $(OPT_CFLAGS)" "LDFLAGS=$(STD_MACOSX_LDFLAGS) $(OPT_LDFLAGS)"
endif

# Entries for other configurations.
linux:
	$(MAKE) "CFLAGS=$(STD_UNIX_CFLAGS) $(OPT_CFLAGS)" "LDFLAGS=$(STD_UNIX_LDFLAGS)" "STRIP_EXE=1" gmic_cli_standard

solaris:
	$(MAKE) "CFLAGS=$(STD_UNIX_CFLAGS) $(OPT_CFLAGS)" "LDFLAGS=$(STD_UNIX_LDFLAGS) -R$(USR)/X11R6/lib -lrt -lnsl -lsocket" "STRIP_EXE=1" gmic_cli_standard

macosx:
	$(MAKE) "CFLAGS=$(STD_MACOSX_CFLAGS) $(OPT_CFLAGS)" "LDFLAGS=$(STD_MACOSX_LDFLAGS) $(OPT_LDFLAGS)" gmic_cli_standard

windows:
	$(MAKE) "CFLAGS=$(STD_WINDOWS_CFLAGS) $(OPT_CFLAGS)" "LDFLAGS=$(STD_WINDOWS_LDFLAGS)" "STRIP_EXE=1" gmic_cli_standard

gmicol:
	$(MAKE) "CFLAGS+=$(GMICOL_UNIX_CFLAGS) $(OPT_CFLAGS)" "LDFLAGS+=$(GMICOL_UNIX_LDFLAGS)" gmic_cli_standard

static:
	$(MAKE) "CFLAGS+=$(STATIC_UNIX_CFLAGS) $(OPT_CFLAGS)" "LDFLAGS+=$(STATIC_UNIX_LDFLAGS)" gmic_cli_standard

minimal:
	$(MAKE) "CFLAGS+=$(MINIMAL_UNIX_CFLAGS)" "LDFLAGS+=$(MINIMAL_UNIX_LDFLAGS)" gmic_cli_standard

full:
	$(MAKE) "CFLAGS=$(FULL_UNIX_CFLAGS) $(OPT_CFLAGS)" "LDFLAGS=$(FULL_UNIX_LDFLAGS)" "STRIP_EXE=1" gmic_cli_full

winminimal:
	$(MAKE) "CFLAGS=$(MINIMAL_WINDOWS_CFLAGS)" "LDFLAGS=$(MINIMAL_WINDOWS_LDFLAGS)" gmic_cli_standard

# Internal rules to build compilation modules.
gmic_lib.o: gmic.cpp
	$(CC) -o gmic_lib.o -c gmic.cpp -fPIC $(CFLAGS) -Dgmic_float_only
gmic_lib: gmic_lib.o
	ar rcs libgmic.a gmic_lib.o
ifneq ($(OS),Darwin)
	$(CC) -shared -Wl,-soname,libgmic.so.1 -o libgmic.so gmic_lib.o $(LDFLAGS)
	$(CC) -o gmic_use_lib gmic_use_lib.cpp -L. -lgmic $(FFTW_LDFLAGS)
else
	$(CC) -shared -o libgmic.so gmic_lib.o $(LDFLAGS)
endif

gmic_gimp.o: gmic.cpp
	$(CC) -o gmic_gimp.o -c gmic.cpp $(CFLAGS) -Dgmic_gimp -Dgmic_float_only
gmic_gimp : gmic_gimp.o gmic_gimp.cpp
	$(CC) -o gmic_gimp gmic_gimp.cpp gmic_gimp.o `gimptool-2.0$(EXE) --cflags` $(CFLAGS) `gimptool-2.0$(EXE) --libs` $(LDFLAGS)
	strip gmic_gimp$(EXE)

gmic_bool.o: gmic.cpp
	$(CC) -o gmic_bool.o -c gmic.cpp $(CFLAGS) -Dgmic_split_compilation -Dgmic_bool
gmic_uchar.o: gmic.cpp
	$(CC) -o gmic_uchar.o -c gmic.cpp $(CFLAGS) -Dgmic_split_compilation -Dgmic_uchar
gmic_char.o: gmic.cpp
	$(CC) -o gmic_char.o -c gmic.cpp $(CFLAGS) -Dgmic_split_compilation -Dgmic_char
gmic_ushort.o: gmic.cpp
	$(CC) -o gmic_ushort.o -c gmic.cpp $(CFLAGS) -Dgmic_split_compilation -Dgmic_ushort
gmic_short.o: gmic.cpp
	$(CC) -o gmic_short.o -c gmic.cpp $(CFLAGS) -Dgmic_split_compilation -Dgmic_short
gmic_uint.o: gmic.cpp
	$(CC) -o gmic_uint.o -c gmic.cpp $(CFLAGS) -Dgmic_split_compilation -Dgmic_uint
gmic_int.o: gmic.cpp
	$(CC) -o gmic_int.o -c gmic.cpp $(CFLAGS) -Dgmic_split_compilation -Dgmic_int
gmic_float.o: gmic.cpp
	$(CC) -o gmic_float.o -c gmic.cpp $(CFLAGS) -Dgmic_split_compilation -Dgmic_float -Dgmic_main
gmic_double.o: gmic.cpp
	$(CC) -o gmic_double.o -c gmic.cpp $(CFLAGS) -Dgmic_split_compilation -Dgmic_double
gmic_cli_full: gmic_bool.o gmic_uchar.o gmic_char.o gmic_ushort.o gmic_short.o gmic_uint.o gmic_int.o gmic_float.o gmic_double.o
	$(CC) -o gmic gmic_bool.o gmic_uchar.o gmic_char.o gmic_ushort.o gmic_short.o gmic_uint.o gmic_int.o gmic_float.o gmic_double.o $(LDFLAGS)
	strip gmic$(EXE)
gmic_cli_standard: gmic.cpp
	$(CC) -o gmic gmic.cpp $(CFLAGS) -Dgmic_float_only -Dgmic_main $(LDFLAGS)

gmic_def: gmic_def.gmic
	@echo "#ifndef gmic_gimp" > gmic_def.h
	@\gmic gmic_def.gmic raw:gmic_def.gmic,char -compress_gmic -o -.h,char | sed 's/unnamed/gmic_def/' | sed 's/ \};/, 0 \};\n\nunsigned int size_data_gmic_def = sizeof(data_gmic_def);\n/g' >> gmic_def.h
	@echo "#else" >> gmic_def.h
	@\gmic gmic_def.gmic -update_gmic_gimp @.,0 raw:$(HOME)/.update@..gmic,char -compress_gmic_gimp -o -.h,char | sed 's/unnamed/gmic_def/' | sed 's/ \};/, 0 \};\n\nunsigned int size_data_gmic_def = sizeof(data_gmic_def);\n/g' >> gmic_def.h
	\gmic -v - ../html/img/logoGMIC.ppm -permute cxyz -o -.h,uchar | sed 's/unnamed/gmic_logo/' >> gmic_def.h
	@echo "unsigned int size_data_logo = sizeof(data_gmic_logo);" >> gmic_def.h
	@echo "#endif" >> gmic_def.h
	@echo >>gmic_def.h

# Install/uninstall/clean procedures.
install:
	mkdir -p $(DESTDIR)$(PLUGINDIR)/
	cp -f gmic_gimp $(DESTDIR)$(PLUGINDIR)/
	mkdir -p $(DESTDIR)$(USR)/bin/
	cp -f gmic $(DESTDIR)$(USR)/bin/
	mkdir -p $(DESTDIR)$(USR)/include/
	cp -f gmic.h $(DESTDIR)$(USR)/include/
ifneq ($(OS),Darwin)
	mkdir -p $(DESTDIR)$(USR)/share
	mkdir -p $(DESTDIR)$(USR)/lib
	cp -f libgmic.so $(DESTDIR)$(USR)/lib/libgmic.so.1.5.9
	ln -s libgmic.so.1.5.9 $(DESTDIR)$(USR)/lib/libgmic.so.1
	ln -s libgmic.so.1 $(DESTDIR)$(USR)/lib/libgmic.so
endif
	mkdir -p $(DESTDIR)$(USR)/share/man/
	mkdir -p $(DESTDIR)$(USR)/share/man/man1/
	mkdir -p $(DESTDIR)$(USR)/share/man/fr/man1/
	cp -f ../man/gmic.1.gz $(DESTDIR)$(USR)/share/man/man1/gmic.1.gz
	cp -f ../man/gmic.1.gz $(DESTDIR)$(USR)/share/man/fr/man1/gmic.1.gz
	if test -d /etc/bash_completion.d/; then mkdir -p $(DESTDIR)/etc/bash_completion.d/; cp -f ../resources/gmic_bashcompletion.sh $(DESTDIR)/etc/bash_completion.d/gmic; fi
	if test -d /opt/local/etc/bash_completion.d/; then mkdir -p $(DESTDIR)/opt/local/etc/bash_completion.d/; cp -f ../resources/gmic_bashcompletion.sh $(DESTDIR)/opt/local/etc/bash_completion.d/gmic; fi

uninstall:
	rm -f $(DESTDIR)$(USR)/bin/gmic
	rm -f $(DESTDIR)$(USR)/include/gmic.h
	rm -f $(DESTDIR)$(USR)/lib/libgmic.so.1.5.9
	rm -f $(DESTDIR)$(USR)/lib/libgmic.so.1
	rm -f $(DESTDIR)$(USR)/lib/libgmic.so
	rm -rf $(DESTDIR)$(USR)/share/doc/gmic/
	rm -f $(DESTDIR)$(USR)/share/man/man1/gmic.1.gz
	rm -f $(DESTDIR)$(USR)/share/man/fr/man1/gmic.1.gz

distclean: clean

clean:
	rm -rf gmic*.o gmic gmic_gimp gmic_use_lib libgmic* *~

# End of Makefile

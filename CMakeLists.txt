PROJECT(digikam)

OPTION(ENABLE_GPHOTO2 "Build digiKam with libgphoto2 support (default=ON)" ON)
OPTION(ENABLE_THEMEDESIGNER "Build digiKam theme designer (default=OFF)" OFF)

# ==================================================================================================
# Information to update before to release this package.

# digiKam version
SET(DIGIKAM_MAJOR_VERSION "0")
SET(DIGIKAM_MINOR_VERSION "11")
SET(DIGIKAM_PATCH_VERSION "0")

# Suffix to add at end of version string. Usual values are:
# "-svn"   : alpha code unstable from svn. Do not use in production
# "-beta1" : beta1 release.
# "-beta2" : beta2 release.
# "-beta3" : beta3 release.
# "-rc"    : release candidate.
# ""       : final release. Can be used in production.
SET(DIGIKAM_SUFFIX_VERSION "-svn")

# ==================================================================================================
# Set env. variables accordinly.

SET(DIGIKAM_VERSION_SHORT
    "${DIGIKAM_MAJOR_VERSION}.${DIGIKAM_MINOR_VERSION}.${DIGIKAM_PATCH_VERSION}"
   )

SET(DIGIKAM_VERSION_STRING
    "${DIGIKAM_VERSION_SHORT}${DIGIKAM_SUFFIX_VERSION}"
   )

# ==================================================================================================
# Depencies

FIND_PACKAGE(KDE4 REQUIRED)
INCLUDE(KDE4Defaults)
INCLUDE(MacroLibrary)
INCLUDE(MacroOptionalAddSubdirectory)

INCLUDE(FindPackageHandleStandardArgs)

MACRO_OPTIONAL_FIND_PACKAGE(LCMS)
MACRO_OPTIONAL_FIND_PACKAGE(JPEG)
MACRO_OPTIONAL_FIND_PACKAGE(PNG)
MACRO_OPTIONAL_FIND_PACKAGE(TIFF)
MACRO_OPTIONAL_FIND_PACKAGE(Jasper)
MACRO_OPTIONAL_FIND_PACKAGE(Kdcraw)
MACRO_OPTIONAL_FIND_PACKAGE(Kexiv2)
MACRO_OPTIONAL_FIND_PACKAGE(Kipi)

IF(ENABLE_GPHOTO2)
    MACRO_OPTIONAL_FIND_PACKAGE(Gphoto2)
ENDIF(ENABLE_GPHOTO2)

# Check KDcraw version installed to compile fine.
IF(NOT WIN32)
    FIND_PACKAGE(PkgConfig)
    PKG_CHECK_MODULES(Kdcraw libkdcraw>=0.4.0)
    IF(Kdcraw_FOUND)
        SET(KDCRAW_FOR_DIGIKAM TRUE)
    ELSE(Kdcraw_FOUND)
        SET(KDCRAW_FOR_DIGIKAM FALSE)
    ENDIF(Kdcraw_FOUND)
ELSE(NOT WIN32)
    SET(KDCRAW_FOR_DIGIKAM TRUE)
ENDIF(NOT WIN32)

# Check KExiv2 version installed to compile fine.
IF(NOT WIN32)
    FIND_PACKAGE(PkgConfig)
    PKG_CHECK_MODULES(Kexiv2 libkexiv2>=0.5.0)
    IF(Kexiv2_FOUND)
        SET(KEXIV2_FOR_DIGIKAM TRUE)
    ELSE(Kexiv2_FOUND)
        SET(KEXIV2_FOR_DIGIKAM FALSE)
    ENDIF(Kexiv2_FOUND)
ELSE(NOT WIN32)
    SET(KEXIV2_FOR_DIGIKAM TRUE)
ENDIF(NOT WIN32)

# ==================================================================================================
# Optional depencies

MACRO_OPTIONAL_FIND_PACKAGE(KdepimLibs)
MACRO_BOOL_TO_01(KDEPIMLIBS_FOUND HAVE_KDEPIMLIBS)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} )

# Check MarbleWidget library for geolocation.
MACRO_OPTIONAL_FIND_PACKAGE(MarbleWidget)
MACRO_BOOL_TO_01(MARBLEWIDGET_FOUND HAVE_MARBLEWIDGET)

# Check LensFun library for lens correction tool.
MACRO_OPTIONAL_FIND_PACKAGE(LensFun)
MACRO_BOOL_TO_01(LENSFUN_FOUND HAVE_LENSFUN)

# Check GLib2 library for liquid rescale tool.
MACRO_OPTIONAL_FIND_PACKAGE(GLIB2)

# Set config file depending of optional depencies

CONFIGURE_FILE(config-digikam.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-digikam.h)

# ==================================================================================================
# Log messages

MESSAGE(STATUS "")
MESSAGE(STATUS "----------------------------------------------------------------------------------")
MESSAGE(STATUS " digiKam ${DIGIKAM_VERSION_STRING} dependencies results   <http://www.digikam.org>")
MESSAGE(STATUS "")

IF(QT_QTSQL_FOUND)
    MESSAGE(STATUS " Qt4 SQL module found................ YES")
ELSE(QT_QTSQL_FOUND)
    MESSAGE(STATUS " Qt4 SQL module found................ NO")
    MESSAGE(STATUS "")
    MESSAGE(SEND_ERROR "digiKam needs the SQL module of Qt4")
    MESSAGE(STATUS "You need to install a package containing the module with support for sqlite3.")
    MESSAGE(STATUS "")
ENDIF(QT_QTSQL_FOUND)

IF(JPEG_FOUND)
    MESSAGE(STATUS " libjpeg library found............... YES")
ELSE(JPEG_FOUND)
    MESSAGE(STATUS " libjpeg library found............... NO")
    MESSAGE(STATUS "")
    MESSAGE(SEND_ERROR " digiKam needs libjpeg. You need to install the libjpeg development package.")
    MESSAGE(STATUS " libjpeg website is at http://www.ijg.org")
    MESSAGE(STATUS "")
ENDIF(JPEG_FOUND)

IF(TIFF_FOUND)
    MESSAGE(STATUS " libtiff library found............... YES")
ELSE(TIFF_FOUND)
    MESSAGE(STATUS " libtiff library found............... NO")
    MESSAGE(STATUS "")
    MESSAGE(SEND_ERROR " digiKam needs libtiff. You need to install the libtiff development package.")
    MESSAGE(STATUS " libtiff website is at http://www.remotesensing.org/libtiff")
    MESSAGE(STATUS "")
ENDIF(TIFF_FOUND)

IF(PNG_FOUND)
    MESSAGE(STATUS " libpng library found................ YES")
ELSE(PNG_FOUND)
    MESSAGE(STATUS " libpng library found................ NO")
    MESSAGE(STATUS "")
    MESSAGE(SEND_ERROR " digiKam needs libpng. You need to install the correct version (>= 1.2.7).")
    MESSAGE(STATUS " Libpng website is at http://www.libpng.org/pub/png/libpng.html")
    MESSAGE(STATUS "")
ENDIF(PNG_FOUND)

IF(JASPER_FOUND)
    MESSAGE(STATUS " libjasper library found............. YES")
ELSE(JASPER_FOUND)
    MESSAGE(STATUS " libjasper library found............. NO")
    MESSAGE(STATUS "")
    MESSAGE(SEND_ERROR " digiKam needs libjasper to support JPEG2000. You need to install the correct version (>= 1.7.0).")
    MESSAGE(STATUS " Libjasper website is at http://www.ece.uvic.ca/~mdadams/jasper")
    MESSAGE(STATUS "")
ENDIF(JASPER_FOUND)

IF(LCMS_FOUND)
    MESSAGE(STATUS " liblcms library found............... YES")
ELSE(LCMS_FOUND)
    MESSAGE(STATUS " liblcms library found............... NO")
    MESSAGE(STATUS "")
    MESSAGE(SEND_ERROR " digiKam needs Lcms library. You need to install the lcms development package.")
    MESSAGE(STATUS " Lcms website is at http://www.littlecms.com")
    MESSAGE(STATUS "")
ENDIF(LCMS_FOUND)

IF(KIPI_FOUND)
    MESSAGE(STATUS " libkipi library found............... YES")
ELSE(KIPI_FOUND)
    MESSAGE(STATUS " libkipi library found............... NO")
    MESSAGE(STATUS "")
    MESSAGE(SEND_ERROR " digiKam needs libkipi library >= 0.2.0. You need to install libkipi first")
    MESSAGE(STATUS " libkipi website is at http://www.digikam.org/sharedlibs")
    MESSAGE(STATUS "")
ENDIF(KIPI_FOUND)

IF(KEXIV2_FOR_DIGIKAM)
    MESSAGE(STATUS " libkexiv2 library found............. YES")
ELSE(KEXIV2_FOR_DIGIKAM)
    MESSAGE(STATUS " libkexiv2 library found............. NO")
    MESSAGE(STATUS "")
    MESSAGE(SEND_ERROR " digiKam needs libkexiv2 library >= 0.5.0. You need to install libkexiv2 first")
    MESSAGE(STATUS " libkexiv2 website is at http://www.digikam.org/sharedlibs")
    MESSAGE(STATUS "")
ENDIF(KEXIV2_FOR_DIGIKAM)

IF(KDCRAW_FOR_DIGIKAM)
    MESSAGE(STATUS " libkdcraw library found............. YES")
ELSE(KDCRAW_FOR_DIGIKAM)
    MESSAGE(STATUS " libkdcraw library found............. NO")
    MESSAGE(STATUS "")
    MESSAGE(SEND_ERROR " digiKam needs libkdcraw library >= 0.4.0. You need to install libkdcraw first")
    MESSAGE(STATUS " libkdcraw website is at http://www.digikam.org/sharedlibs")
    MESSAGE(STATUS "")
ENDIF(KDCRAW_FOR_DIGIKAM)

IF(ENABLE_GPHOTO2)
    IF(GPHOTO2_FOUND)
        EXEC_PROGRAM(gphoto2-config ARGS --version RETURN_VALUE _return_VALUE OUTPUT_VARIABLE GPHOTO2_VERSION)
        STRING(REPLACE "libgphoto2" "" GPHOTO2_VERSION "${GPHOTO2_VERSION}")
        MACRO_ENSURE_VERSION("2.4.0" "${GPHOTO2_VERSION}" VERSION_GPHOTO2)
        IF(VERSION_GPHOTO2)
            MESSAGE(STATUS " libgphoto2 library found............ YES")
        ELSE(VERSION_GPHOTO2)
            MESSAGE(STATUS " libgphoto2 library found............ NO")
            MESSAGE(STATUS " Install libgphoto2 > 2.4.0")
            MESSAGE(STATUS " Or disable GPhoto2 support using CMake flag")
            SET(GPHOTO2_FOUND false)
        ENDIF(VERSION_GPHOTO2)
    ELSE(GPHOTO2_FOUND)
        MESSAGE(STATUS " libgphoto2 library found............ NO")
        MESSAGE(STATUS "")
        MESSAGE(WARNING " digiKam needs libgphoto2. You need to install libgphoto2 first")
        MESSAGE(STATUS " libgphoto2 website is at http://www.gphoto.org")
        MESSAGE(STATUS "")
    ENDIF(GPHOTO2_FOUND)
    MACRO_LOG_FEATURE(GPHOTO2_FOUND "libgphoto2" "libgphoto2 is needed to build digikam" "http://www.gphoto.org/" FALSE "" "")
ELSE(ENABLE_GPHOTO2)
    MESSAGE(STATUS "")
    MESSAGE(STATUS " digiKam will be compiled without GPhoto2 support.")
    MESSAGE(STATUS "")
ENDIF(ENABLE_GPHOTO2)

IF(KDEPIMLIBS_FOUND)
    MESSAGE(STATUS " libkdepimlibs library found......... YES (optional)")
    INCLUDE_DIRECTORIES(${KDEPIMLIBS_INCLUDE_DIR})    
ELSE(KDEPIMLIBS_FOUND)
    MESSAGE(STATUS " libkdepimlibs library found......... NO  (optional)")
    MESSAGE(STATUS "")
    MESSAGE(STATUS " digiKam will be compiled without address book support.")
    MESSAGE(STATUS "")
ENDIF(KDEPIMLIBS_FOUND)

IF(MARBLEWIDGET_FOUND)
    MESSAGE(STATUS " libmarblewidget library found....... YES (optional)")
ELSE(MARBLEWIDGET_FOUND)
    MESSAGE(STATUS " libmarblewidget library found....... NO  (optional)")
    MESSAGE(STATUS "")
    MESSAGE(STATUS " digiKam will be compiled without geolocation using Marble widget support.")
    MESSAGE(STATUS "")
ENDIF(MARBLEWIDGET_FOUND)

IF(LENSFUN_FOUND)
    MESSAGE(STATUS " liblensfun library found............ YES (optional)")
ELSE(LENSFUN_FOUND)
    MESSAGE(STATUS " liblensfun library found............ NO  (optional)")
    MESSAGE(STATUS "")
    MESSAGE(STATUS " digiKam will be compiled without lens auto-correction image editor tool.")
    MESSAGE(STATUS "")
ENDIF(LENSFUN_FOUND)

IF(GLIB2_FOUND)
    MESSAGE(STATUS " libglib2 library found.............. YES (optional)")
ELSE(GLIB2_FOUND)
    MESSAGE(STATUS " libglib2 library found.............. NO  (optional)")
    MESSAGE(STATUS "")
    MESSAGE(STATUS " digiKam will be compiled without liquid rescale image editor tool.")
    MESSAGE(STATUS "")
ENDIF(GLIB2_FOUND)


IF(LCMS_FOUND AND JPEG_FOUND AND PNG_FOUND AND TIFF_FOUND AND JASPER_FOUND AND KDCRAW_FOR_DIGIKAM AND KEXIV2_FOUND AND KIPI_FOUND)
    IF(ENABLE_GPHOTO2)
        IF(GPHOTO2_FOUND)
            MESSAGE(STATUS " digiKam will be compiled............ YES")
            SET(DIGIKAM_CAN_BE_COMPILED true)
        ELSE(GPHOTO2_FOUND)
            MESSAGE(ERROR " digiKam will be compiled............ NO  (Look README file for more details about dependencies)")
            SET(DIGIKAM_CAN_BE_COMPILED false)
        ENDIF(GPHOTO2_FOUND)
    ELSE(ENABLE_GPHOTO2)
        MESSAGE(STATUS " digiKam will be compiled............ YES")
        SET(DIGIKAM_CAN_BE_COMPILED true)
    ENDIF(ENABLE_GPHOTO2)
ELSE(LCMS_FOUND AND JPEG_FOUND AND PNG_FOUND AND TIFF_FOUND AND AND JASPER_FOUND AND KDCRAW_FOR_DIGIKAM AND KEXIV2_FOUND AND KIPI_FOUND)
    MESSAGE(FATAL_ERROR " digiKam will be compiled............ NO  (Look README file for more details about dependencies)")
    SET(DIGIKAM_CAN_BE_COMPILED false)
ENDIF(LCMS_FOUND AND JPEG_FOUND AND PNG_FOUND AND TIFF_FOUND AND JASPER_FOUND AND KDCRAW_FOR_DIGIKAM AND KEXIV2_FOUND AND KIPI_FOUND)

MESSAGE(STATUS "----------------------------------------------------------------------------------")
MESSAGE(STATUS "")

IF(DIGIKAM_CAN_BE_COMPILED)

    # ==================================================================================================
    # Create svn version header

    # See http://public.kitware.com/pipermail/cmake/2006-July/010299.html
    # We only do this IF we are in a .svn dir

    FIND_FILE(SVN_MARKER entries PATHS ${CMAKE_SOURCE_DIR}/.svn)

    IF(NOT SVN_MARKER)
        SET (SVN_MARKER ${CMAKE_SOURCE_DIR}/CMakeLists.txt)  # Dummy file
    ENDIF(NOT SVN_MARKER)

    # Add a custom command to drive the svn script whenever the svn entries
    # file changes.
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/svnscript.cmake.in_cmake"
                   "${CMAKE_CURRENT_BINARY_DIR}/svnscript.cmake"
                   @ONLY)

    ADD_CUSTOM_COMMAND(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/digikam/svnversion.h"
                       DEPENDS ${SVN_MARKER}
                       COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/svnscript.cmake")

    # Add a custom target to drive the custom command.
    ADD_CUSTOM_TARGET(digikam-svnversion
                      ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/digikam/svnversion.h")

    # ==================================================================================================
    # get the gcc version

    # CImg.h version 1.2.8 do not compile fine with gcc 4.3.x
    # See B.K.O #163118: digikam-0.9.4_beta5 compilation hangs with gcc 4.3
    # Using -fno-tree-pre is work around this problem.

    IF(CMAKE_COMPILER_IS_GNUCXX)
        EXEC_PROGRAM(${CMAKE_CXX_COMPILER} ARGS --version OUTPUT_VARIABLE VERSION_GCC_INFO)
        STRING(REGEX MATCH " [34]\\.[0-9]\\.[0-9]" GCC_VERSION "${VERSION_GCC_INFO}")

        # gcc on mac just reports: "gcc (GCC) 3.3 20030304 ..." without the patch level, handle this here:
        IF(NOT GCC_VERSION)
            STRING(REGEX REPLACE ".*\\(GCC\\).* ([34]\\.[0-9]) .*" "\\1.0" GCC_VERSION "${VERSION_GCC_INFO}")
        ENDIF (NOT GCC_VERSION)
        
        MACRO_ENSURE_VERSION("4.3.0" "${GCC_VERSION}" GCC_VERSION_BAD)

        IF(GCC_VERSION_BAD)
            MESSAGE(STATUS "Adjusting compilation flags for GCC version (${GCC_VERSION} )")
            ADD_DEFINITIONS(-fno-tree-pre)
        ENDIF(GCC_VERSION_BAD)
    ENDIF(CMAKE_COMPILER_IS_GNUCXX)

    # ==================================================================================================
    # Definitions and includes rules

    ADD_DEFINITIONS(${QT_DEFINITIONS}
                    ${KDE4_DEFINITIONS}
                    ${KDCRAW_DEFINITIONS}
                    ${KEXIV2_DEFINITIONS}
                   )

    # QT3 support rules
    ADD_DEFINITIONS(-DQT3_SUPPORT
                    -DQT3_SUPPORT_WARNINGS
                   )

    # Enable C++ Exceptions support, require by Greycstoration algorithm (CImg.h)
    IF(NOT MSVC)
        ADD_DEFINITIONS(${KDE4_ENABLE_EXCEPTIONS})
    ENDIF(NOT MSVC)

    INCLUDE_DIRECTORIES(${KIPI_INCLUDE_DIR}
                        ${KEXIV2_INCLUDE_DIR}
                        ${KDCRAW_INCLUDE_DIR}
                        ${KDE4_INCLUDES}
                        ${QT_INCLUDES}
                        ${GPHOTO2_INCLUDE_DIRS}
                        ${CMAKE_CURRENT_BINARY_DIR}/digikam
                       )

    IF(MARBLEWIDGET_FOUND)
        INCLUDE_DIRECTORIES(${LIBMARBLEWIDGET_INCLUDE_DIR})
    ENDIF(MARBLEWIDGET_FOUND)

    # ==================================================================================================
    # Common targets parts

    SET(libcurves_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/curves/imagecurves.cpp
       )

    SET(libdimg_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dimg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dimgscale.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dcolor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dcolorcomposer.cpp
       )

    SET(libdimgloaders_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/dimgloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/pngloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/jpegloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/tiffloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/rawloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/ppmloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/qimageloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/iccjpeg.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/jp2kloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/jpegsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/pngsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/tiffsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/jp2ksettings.cpp
       )

	SET(libclapack_SRCS 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/abort_.c
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/dgesv.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/dlaswp.c
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/endfile.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/idamax.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/open.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/sig_die.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/wref.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/close.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/dgetf2.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/dscal.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/err.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/ieeeck.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/s_cmp.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/s_stop.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/wrtfmt.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/dgemm.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/dgetrf.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/dswap.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/fmt.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/ilaenv.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/s_copy.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/wsfe.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/dger.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/dgetrs.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/dtrsm.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/fmtlib.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/lsame.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/sfe.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/util.c 
    	${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/clapack/xerbla.c
       )

    SET(libdimgfilters_SRCS
        ${libclapack_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/bcgmodifier.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/hslmodifier.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/icctransform.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/dimgimagefilters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/colormodifier.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/dimggaussianblur.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/dimgsharpen.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/dimgthreadedfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/dimgunsharpmask.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/dimgrefocus.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/matrix.cpp
       )

    SET(libdmetadata_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/photoinfocontainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/dmetadata.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/geodetictools.cpp
       )

    # CImg.h version 1.2.8 do not compile fine with gcc 4.3.
    # See B.K.O #163118: digikam-0.9.4_beta5 compilation hangs with gcc 4.3
    # This is a temporary solution until CImg.h will be fixed.
    KDE4_NO_ENABLE_FINAL(libgreycstoration)
    SET(libgreycstoration_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/greycstoration/greycstorationiface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/greycstoration/greycstorationwidget.cpp
       )

    SET_SOURCE_FILES_PROPERTIES(${libgreycstoration_SRCS} PROPERTIES COMPILE_FLAGS ${KDE4_ENABLE_EXCEPTIONS})

    SET(libhistogram_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/histogram/imagehistogram.cpp
       )

    SET(libimageproperties_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiessidebar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiestab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiesmetadatatab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiescolorstab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiesgpstab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/worldmapwidget.cpp
       )

    # --enable-final triggers: http://bugs.kde.org/show_bug.cgi?id=126326
    # digikam: camera download: auto-rotated images lose EXIF info ...
    # So make sure nofinal is always used here!
    KDE4_NO_ENABLE_FINAL(libjpegutils)
    SET(libjpegutils_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/jpegutils/jpegutils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/jpegutils/transupp.cpp
       )
IF(MSVC)
    SET(libjpegutils_SRCS
        ${libjpegutils_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/jpegutils/jpegwin.cpp
       )
ENDIF(MSVC)

    SET(liblevels_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/levels/imagelevels.cpp
       )

    # Gilles Caulier 12/01/06: lprof implementation is C not C++ and do not
    # support 'nofinal' compilation option.
    KDE4_NO_ENABLE_FINAL(liblprof)
    SET(liblprof_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmshull.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmslm.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmslnr.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmsmatn.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmsmkmsh.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmsmntr.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmsoutl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmspcoll.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmsprf.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmsreg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmsscn.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmssheet.cpp
       )

    SET(libthreadimageio_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/loadsavethread.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/managedloadsavethread.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/sharedloadsavethread.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/loadingdescription.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/loadingcache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/loadingcacheinterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/loadsavetask.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/previewloadthread.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/previewtask.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/thumbnailbasic.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/thumbnailcreator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/thumbnailloadthread.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/thumbnailtask.cpp
       )

    SET(libwhitebalance_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/whitebalance/whitebalance.cpp
       )

    SET(libcommonwidgets_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/clicklabel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/colorgradientwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/comboboxutilities.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dcursortracker.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dlogoaction.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dfontselect.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dpopupmenu.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dgradientslider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/drubberband.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/ditemtooltip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/filesaveoptionsbox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/itemviewhoverbutton.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/itemviewtooltip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/statuszoombar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/statusprogressbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/statusnavigatebar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/statusled.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/sidebar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/paniconwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/previewwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/curveswidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/histogramwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/histogrambox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/splashscreen.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/searchtextbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/thumbbartooltip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/thumbbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/visibilitycontroller.cpp
       )

    SET(libiccprofileswidgets_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/iccprofiles/iccpreviewwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/iccprofiles/iccprofilewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/iccprofiles/cietonguewidget.cpp
       )

    SET(libimagepluginswidgets_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/imageplugins/imagepaniconwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/imageplugins/imageregionwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/imageplugins/imageguidewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/imageplugins/imagewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/imageplugins/imagepanelwidget.cpp
       )

    SET(libmetadatawidgets_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/mdkeylistviewitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/metadatalistview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/metadatalistviewitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/iptcwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/exifwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/makernotewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/metadatawidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/xmpwidget.cpp
       )

    SET(libslideshow_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/slideshow/toolbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/slideshow/slideshow.cpp
       )

    SET(libsetupshowfoto_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupiofiles.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupdcraw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupicc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupslideshow.cpp
       )

    SET(libdimgcanvas_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/imageplugin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/imagepluginloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/undocache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/undoaction.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/undomanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/colorcorrectiondlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/dimginterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/canvas.cpp
       )

    SET(libdimgeditorprintiface_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/printiface/printhelper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/printiface/printoptionspage.cpp
       )

    SET(librawimport_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/rawimport/rawpostprocessing.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/rawimport/rawimport.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/rawimport/rawpreview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/rawimport/rawsettingsbox.cpp
       )

    SET(libthemeengine_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/themeengine/theme.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/themeengine/texture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/themeengine/themeengine.cpp
       )

    # ==================================================================================================
    # Showfoto target parts

    SET(libshowfotosetup_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/showfoto/setup/setuptooltip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/showfoto/setup/setupeditor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/showfoto/setup/setup.cpp
       )

    # ==================================================================================================
    # digiKam target parts

    SET(libdialogs_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/iccprofileinfodlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/dprogressdlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/imagedialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/rawcameradlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/libsinfodlg.cpp
       )

    SET(libdeletedialog_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/deletedialog.cpp
       )

    SET(libsetup_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupicc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupcollections.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupcollectionview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupcategory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupalbumview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setup.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setuptooltip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupidentity.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupmime.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupslideshow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupmetadata.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupeditor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupmisc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setuplighttable.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupiofiles.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupplugins.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupdcraw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupcamera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/cameraselection.cpp
       )

    SET(libkipiiface_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface/kipiimageinfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface/kipiimagecollection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface/kipiimagecollectionselector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface/kipiuploadwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface/kipiinterface.cpp
       )

    SET(libfirstrun_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/firstrun/assistantdlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/firstrun/assistantdlgpage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/firstrun/welcomepage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/firstrun/collectionpage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/firstrun/rawpage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/firstrun/tooltipspage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/firstrun/previewpage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/firstrun/openfilepage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/firstrun/metadatapage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/firstrun/startscanpage.cpp
       )

    SET(libimagepropertiesdigikam_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiessidebarcamgui.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiessidebardb.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/cameraitempropertiestab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagedescedittab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/talbumlistview.cpp
       )

    SET(libdimgeditor_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/imageiface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/editortool.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/editortooliface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/editorstackview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/editortoolsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/editorwindow.cpp
       )

    SET(libdimgimagewindow_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/imagewindow.cpp
       )

    SET(libbatch_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/batch/imageinfojob.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/batch/imageinfoalbumsjob.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/batch/batchthumbsgenerator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/batch/batchalbumssyncmetadata.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/batch/batchsyncmetadata.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/batch/fingerprintsgenerator.cpp
       )

    SET(libdatabase_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/albumdb.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/albumdb_sqlite2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/collectionscanner.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagescanner.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/collectionmanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/collectionlocation.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databaseaccess.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databasewatch.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databasebackend.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databaseparameters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databasetransaction.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databaseurl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databasechangesets.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/collectionscannerhints.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imageinfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imageinfolist.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imageinfocache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagelister.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagelisterreceiver.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagecomments.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imageposition.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagecopyright.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/downloadhistory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagequerybuilder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/searchxml.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/namefilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/schemaupdater.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/upgradedb_sqlite2tosqlite3.cpp
       )

    SET(libsqlite2_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/attach.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/auth.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/btree.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/btree_rb.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/build.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/copy.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/date.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/delete.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/encode.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/expr.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/func.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/hash.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/insert.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/main.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/opcodes.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/os.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/pager.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/parse.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/pragma.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/printf.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/random.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/select.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/shell.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/table.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/tokenize.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/trigger.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/update.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/util.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/vacuum.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/vdbe.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/vdbeaux.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/where.c
       )

    SET(libhaar_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/haar/haar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/haar/haariface.cpp
       )

    SET(libdatabasemodels_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagemodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagefiltermodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagefiltersettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagemodeldragdrophandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagethumbnailmodel.cpp
       )

    SET(libalbummodels_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagealbummodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagealbumfiltermodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/abstractalbummodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/albummodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/albumfiltermodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/albummodeldragdrophandler.cpp
       )

    SET(liblighttable_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/lighttable/lighttablepreview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/lighttable/lighttableview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/lighttable/lighttablebar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/lighttable/lighttablewindow.cpp
       )

    SET(libqueuemanager_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/actionthread.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/assignedlist.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/toolsview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/toolslistview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/toolsettingsview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/queuemgrwindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/queuepool.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/queuelist.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/queuetooltip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/queuesettingsview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/batchtoolsmanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/batchtool.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/convert/convert2jpeg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/convert/convert2png.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/convert/convert2tiff.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/convert/convert2jp2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/enhance/autocorrection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/enhance/restoration.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/enhance/sharpen.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/transform/rotate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/transform/flip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/transform/resize.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/decorate/watermark.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/metadata/metadata.cpp
       )

    SET(libcameragui_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/dkcamera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/umscamera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/gpcamera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/gpiteminfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/renamecustomizer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/camerainfodialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/camerafolderdialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/camerafolderitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/camerafolderview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/cameraiconitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/cameraiconview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/cameraiconviewtooltip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/cameracontroller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/cameraui.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/freespacewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/freespacetooltip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/capturewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/capturedlg.cpp
       )

    SET(libsearchwindow_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/searchwindow/searchwindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/searchwindow/searchview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/searchwindow/searchgroup.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/searchwindow/searchfieldgroup.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/searchwindow/searchfields.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/searchwindow/searchutilities.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/searchwindow/ratingsearchutilities.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/searchwindow/choicesearchutilities.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/searchwindow/searchfolderview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/searchwindow/searchtabheader.cpp
       )

    SET(libfuzzysearch_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/fuzzysearch/fuzzysearchfolderview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/fuzzysearch/fuzzysearchview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/fuzzysearch/findduplicatesview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/fuzzysearch/findduplicatesalbumitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/fuzzysearch/sketchwidget.cpp
       )

    SET(libgpssearch_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/gpssearch/gpssearchfolderview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/gpssearch/gpssearchview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/gpssearch/gpssearchwidget.cpp
       )

    SET(libtimeline_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/timeline/timelinefolderview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/timeline/timelineview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/timeline/timelinewidget.cpp
       )

    SET(libdigikamgui_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/album.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/albumdragdrop.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/albumiconviewfilter.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/albumfolderview.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/albumhistory.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/albumlister.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/albummanager.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/albumpropsedit.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/albumselectcombobox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/albumselectwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/albumselectdialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/albumsettings.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/albumthumbnailloader.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/albumtreeview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/albumwidgetstack.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/cameralist.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/cameratype.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/contextmenuhelper.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/datefolderview.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/digikamapp.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/digikamimageview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/digikamview.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/dio.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/ddragobjects.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/folderitem.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/folderview.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/folderviewtooltip.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/iconview.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/icongroupitem.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/iconitem.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/imageattributeswatch.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/imagecategorizedview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/imagecategorydrawer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/imagedelegate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/imagedelegateoverlay.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/imagedragdrop.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/imagepreviewview.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/imagepreviewbar.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/imageviewutilities.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/imageselectionoverlay.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/kcategorizedview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/kdateedit.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/kdatetimeedit.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/kdatepickerpopup.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/mediaplayerview.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/metadatahub.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/metadatamanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/mimefilter.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/monthwidget.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/ratingbox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/ratingfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/ratingpopupmenu.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/ratingwidget.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/scancontroller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/syncjob.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/tageditdlg.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/tagfolderview.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/tagfilterview.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/tooltipfiller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/treefolderitem.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/tagspopupmenu.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/welcomepageview.cpp 
       )

    ADD_SUBDIRECTORY(data)
    ADD_SUBDIRECTORY(libs)
    ADD_SUBDIRECTORY(utilities)
    ADD_SUBDIRECTORY(digikam)
    ADD_SUBDIRECTORY(kioslave)
    ADD_SUBDIRECTORY(showfoto)
    ADD_SUBDIRECTORY(imageplugins)

    IF(ENABLE_THEMEDESIGNER)
        ADD_SUBDIRECTORY(themedesigner)
    ENDIF(ENABLE_THEMEDESIGNER)

    INSTALL(FILES tips DESTINATION ${DATA_INSTALL_DIR}/digikam)

    IF(NOT WIN32)
        CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/digikam.lsm.cmake ${CMAKE_CURRENT_BINARY_DIR}/digikam.lsm)
    ENDIF(NOT WIN32)

ENDIF(DIGIKAM_CAN_BE_COMPILED)

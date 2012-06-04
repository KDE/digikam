
PROJECT(digikam)

MESSAGE(STATUS "----------------------------------------------------------------------------------")
MESSAGE(STATUS "Starting CMake configuration for: digiKam")

OPTION(ENABLE_SCRIPTIFACE      "Build digiKam with experimental scripting support (default=OFF)"  OFF)
OPTION(ENABLE_PRESENTATIONMODE "Build digiKam with presentation mode support (default=OFF)"       OFF)
OPTION(ENABLE_THUMBS_DB        "Build digiKam with thumbnails database support (default=ON)"      ON)
OPTION(ENABLE_INTERNALMYSQL    "Look for MySQL server executable at build time (default=ON)"      ON)
OPTION(ENABLE_ADVANCEDDEBUGMSG "Build digiKam with more internal debug message (default=OFF)"     OFF)
OPTION(ENABLE_NEPOMUKSUPPORT   "Build digiKam with Nepomuk support (default=ON)"                  ON)
OPTION(ENABLE_LCMS2            "Link digiKam to LCMS2 instead LCMS1 (experimental) (default=OFF)" OFF)

# ==================================================================================================
# Information to update before to release this package.

# digiKam version
SET(DIGIKAM_MAJOR_VERSION "2")
SET(DIGIKAM_MINOR_VERSION "7")
SET(DIGIKAM_PATCH_VERSION "0")

#digiKam release date
SET(DIGIKAM_RELEASE_DATE "2012-07-08")

# Suffix to add at end of version string. Usual values are:
# "-git"   : alpha code unstable from git. Do not use in production
# "-beta1" : beta1 release.
# "-beta2" : beta2 release.
# "-beta3" : beta3 release.
# "-rc"    : release candidate.
# ""       : final release. Can be used in production.
SET(DIGIKAM_SUFFIX_VERSION "")

# ==================================================================================================
# Set env. variables accordingly.

SET(DIGIKAM_VERSION_SHORT
    "${DIGIKAM_MAJOR_VERSION}.${DIGIKAM_MINOR_VERSION}.${DIGIKAM_PATCH_VERSION}"
   )

SET(DIGIKAM_VERSION_STRING
    "${DIGIKAM_VERSION_SHORT}${DIGIKAM_SUFFIX_VERSION}"
   )

# Path to find FindPGF.cmake script.
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# ==================================================================================================
# Debug area codes
# These codes will be set for every project specific path automatically, so
# that you can use kDebug() without any usage of the area code throughout the
# digiKam code base.
SET(AREA_CODE_GENERAL        "50003")
SET(AREA_CODE_KIOSLAVES      "50004")
SET(AREA_CODE_SHOWFOTO       "50005")
SET(AREA_CODE_IMAGEPLUGINS   "50006")
SET(AREA_CODE_DATABASESERVER "50007")
ADD_DEFINITIONS(-DKDE_DEFAULT_DEBUG_AREA=${AREA_CODE_GENERAL})
ADD_DEFINITIONS(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

MACRO(SET_DEBUG_AREA_CODE _areacode)
    REMOVE_DEFINITIONS(-DKDE_DEFAULT_DEBUG_AREA=${AREA_CODE_GENERAL})
    ADD_DEFINITIONS(-DKDE_DEFAULT_DEBUG_AREA=${_areacode})
ENDMACRO(SET_DEBUG_AREA_CODE)

# =======================================================
# some useful macros for printing status information

SET(PRINT_COMPILE_LENGTH "40")
MACRO(FILL_WITH_DOTS VAR)

    STRING(LENGTH ${${VAR}} NAME_LENGTH)

    MATH(EXPR DOT_LENGTH "${PRINT_COMPILE_LENGTH} - ${NAME_LENGTH}")
    IF(${DOT_LENGTH} LESS 0)
        SET(DOT_LENGTH 0)
    ENDIF(${DOT_LENGTH} LESS 0)

    FOREACH(COUNT RANGE ${DOT_LENGTH})
        SET(${VAR} "${${VAR}}.")
    ENDFOREACH(COUNT)

ENDMACRO(FILL_WITH_DOTS)

MACRO(PRINT_LIBRARY_STATUS NAME WEBSITE VERSIONHINT)

    SET(LIB_MESSAGE "${NAME} library found")
    FILL_WITH_DOTS(LIB_MESSAGE)

    IF(${ARGN})
        MESSAGE(STATUS " ${LIB_MESSAGE} YES")
    ELSE(${ARGN})
        MESSAGE(STATUS " ${LIB_MESSAGE} NO")
        MESSAGE(STATUS "")
        MESSAGE(SEND_ERROR " digiKam needs ${NAME}. You need to install the ${NAME}${VERSIONHINT} library development package.")
        MESSAGE(STATUS " ${NAME} website is at ${WEBSITE}")
        MESSAGE(STATUS "")
    ENDIF(${ARGN})

ENDMACRO(PRINT_LIBRARY_STATUS)

MACRO(PRINT_QTMODULE_STATUS NAME)

    SET(LIB_MESSAGE "${NAME} module found")
    FILL_WITH_DOTS(LIB_MESSAGE)

    IF(${ARGN})
        MESSAGE(STATUS " ${LIB_MESSAGE} YES")
    ELSE(${ARGN})
        MESSAGE(STATUS " ${LIB_MESSAGE} NO")
        MESSAGE(STATUS "")
        MESSAGE(SEND_ERROR " digiKam needs ${NAME} module. You need to install a package containing the ${NAME} module.")
        MESSAGE(STATUS "")
    ENDIF(${ARGN})

ENDMACRO(PRINT_QTMODULE_STATUS)

MACRO(PRINT_EXECUTABLE_STATUS NAME TECHNICAL_NAME PATH_VARIABLE)

    SET(LIB_MESSAGE "${NAME} found")
    FILL_WITH_DOTS(LIB_MESSAGE)

    IF(${ARGN})
        MESSAGE(STATUS " ${LIB_MESSAGE} YES")
    ELSE(${ARGN})
        MESSAGE(STATUS " ${LIB_MESSAGE} NO")
        MESSAGE(STATUS "")
        MESSAGE(STATUS " digiKam needs ${NAME}. You need to install the package containing the \"${TECHNICAL_NAME}\" executable.")
        MESSAGE(STATUS " If you have this executable installed, please specify the folder containing it by ${PATH_VARIABLE}")
        MESSAGE(SEND_ERROR "")
    ENDIF(${ARGN})

ENDMACRO(PRINT_EXECUTABLE_STATUS)

# ==================================================================================================
# Dependencies

FIND_PACKAGE(KDE4 REQUIRED)
INCLUDE(KDE4Defaults)
INCLUDE(MacroLibrary)

INCLUDE(FindPackageHandleStandardArgs)
INCLUDE(FindPkgConfig)
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} )

FIND_PACKAGE(PkgConfig)
FIND_PACKAGE(JPEG)
FIND_PACKAGE(PNG)
FIND_PACKAGE(TIFF)
FIND_PACKAGE(Jasper)
FIND_PACKAGE(Kdcraw)
FIND_PACKAGE(Kexiv2)
FIND_PACKAGE(Kipi)
FIND_PACKAGE(Kface)
FIND_PACKAGE(KGeoMap)
FIND_PACKAGE(PGF)
FIND_PACKAGE(CLAPACK)
FIND_PACKAGE(Boost) # 1.36.0)

# LCMS version 1 and 2 library handling
# See B.K.O #294105
# At the moment (2.6) we still DEFAULT to LCMS1 because porting to LCMS2 still experimental.
# It's possible to force to use LCMS2 by switch on ENABLE_LCMS2 option
# Future plans:
#   2.7 will compile against lcms2 by default but lcms1 will stay as a fallback
#   2.8 will support lcms2 only, probably ENABLE_LCMS2 will be removed too
#       unless we decide to support another color managment library
# Note: if ENABLE_LCMS2 is on and no LCMS2 is available, build will fail, even if LCMS1 is present
IF (ENABLE_LCMS2)
    MESSAGE(STATUS "Check for LCMS2 availability...")
    FIND_PACKAGE(LCMS2)
    IF (LCMS2_FOUND AND (LCMS2_VERSION VERSION_EQUAL 2.1 OR LCMS2_VERSION VERSION_GREATER 2.1))
        MESSAGE(STATUS "Found LCMS2: ${LCMS2_LIBRARIES} ${LCMS2_INCLUDE_DIR}")
        SET(LCMS_FOUND ${LCMS2_FOUND})
        SET(LCMS_LIBRARIES ${LCMS2_LIBRARIES})
        SET(LCMS_INCLUDE_DIR ${LCMS2_INCLUDE_DIR})
        SET(USE_LCMS_VERSION_2000 1)
    ENDIF (LCMS2_FOUND AND (LCMS2_VERSION VERSION_EQUAL 2.1 OR LCMS2_VERSION VERSION_GREATER 2.1))
ELSE (ENABLE_LCMS2)
    MESSAGE(STATUS "Check for LCMS1 availability...")
    FIND_PACKAGE(LCMS)
    IF (LCMS_FOUND)
        MESSAGE(STATUS "Found LCMS1: ${LCMS_LIBRARIES} ${LCMS_INCLUDE_DIR}")
        SET(USE_LCMS_VERSION_1000 1)
    ELSE (LCMS_FOUND)
        MESSAGE(STATUS "Check for LCMS2 availability instead LCMS1...")
        FIND_PACKAGE(LCMS2)
        IF (LCMS2_FOUND AND (LCMS2_VERSION VERSION_EQUAL 2.1 OR LCMS2_VERSION VERSION_GREATER 2.1))
            MESSAGE(STATUS "Found LCMS2 instead LCMS1: ${LCMS2_LIBRARIES} ${LCMS2_INCLUDE_DIR}")
            MESSAGE(WARNING "Support of LCMS2 still experimental")
            MESSAGE(WARNING "Please install LCMS1 if possible")
            SET(LCMS_FOUND ${LCMS2_FOUND})
            SET(LCMS_LIBRARIES ${LCMS2_LIBRARIES})
            SET(LCMS_INCLUDE_DIR ${LCMS2_INCLUDE_DIR})
            SET(USE_LCMS_VERSION_2000 1)
        ENDIF (LCMS2_FOUND AND (LCMS2_VERSION VERSION_EQUAL 2.1 OR LCMS2_VERSION VERSION_GREATER 2.1))
    ENDIF (LCMS_FOUND)
ENDIF (ENABLE_LCMS2)

# To link under Solaris (see B.K.O #274484)
IF(NOT WIN32)
    FIND_LIBRARY(MATH_LIBRARY m)
ENDIF(NOT WIN32)

IF(Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
ENDIF(Boost_FOUND)

#-------------------------------------------------------------------------------------

MACRO_BOOL_TO_01(PGF_FOUND USE_EXT_LIBPGF)

# NOTE: currently libpgf 6.11.24 do not support very well OpenMP. We disable it temporally.
# See B.K.O #273765 for details.
ADD_DEFINITIONS(-DLIBPGF_DISABLE_OPENMP)

# LibPGF need to be linked to OpenMP if necessary.
# OpenMP detection (requires CMake >= 2.6.3)
# NOTE: OpenMP under MacOSX do not support multithreading.

IF(NOT APPLE AND NOT USE_EXT_LIBPGF)
    MACRO_OPTIONAL_FIND_PACKAGE(OpenMP)

    IF(OPENMP_FOUND)
        # OpenMP defintions ${OpenMP_CXX_FLAGS} is set on right digiKam components through sub-cmake scripts.
        IF("${OpenMP_CXX_FLAGS}" STREQUAL "-fopenmp" )
            SET(OPENMP_LDFLAGS "-lgomp")
        ENDIF("${OpenMP_CXX_FLAGS}" STREQUAL "-fopenmp" )
        IF("${OpenMP_CXX_FLAGS}" STREQUAL "-xopenmp" )
            SET(OPENMP_LDFLAGS "-xopenmp")
        ENDIF("${OpenMP_CXX_FLAGS}" STREQUAL "-xopenmp" )
    ENDIF(OPENMP_FOUND)
ENDIF(NOT APPLE AND NOT USE_EXT_LIBPGF)

#-------------------------------------------------------------------------------------

IF(NOT APPLE)
    MACRO_BOOL_TO_01(CLAPACK_FOUND USE_EXT_LIBCLAPACK)
ELSE(NOT APPLE)
    # Under MACOS-X Clapack exist in standard, but it's not suitable as well. Need investiguation here.
    SET(CLAPACK_FOUND FALSE)
ENDIF(NOT APPLE)

#-------------------------------------------------------------------------------------

# check version of libjpeg so that we can use the appropriate dir
# See bug #227313
SET(_jpeglib_version_source "#include <stddef.h>\n#include<stdio.h>\n#include <jpeglib.h>\n int main()\n {\n #if (JPEG_LIB_VERSION >= 80) \n #error JPEG_LIB_VERSION >= 80 found \n #endif \n }\n")
SET(_jpeglib_version_source_file ${CMAKE_BINARY_DIR}/CMakeTmp/cmake_jpeglib_version_check.cpp)
FILE(WRITE "${_jpeglib_version_source_file}" "${_jpeglib_version_source}")
SET(_jpeglib_version_include_dirs "-DINCLUDE_DIRECTORIES:STRING=${JPEG_INCLUDE_DIR}")

TRY_COMPILE(_jpeglib_version_compile_result ${CMAKE_BINARY_DIR} ${_jpeglib_version_source_file}
            CMAKE_FLAGS "${_jpeglib_version_include_dirs}"
            COMPILE_OUTPUT_VARIABLE _jpeglib_version_compile_output_var)

IF(_jpeglib_version_compile_result)
    MESSAGE(STATUS "Identified libjpeg version: 62")
    SET(DIGIKAM_LIBJPEG_DIR libjpeg-62)
ELSE(_jpeglib_version_compile_result)
    MESSAGE(STATUS "Identified libjpeg version: 80+")
    SET(DIGIKAM_LIBJPEG_DIR libjpeg-80)
ENDIF(_jpeglib_version_compile_result)

#-------------------------------------------------------------------------------------

IF (ENABLE_INTERNALMYSQL)
# Check MySQL executables
    FIND_PROGRAM( SERVERCMD_MYSQL mysqld
                  PATHS /usr/sbin /usr/local/sbin /usr/libexec /usr/local/libexec /opt/local/libexec
                        /opt/mysql/libexec /usr/mysql/bin /opt/mysql/sbin ${MYSQLD_PATH}
                        ENV MYSQLD_PATH )
    MESSAGE(STATUS "Found MySQL server executable at: ${SERVERCMD_MYSQL}")

# NOTE: under windows, mysql_install_db PERL script is named "mysql_install_db.pl"
#       under MACOS-X, through macports project, mysql_install_db PERL script is named "mysql_install_db5"
    FIND_PROGRAM( INITCMD_MYSQL mysql_install_db mysql_install_db5 mysql_install_db.pl
                  PATHS /usr/sbin /usr/local/sbin /usr/libexec /usr/local/libexec
                        /opt/mysql/libexec /usr/mysql/bin /opt/mysql/bin ${MYSQL_TOOLS_PATH}
                       ENV MYSQL_TOOLS_PATH )
    MESSAGE(STATUS "Found MySQL install_db executable at: ${INITCMD_MYSQL}")
    SET(DATABASESERVERDIR "${CMAKE_CURRENT_SOURCE_DIR}/databaseserver")
ELSE (ENABLE_INTERNALMYSQL)
    SET(DATABASESERVERDIR "")
ENDIF (ENABLE_INTERNALMYSQL)
MACRO_BOOL_TO_01(ENABLE_INTERNALMYSQL HAVE_INTERNALMYSQL)

# check the libkdcraw version
# note that old versions don't specify version information, so for now
# just inform the user if version information was found
SET(KDCRAW_MIN_VERSION "2.0.0")
IF (KDCRAW_VERSION)
  MESSAGE(STATUS "libkdcraw: Found version ${KDCRAW_VERSION} (required: ${KDCRAW_MIN_VERSION})")
  IF (${KDCRAW_VERSION} VERSION_LESS ${KDCRAW_MIN_VERSION})
    SET(KDCRAW_FOUND FALSE)
  ENDIF (${KDCRAW_VERSION} VERSION_LESS ${KDCRAW_MIN_VERSION})
ELSE (KDCRAW_VERSION)
  MESSAGE(WARNING "libkdcraw: Version information not found, your version is probably too old.")
ENDIF (KDCRAW_VERSION)

# check the libkgeomap version
# note that old versions don't specify version information, so for now
# just inform the user if version information was found
IF (KGEOMAP_VERSION)
  MESSAGE(STATUS "libkgeomap: Found version ${KGEOMAP_VERSION}")
  # TODO: fail if the version is too low
ENDIF (KGEOMAP_VERSION)

# ==================================================================================================
# Optional dependencies

MACRO_OPTIONAL_FIND_PACKAGE(Gphoto2)
MACRO_BOOL_TO_01(GPHOTO2_FOUND HAVE_GPHOTO2)

IF (NOT WIN32 AND HAVE_GPHOTO2)
    # See B.K.O #268267 : digiKam need to be linked to libusb to prevent crash at gphoto2 init if opencv is linked with libdc1394
    MACRO_OPTIONAL_FIND_PACKAGE(USB)
ENDIF (NOT WIN32 AND HAVE_GPHOTO2)

MACRO_OPTIONAL_FIND_PACKAGE(KdepimLibs)
MACRO_BOOL_TO_01(KDEPIMLIBS_FOUND HAVE_KDEPIMLIBS)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} )

# Check GLib2 library for lrqr-1 and lensfun libraries.
MACRO_OPTIONAL_FIND_PACKAGE(GLIB2)
MACRO_BOOL_TO_01(GLIB2_FOUND HAVE_GLIB2)

# Check LensFun library for Lens auto-correction tool.
MACRO_OPTIONAL_FIND_PACKAGE(LensFun)
MACRO_BOOL_TO_01(LENSFUN_FOUND USE_EXT_LIBLENSFUN)

# Check Lqr-1 library for Contents Aware Resizer tool.
MACRO_OPTIONAL_FIND_PACKAGE(Lqr-1)
MACRO_BOOL_TO_01(LQR-1_FOUND USE_EXT_LIBLQR-1)

MACRO_BOOL_TO_01(ENABLE_SCRIPTIFACE      USE_SCRIPT_IFACE)
MACRO_BOOL_TO_01(ENABLE_PRESENTATIONMODE USE_PRESENTATION_MODE)
MACRO_BOOL_TO_01(ENABLE_THUMBS_DB        USE_THUMBS_DB)
MACRO_BOOL_TO_01(ENABLE_ADVANCEDDEBUGMSG USE_ADVANCEDDEBUGMSG)

# Win32 compilation with Nepomuk is broken. It's temporally disabled.
IF (ENABLE_NEPOMUKSUPPORT)
    IF (${KDE_VERSION} VERSION_GREATER "4.3.99")
        MACRO_OPTIONAL_FIND_PACKAGE(Nepomuk)
        MACRO_LOG_FEATURE(NEPOMUK_FOUND "Nepomuk" "The Nepomuk libraries"
                "http://www.kde.org" FALSE "" "Nepomuk extends the search and tagging")

        MACRO_OPTIONAL_FIND_PACKAGE(SharedDesktopOntologies 0.2)
        MACRO_LOG_FEATURE(SHAREDDESKTOPONTOLOGIES_FOUND "Shared desktop ontologies" "Desktop ontologies"
            "http://oscaf.sourceforge.net" TRUE "0.2" "Ontologies necessary for the Nepomuk semantic desktop.")

        IF (NEPOMUK_FOUND AND SHAREDDESKTOPONTOLOGIES_FOUND)
            SET(NEPOMUK_FOUND TRUE)
        ELSE (NEPOMUK_FOUND AND SHAREDDESKTOPONTOLOGIES_FOUND)
            SET(NEPOMUK_FOUND FALSE)
        ENDIF (NEPOMUK_FOUND AND SHAREDDESKTOPONTOLOGIES_FOUND)

        MACRO_BOOL_TO_01(NEPOMUK_FOUND HAVE_NEPOMUK)
    ENDIF (${KDE_VERSION} VERSION_GREATER "4.3.99")
ENDIF (ENABLE_NEPOMUKSUPPORT)

MACRO_OPTIONAL_FIND_PACKAGE(Doxygen)

# Set config files accordingly with optional dependencies

CONFIGURE_FILE(digikam/utils/config-digikam.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/digikam/utils/config-digikam.h)

# ==================================================================================================
# Log messages

MESSAGE(STATUS "")
MESSAGE(STATUS "----------------------------------------------------------------------------------")
MESSAGE(STATUS " digiKam ${DIGIKAM_VERSION_STRING} dependencies results   <http://www.digikam.org>")
MESSAGE(STATUS "")

PRINT_QTMODULE_STATUS("Qt4 SQL" QT_QTSQL_FOUND)

IF(USE_SCRIPT_IFACE)
    PRINT_QTMODULE_STATUS("Qt4 SCRIPT" QT_QTSCRIPT_FOUND)
    PRINT_QTMODULE_STATUS("Qt4 SCRIPTTOOLS" QT_QTSCRIPTTOOLS_FOUND)
ENDIF(USE_SCRIPT_IFACE)

IF (ENABLE_INTERNALMYSQL)
    PRINT_EXECUTABLE_STATUS("MySQL Server" mysqld MYSQLD_PATH SERVERCMD_MYSQL)
    IF (NOT WIN32)
        PRINT_EXECUTABLE_STATUS("MySQL install_db tool" mysql_install_db MYSQL_TOOLS_PATH INITCMD_MYSQL)
    ENDIF (NOT WIN32)
ELSE (ENABLE_INTERNALMYSQL)
    MESSAGE(STATUS " mysql server found..................... NO  (optional - disabled by user)")
ENDIF (ENABLE_INTERNALMYSQL)

PRINT_LIBRARY_STATUS("libtiff" "http://www.remotesensing.org/libtiff" "" TIFF_FOUND)
PRINT_LIBRARY_STATUS("libpng" "http://www.libpng.org/pub/png/libpng.html" " (version >= 1.2.7)" PNG_FOUND)
PRINT_LIBRARY_STATUS("libjasper" "http://www.ece.uvic.ca/~mdadams/jasper" "" JASPER_FOUND)
PRINT_LIBRARY_STATUS("liblcms" "http://www.littlecms.com" "" LCMS_FOUND)
PRINT_LIBRARY_STATUS("Boost Graph" "http://www.boost.org/doc/libs/1_43_0/libs/graph/doc/index.html" " (headers only)" Boost_FOUND)
PRINT_LIBRARY_STATUS("libkipi" "http://www.digikam.org/sharedlibs" " (version >= 1.2.0)" KIPI_FOUND)
PRINT_LIBRARY_STATUS("libkexiv2" "http://www.digikam.org/sharedlibs" " (version >= 1.2.0)" KEXIV2_FOUND)
PRINT_LIBRARY_STATUS("libkdcraw" "http://www.digikam.org/sharedlibs" " (version >= ${KDCRAW_MIN_VERSION})" KDCRAW_FOUND)
PRINT_LIBRARY_STATUS("libkface" "http://www.digikam.org/sharedlibs" " (version >= 0.1.0)" KFACE_FOUND)
PRINT_LIBRARY_STATUS("libkgeomap" "http://www.digikam.org/sharedlibs" " (version >= 0.1.0)" KGEOMAP_FOUND)

IF(PGF_FOUND)
    MESSAGE(STATUS " libpgf library found..................... YES (optional)")
ELSE(PGF_FOUND)
    MESSAGE(STATUS " libpgf library found..................... NO  (optional - internal version used instead)")
ENDIF(PGF_FOUND)

IF(CLAPACK_FOUND)
    MESSAGE(STATUS " libclapack library found................. YES (optional)")
ELSE(CLAPACK_FOUND)
    MESSAGE(STATUS " libclapack library found................. NO  (optional - internal version used instead)")
ENDIF(CLAPACK_FOUND)

# ================================================================

IF(GPHOTO2_FOUND)
    EXEC_PROGRAM(gphoto2-config ARGS --version RETURN_VALUE _return_VALUE OUTPUT_VARIABLE GPHOTO2_VERSION)
    STRING(REPLACE "libgphoto2" "" GPHOTO2_VERSION "${GPHOTO2_VERSION}")
    MACRO_ENSURE_VERSION("2.4.0" "${GPHOTO2_VERSION}" VERSION_GPHOTO2)
    IF(VERSION_GPHOTO2 AND LIBUSB_FOUND)
        SET(GPHOTO2_FOUND true)
    ELSE(VERSION_GPHOTO2 AND LIBUSB_FOUND)
        SET(GPHOTO2_FOUND false)
    ENDIF(VERSION_GPHOTO2 AND LIBUSB_FOUND)
ENDIF(GPHOTO2_FOUND)

IF(GPHOTO2_FOUND)
    MESSAGE(STATUS " libgphoto2 and libusb libraries found.... YES (optional)")
ELSE(GPHOTO2_FOUND)
    MESSAGE(STATUS " libgphoto2 and libusb libraries found.... NO  (optional)")
    MESSAGE(STATUS "")
    MESSAGE(STATUS " digiKam will be compiled without GPhoto2 support.")
    MESSAGE(STATUS "")
ENDIF(GPHOTO2_FOUND)

# ================================================================

IF(KDEPIMLIBS_FOUND)
    MESSAGE(STATUS " libkdepimlibs library found.............. YES (optional)")
    INCLUDE_DIRECTORIES(${KDEPIMLIBS_INCLUDE_DIR})
ELSE(KDEPIMLIBS_FOUND)
    MESSAGE(STATUS " libkdepimlibs library found.............. NO  (optional)")
    MESSAGE(STATUS "")
    MESSAGE(STATUS " digiKam will be compiled without address book support.")
    MESSAGE(STATUS "")
ENDIF(KDEPIMLIBS_FOUND)

IF(NEPOMUK_FOUND)
    MESSAGE(STATUS " Nepomuk libraries found.................. YES (optional)")
ELSE(NEPOMUK_FOUND)
    IF (${KDE_VERSION} VERSION_GREATER "4.4.0")
        MESSAGE(STATUS " Nepomuk libraries found.................. NO  (optional)")
    ELSE (${KDE_VERSION} VERSION_GREATER "4.4.0")
        MESSAGE(STATUS " Nepomuk libraries found.................. NO  (at least KDE 4.4 and shared-desktop-ontologies required)")
    ENDIF (${KDE_VERSION} VERSION_GREATER "4.4.0")
    MESSAGE(STATUS "")
    MESSAGE(STATUS " digiKam will be compiled without support for the Nepomuk semantic desktop services.")
    MESSAGE(STATUS "")
ENDIF(NEPOMUK_FOUND)

IF(GLIB2_FOUND)
    MESSAGE(STATUS " libglib2 library found................... YES (optional)")
    IF(LQR-1_FOUND)
        MESSAGE(STATUS " liblqr-1 library found................... YES (optional)")
    ELSE(LQR-1_FOUND)
        MESSAGE(STATUS " liblqr-1 library found................... NO  (optional - internal version used instead)")
    ENDIF(LQR-1_FOUND)

    IF(LENSFUN_FOUND)
        MESSAGE(STATUS " liblensfun library found................. YES (optional)")
    ELSE(LENSFUN_FOUND)
        MESSAGE(STATUS " liblensfun library found................. NO  (optional - internal version used instead)")
    ENDIF(LENSFUN_FOUND)

ELSE(GLIB2_FOUND)
    MESSAGE(STATUS " libglib2 library found................... NO  (optional)")
    MESSAGE(STATUS "")
    MESSAGE(STATUS " digiKam will be compiled without liquid rescale and lens auto-correction tools.")
    MESSAGE(STATUS "")
ENDIF(GLIB2_FOUND)

IF(DOXYGEN_FOUND)
    MESSAGE(STATUS " Doxygen found............................ YES (optional)")
ELSE(DOXYGEN_FOUND)
    MESSAGE(STATUS " Doxygen found............................ NO  (optional)")
    MESSAGE(STATUS "")
    MESSAGE(STATUS " You will not be able to to generate the API documentation.")
    MESSAGE(STATUS " This does not affect building digiKam in any way.")
    MESSAGE(STATUS "")

ENDIF(DOXYGEN_FOUND)

IF(LCMS_FOUND AND JPEG_FOUND AND PNG_FOUND AND TIFF_FOUND AND Boost_FOUND AND JASPER_FOUND AND KDCRAW_FOUND AND KEXIV2_FOUND AND KIPI_FOUND AND KFACE_FOUND AND KGEOMAP_FOUND)
    MESSAGE(STATUS " digiKam can be compiled.................. YES")
    SET(DIGIKAM_CAN_BE_COMPILED true)
ELSE(LCMS_FOUND AND JPEG_FOUND AND PNG_FOUND AND TIFF_FOUND AND Boost_FOUND AND JASPER_FOUND AND KDCRAW_FOUND AND KEXIV2_FOUND AND KIPI_FOUND AND KFACE_FOUND AND KGEOMAP_FOUND)
    MESSAGE(FATAL_ERROR " digiKam will be compiled................. NO  (Look README file for more details about dependencies)")
    SET(DIGIKAM_CAN_BE_COMPILED false)
ENDIF(LCMS_FOUND AND JPEG_FOUND AND PNG_FOUND AND TIFF_FOUND AND Boost_FOUND AND JASPER_FOUND AND KDCRAW_FOUND AND KEXIV2_FOUND AND KIPI_FOUND AND KFACE_FOUND AND KGEOMAP_FOUND)

IF(DIGIKAM_CAN_BE_COMPILED AND FORCED_UNBUNDLE AND ((NOT PGF_FOUND) OR (NOT CLAPACK_FOUND) OR (NOT LENSFUN_FOUND)))
    MESSAGE(STATUS "")
    MESSAGE(FATAL_ERROR " BUNDLED LIBRARIES HAVE BEEN FORCE-DISABLED IN THIS BUILD. Aborting.")
    SET(DIGIKAM_CAN_BE_COMPILED false)
ENDIF(DIGIKAM_CAN_BE_COMPILED AND FORCED_UNBUNDLE AND ((NOT PGF_FOUND) OR (NOT CLAPACK_FOUND) OR (NOT LENSFUN_FOUND)))

MESSAGE(STATUS "----------------------------------------------------------------------------------")
MESSAGE(STATUS "")

IF(DIGIKAM_CAN_BE_COMPILED)

    # ==================================================================================================
    # Create git version header

    # We only do this IF we are in a .git dir
    FIND_FILE(GIT_MARKER entries PATHS ${CMAKE_SOURCE_DIR}/.git)

    IF(NOT GIT_MARKER)
        SET (GIT_MARKER ${CMAKE_SOURCE_DIR}/CMakeLists.txt)  # Dummy file
    ENDIF(NOT GIT_MARKER)

    # Add a custom command to drive the git script whenever the git entries
    # file changes.
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/gitscript.cmake.in_cmake"
                   "${CMAKE_CURRENT_BINARY_DIR}/gitscript.cmake"
                   @ONLY)

    # Add a custom target to drive the custom command.
    ADD_CUSTOM_TARGET(digikam-gitversion ALL COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/gitscript.cmake")

    # ==================================================================================================
    # get the gcc version

    # CImg.h version 1.2.8 do not compile fine with gcc 4.3.x
    # See B.K.O #163118: digikam-0.9.4_beta5 compilation hangs with gcc 4.3
    # Using -fno-tree-pre is work around this problem.

    IF(CMAKE_COMPILER_IS_GNUCXX)
        EXEC_PROGRAM(${CMAKE_CXX_COMPILER} ARGS ${CMAKE_CXX_COMPILER_ARG1} --version OUTPUT_VARIABLE VERSION_GCC_INFO)
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
    # To prevent warnings from M$ compiler

    IF(WIN32 AND MSVC)
        ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
        ADD_DEFINITIONS(-D_ATL_SECURE_NO_WARNINGS)
        ADD_DEFINITIONS(-D_AFX_SECURE_NO_WARNINGS)
    ENDIF(WIN32 AND MSVC)

    # ==================================================================================================
    # Definitions and includes rules

    ADD_DEFINITIONS(${QT_DEFINITIONS}
                    ${KDE4_DEFINITIONS}
                    ${KDCRAW_DEFINITIONS}
                    ${KEXIV2_DEFINITIONS}
                    ${KFACE_DEFINITIONS}
                    ${KGEOMAP_DEFINITIONS}
                   )

    # QT3 support rules
    ADD_DEFINITIONS(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

    # Enable C++ Exceptions support, require by Greycstoration algorithm (CImg.h)
    IF(NOT MSVC)
        ADD_DEFINITIONS(${KDE4_ENABLE_EXCEPTIONS})
    ENDIF(NOT MSVC)

    INCLUDE_DIRECTORIES(${KIPI_INCLUDE_DIR}
                        ${KEXIV2_INCLUDE_DIR}
                        ${KDCRAW_INCLUDE_DIR}
                        ${KFACE_INCLUDE_DIR}
                        ${KGEOMAP_INCLUDE_DIR}
                        ${KDE4_INCLUDES}
                        ${QT_INCLUDES}
                        ${GPHOTO2_INCLUDE_DIRS}
                        ${CMAKE_CURRENT_SOURCE_DIR}/digikam
                        ${CMAKE_CURRENT_BINARY_DIR}/digikam
    )

    IF(USE_SCRIPT_IFACE)
        INCLUDE_DIRECTORIES(${QT_QTSCRIPT_INCLUDE_DIR})
    ENDIF(USE_SCRIPT_IFACE)

    IF(USE_PRESENTATION_MODE)
        INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/utilities/qmlshow)
    ENDIF(USE_PRESENTATION_MODE)

    IF(NEPOMUK_FOUND)
        INCLUDE_DIRECTORIES(${SOPRANO_INCLUDE_DIR} ${NEPOMUK_INCLUDE_DIR})
    ENDIF(NEPOMUK_FOUND)

    IF(PGF_FOUND)
       INCLUDE_DIRECTORIES(${PGF_INCLUDE_DIRS})
    ELSE(PGF_FOUND)
       INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/libpgf)
       SET(libpgf_SRCS
          ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/libpgf/Decoder.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/libpgf/Encoder.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/libpgf/PGFimage.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/libpgf/PGFstream.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/libpgf/Subband.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/libpgf/WaveletTransform.cpp
       )
    ENDIF(PGF_FOUND)

    IF(CLAPACK_FOUND)
        INCLUDE_DIRECTORIES(${CLAPACK_INCLUDE_DIR})
    ELSE(CLAPACK_FOUND)
        INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack)
        SET(libclapack_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/abort_.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/dgesv.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/dlaswp.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/endfile.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/idamax.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/open.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/sig_die.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/wref.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/close.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/dgetf2.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/dscal.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/err.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/ieeeck.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/s_cmp.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/s_stop.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/wrtfmt.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/dgemm.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/dgetrf.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/dswap.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/fmt.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/ilaenv.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/s_copy.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/wsfe.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/dger.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/dgetrs.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/dtrsm.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/fmtlib.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/lsame.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/sfe.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/util.c
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/clapack/xerbla.c
        )
    ENDIF(CLAPACK_FOUND)

    # ==================================================================================================
    # Lensfun and LibLqr-1 library rules
    # NOTE: both libraries needs GLIB2.

    IF(GLIB2_FOUND)

        INCLUDE_DIRECTORIES(${GLIB2_INCLUDE_DIR})

        IF(LQR-1_FOUND)

            INCLUDE_DIRECTORIES(${LQR-1_INCLUDE_DIRS})
            SET(liblqr_LIBS ${LQR-1_LIBRARIES})

        ELSE(LQR-1_FOUND)

            INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty)
            INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lqr)

            SET(liblqr_SRCS
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lqr/lqr_carver_bias.c
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lqr/lqr_carver.c
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lqr/lqr_carver_list.c
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lqr/lqr_carver_rigmask.c
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lqr/lqr_cursor.c
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lqr/lqr_gradient.c
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lqr/lqr_progress.c
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lqr/lqr_vmap.c
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lqr/lqr_vmap_list.c
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lqr/lqr_energy.c
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lqr/lqr_rwindow.c
            )

            IF(WIN32)
                ADD_DEFINITIONS(-DLQR_DISABLE_DECLSPEC)
            ENDIF(WIN32)

        ENDIF(LQR-1_FOUND)

        IF(LENSFUN_FOUND)

            INCLUDE_DIRECTORIES(${LENSFUN_INCLUDE_DIR})
            SET(liblensfun_LIBS ${LENSFUN_LIBRARY})

        ELSE(LENSFUN_FOUND)

            ADD_DEFINITIONS(-DTRE_REGEX_T_FIELD=value -DTRE_VERSION="0")

            INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/libs/3rdparty/lensfun)
            INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lensfun)

            IF(WIN32)
                INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/regex)
            ENDIF(WIN32)

            SET(liblensfun_SRCS
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lensfun/config-lensfun.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lensfun/auxfun.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lensfun/camera.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lensfun/cpuid.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lensfun/database.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lensfun/lens.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lensfun/mod-color.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lensfun/mod-coord.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lensfun/mod-subpix.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lensfun/modifier.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/lensfun/mount.cpp
               )

            # We only need regex with lensfun code under windows where this library do not exist.
            IF(WIN32)
                SET(liblensfun_SRCS ${liblensfun_SRCS}
                    ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/regex/regcomp.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/regex/regerror.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/regex/regexec.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/regex/tre-ast.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/regex/tre-compile.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/regex/tre-match-approx.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/regex/tre-match-backtrack.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/regex/tre-match-parallel.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/regex/tre-mem.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/regex/tre-parse.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/regex/tre-stack.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/regex/xmalloc.c
                   )
            ENDIF(WIN32)

        ENDIF(LENSFUN_FOUND)

    ENDIF(GLIB2_FOUND)

    # ==================================================================================================
    # Common targets parts

    SET(libdimg_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dimg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/drawdecoding.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dimgscale.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dcolor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dcolorcomposer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/imagehistory/dimagehistory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/imagehistory/filteraction.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/imagehistory/historyimageid.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/pgfloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/jpegsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/pngsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/tiffsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/jp2ksettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/pgfsettings.cpp
       )

    SET(libdimgfilters_SRCS
        ${libclapack_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dklcms/digikam-lcms.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/dimgbuiltinfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/dimgthreadedfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/dimgfiltermanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/dimgfiltergenerator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/filteractionfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/randomnumbergenerator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/rawprocessingfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/decorate/borderfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/decorate/bordersettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/decorate/texturefilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/film/filmfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/fx/blurfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/fx/blurfxfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/fx/colorfxfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/fx/distortionfxfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/fx/charcoalfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/fx/embossfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/fx/filmgrainfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/fx/filmgrainsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/fx/invertfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/fx/pixelsaliasfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/fx/oilpaintfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/fx/raindropfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/auto/autolevelsfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/auto/autoexpofilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/auto/equalizefilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/auto/stretchfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/auto/normalizefilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/cb/cbfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/cb/cbsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/bcg/bcgfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/bcg/bcgsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/bw/bwsepiafilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/bw/bwsepiasettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/bw/tonalityfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/bw/infraredfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/bw/mixerfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/bw/mixersettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/hsl/hslfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/hsl/hslsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/hsl/hspreviewwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/icc/iccmanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/icc/iccprofile.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/icc/iccprofilesettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/icc/icctransform.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/icc/icctransformfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/icc/iccsettingscontainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/icc/iccsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lc/localcontrastfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lc/localcontrastsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lc/localcontrastcontainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/nr/nrfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/nr/nrsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/sharp/sharpenfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/sharp/unsharpmaskfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/sharp/refocusfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/sharp/matrix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/sharp/sharpsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/levels/imagelevels.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/levels/levelsfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/levels/imagehistogram.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/levels/histogrambox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/levels/histogramwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/levels/histogrampainter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/curves/curvescontainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/curves/imagecurves.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/curves/curvesfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/curves/curvessettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/curves/curveswidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/curves/curvesbox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/wb/wbfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/wb/wbsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/transform/freerotationfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/transform/freerotationsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/transform/shearfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/greycstoration/greycstorationfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/greycstoration/greycstorationsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lens/antivignettingfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lens/antivignettingsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lens/lensdistortionfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lens/pixelaccess.cpp
       )

    IF(GLIB2_FOUND)

        SET(libdimgfilters_SRCS ${libdimgfilters_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/transform/contentawarefilter.cpp
            ${liblqr_SRCS}
           )

        SET(libdimgfilters_SRCS ${libdimgfilters_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lens/lensfunfilter.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lens/lensfuncameraselector.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lens/lensfuniface.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lens/lensfunsettings.cpp
            ${liblensfun_SRCS}
           )

    ENDIF(GLIB2_FOUND)

    SET(libdmetadata_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/metadatasettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/metadatasettingscontainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/metadatainfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/photoinfocontainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/dmetadata.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/geodetictools.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/template.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/captionvalues.cpp
       )

    SET(libversionmanager_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/versionmanager/versionmanager.cpp
       )

    SET(libimageproperties_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiessidebar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiestab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiesmetadatatab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiescolorstab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiesgpstab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagegpsmodelhelper.cpp
       )

    SET(libkgeomaphelper_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/kgeomaphelper/digikam2kgeomap.cpp
       )

    # --enable-final triggers: http://bugs.kde.org/show_bug.cgi?id=126326
    # digikam: camera download: auto-rotated images lose EXIF info ...
    # So make sure nofinal is always used here!
    KDE4_NO_ENABLE_FINAL(libjpegutils)
    SET(libjpegutils_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/jpegutils/jpegutils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/jpegutils/${DIGIKAM_LIBJPEG_DIR}/transupp.cpp
       )

    IF(MSVC)
    SET(libjpegutils_SRCS
            ${libjpegutils_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/jpegutils/jpegwin.cpp
           )
    ENDIF(MSVC)

    SET(libthreadimageio_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/filereadwritelock.cpp
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

    SET(libcommonwidgets_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/abstractitemdragdrophandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/buttonicondisabler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/colorgradientwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/comboboxutilities.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dcursortracker.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dfontselect.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dhistoryview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dgradientslider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/ditemtooltip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dlogoaction.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dragdropimplementations.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dzoombar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/filesaveoptionsbox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/knotificationwrapper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/modelcompletion.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/paniconwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/searchtextbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/sidebar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/splashscreen.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/statesavingobject.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/statusnavigatebar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/statusprogressbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/visibilitycontroller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/workingwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/ratingwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/colorlabelwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/picklabelwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/thumbbardock.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/q3support/previewwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/q3support/drubberband.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/q3support/thumbbar.cpp
       )

    SET(libitemview_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/itemview/digikamkcategorizedview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/itemview/dcategorizedview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/itemview/ditemdelegate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/itemview/imagedelegateoverlay.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/itemview/itemviewhoverbutton.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/itemview/itemviewimagedelegate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/itemview/itemviewtooltip.cpp
       )

    SET(libgraphicsviewwidgets_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/clickdragreleaseitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/dimgchilditem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/dimgpreviewitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/regionframeitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/graphicsdimgitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/graphicsdimgview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/imagezoomsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/previewlayout.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/itemvisibilitycontroller.cpp
       )

    SET(libiccprofileswidgets_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/iccprofiles/iccpreviewwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/iccprofiles/iccprofilewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/iccprofiles/cietonguewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/iccprofiles/iccprofilescombobox.cpp
       )

    SET(libmetadatawidgets_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/mdkeylistviewitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/metadatalistview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/metadatalistviewitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/metadatawidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/metadataselector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/metadatapanel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/iptcwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/exifwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/makernotewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/xmpwidget.cpp
       )

    IF(USE_SCRIPT_IFACE)

        SET(libscriptiface_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/scriptiface/scriptiface.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/scriptiface/scriptplugintype.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/scriptiface/setupscriptmanager.cpp
           )

    ENDIF(USE_SCRIPT_IFACE)

    IF(USE_PRESENTATION_MODE)

    SET(libpresentationview_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/qmlshow/qmlshow.cpp
       )

    ENDIF(USE_PRESENTATION_MODE)

    SET(libslideshow_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/slideshow/toolbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/slideshow/slideshow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/slideshow/slideshowsettings.cpp
       )

    SET(libsetupshowfoto_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupeditor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupiofiles.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupdcraw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupicc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupslideshow.cpp
       )

    SET(libeditorwidgets_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/widgets/imageregionwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/widgets/imageguidewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/widgets/imagepreviewitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/widgets/previewtoolbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/widgets/previewlist.cpp
       )

    SET(libdimgcanvas_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/imageplugin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/imagepluginloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/undocache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/undoaction.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/undomanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/colorcorrectiondlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/softproofdialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/dimginterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/iccpostloadingmanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/canvas.cpp
       )

    SET(libdimgeditorprintiface_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/printiface/printhelper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/printiface/printoptionspage.cpp
       )

    SET(librawimport_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/rawimport/rawimport.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/rawimport/rawpreview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/rawimport/rawsettingsbox.cpp
       )

    SET(libprogressmanager_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/progressmanager/overlaywidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/progressmanager/progressview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/progressmanager/progressmanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/progressmanager/statusbarprogresswidget.cpp
       )

    SET(libthememanager_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/thememanager/thememanager.cpp
       )

    SET(libtthread_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threads/threadmanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threads/workerobject.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threads/dynamicthread.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threads/parallelworkers.cpp
       )

    # ==================================================================================================
    # Showfoto target parts

    SET(libshowfotosetup_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/showfoto/setup/setuptooltip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/showfoto/setup/setupmisc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/showfoto/setup/setupmetadata.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/showfoto/setup/setup.cpp
       )

    # ==================================================================================================
    # digiKam target parts

    SET(libtemplate_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/template/templatemanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/template/templatelist.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/template/templateselector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/template/templateviewer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/template/templatepanel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/template/subjectedit.cpp
       )

    SET(libdialogs_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/dprogressdlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/dbusydlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/filesaveoptionsdlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/iccprofileinfodlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/imagedialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/infodlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/libsinfodlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/rawcameradlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/triplechoicedialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/daboutdata.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setuptemplate.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/cameralist.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/cameratype.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupdatabase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupfacetags.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupversioning.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/importfilters.cpp
       )

    SET(libkipiiface_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface/kipipluginloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface/kipiimageinfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface/kipiimagecollection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface/kipiimagecollectionselector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface/kipiuploadwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface/kipiinterface.cpp
       )

    SET(libnepomukservices_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/nepomuk/digikamnepomukservice.cpp
       )

    SET(libfacedetection_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/facedetection/faceiface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/facedetection/facegroup.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/facedetection/facepipeline.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/facedetection/facescandialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/facedetection/assignnamewidget.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/captionedit.cpp
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

    SET(libmaintenance_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/imageinfojob.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/imageinfoalbumsjob.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/metadatasynchronizer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/duplicatesfinder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/newitemsfinder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/facedetector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/thumbsgenerator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/fingerprintsgenerator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/maintenancedlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/maintenancemngr.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/maintenancetool.cpp
       )

    SET(libdatabasecore_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/core/databasecorebackend.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/core/databaseparameters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/core/databaseerrorhandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/core/thumbnaildatabaseaccess.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/core/thumbnaildb.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/core/thumbnailschemaupdater.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/core/databaseconfigelement.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/core/sqlquery.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/core/dbactiontype.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/core/tagregion.cpp
       )

    # We must set this variable here at top level because it is used in both
    # digikam/database and data/database
    SET(DBCONFIG_XML_VERSION "1")

    SET(libdatabase_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/albumdb.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/albumdb_sqlite2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/collectionscanner.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/collectionmanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/collectionlocation.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/collectionscannerhints.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databaseaccess.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databasewatch.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databasebackend.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databaseoperationgroup.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databasetransaction.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databaseurl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databasechangesets.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databaseconstants.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databasethumbnailinfoprovider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/downloadhistory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagecomments.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imageextendedproperties.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imageinfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imageinfolist.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imageinfocache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagelister.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagelisterreceiver.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imageposition.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagecopyright.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagequerybuilder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagescanner.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagetagpair.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/namefilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/searchxml.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/schemaupdater.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/tagproperties.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/upgradedb_sqlite2tosqlite3.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databasechecker.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databasecopymanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databaseserverstarter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/tagscache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databaseservererror.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagehistory/imagehistorygraph.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagehistory/imagehistorygraphmodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/faces/databaseface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/faces/facetags.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/faces/facetagseditor.cpp
       )

    SET(libdatabaseui_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/databasewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/migrationdlg.cpp
       )

    SET(pgfutils_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/pgfutils.cpp
       )

    SET(libsqlite2_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/attach.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/auth.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/btree.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/btree_rb.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/build.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/copy.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/date.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/delete.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/encode.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/expr.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/func.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/hash.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/insert.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/main.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/opcodes.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/os.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/pager.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/parse.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/pragma.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/printf.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/random.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/select.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/shell.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/table.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/tokenize.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/trigger.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/update.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/util.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/vacuum.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/vdbe.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/vdbeaux.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2/where.c
       )

    SET(libkde3rdparty_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/kmemoryinfo/kmemoryinfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/kinotify/kinotify.cpp
      )

    SET(libhaar_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/haar/haar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/haar/haariface.cpp
       )

    SET(libgenericmodels_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/categorizeditemmodel.cpp
       )

    SET(libdatabasemodels_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagemodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagefiltermodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagefiltersettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagelistmodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagesortsettings.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/lighttable/lighttablethumbbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/lighttable/lighttablewindow.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/fuzzysearch/fuzzysearchview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/fuzzysearch/findduplicatesview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/fuzzysearch/findduplicatesalbumitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/fuzzysearch/sketchwidget.cpp
       )

    SET(libgpssearch_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/gpssearch/gpssearchview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/gpssearch/gpsmarkertiler.cpp
       )

    SET(libimageversionssidebar_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imageversionsmodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiesversionstab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/versionsdelegate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/versionstreeview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/versionsoverlays.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/versionswidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/filtershistorywidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/imagefiltershistorytreeitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/imagefiltershistorymodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/imagefiltershistoryitemdelegate.cpp
       )

    SET(libdigikamgui_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/main/digikamapp.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/album/album.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/album/albumhistory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/album/albummanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/album/albummodificationhelper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/album/albumpropsedit.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/album/albumselectcombobox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/album/albumselectdialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/album/albumselectiontreeview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/album/albumselectwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/album/albumthumbnailloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/album/albumtreeview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/album/albumwatch.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/database/databaseguierrorhandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/database/dbstatdlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/database/scancontroller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/database/dio.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/database/syncjob.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/database/imageattributeswatch.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/date/ddateedit.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/date/ddatetimeedit.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/date/ddatepickerpopup.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/date/datefolderview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/date/monthwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/date/timelinewidget.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/dragdrop/albumdragdrop.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/dragdrop/ddragobjects.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/dragdrop/imagedragdrop.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/dragdrop/tagdragdrop.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/filters/filterstatusbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/filters/filtersidebarwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/filters/geolocationfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/filters/textfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/filters/mimefilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/filters/colorlabelfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/filters/picklabelfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/filters/ratingfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/filters/tagfilterview.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/items/digikamimageview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/items/digikamimageview_p.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/items/digikamimagedelegate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/items/digikamimagefacedelegate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/items/groupindicatoroverlay.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/items/imagecategorizedview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/items/imagecategorydrawer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/items/imagedelegate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/items/imageratingoverlay.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/items/imagerotationoverlay.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/items/imagethumbnailbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/items/imagethumbnaildelegate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/items/imageselectionoverlay.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/items/imageviewutilities.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/items/assignnameoverlay.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/items/facerejectionoverlay.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/fileaction/metadatahub.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/fileaction/fileactionprogress.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/fileaction/fileactionmngr.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/fileaction/fileactionmngr_p.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/fileaction/imageinfotasksplitter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/fileaction/databaseworkeriface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/fileaction/fileworkeriface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/fileaction/fileactionimageinfolist.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/tags/addtagscombobox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/tags/addtagscompletionbox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/tags/addtagslineedit.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/tags/tagcheckview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/tags/tagfolderview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/tags/taggingaction.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/tags/tagslineeditoverlay.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/tags/tagspopupmenu.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/tags/tagsactionmngr.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/tags/tageditdlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/tags/tagmodificationhelper.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/utils/albumsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/utils/contextmenuhelper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/utils/tooltipfiller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/utils/searchmodificationhelper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/utils/editablesearchtreeview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/utils/sidebarwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/utils/slideshowbuilder.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/views/stackedview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/views/digikamview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/views/imagepreviewview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/views/mediaplayerview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/views/mapwidgetview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/views/welcomepageview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/views/leftsidebarwidgets.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/models/digikammodelcollection.cpp
       )

    SET(libdigikamhelpers_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/utils/uifilevalidator.cpp
       )

    INCLUDE_DIRECTORIES(
        ${DATABASESERVERDIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/album
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/database
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/date
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/dragdrop
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/filters
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/items
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/main
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/fileaction
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/models
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/tags
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/digikam/views
        ${CMAKE_CURRENT_SOURCE_DIR}/imageplugins
        ${CMAKE_CURRENT_SOURCE_DIR}/imageplugins/color
        ${CMAKE_CURRENT_SOURCE_DIR}/imageplugins/decorate
        ${CMAKE_CURRENT_SOURCE_DIR}/imageplugins/decorate/superimpose
        ${CMAKE_CURRENT_SOURCE_DIR}/imageplugins/enhance
        ${CMAKE_CURRENT_SOURCE_DIR}/imageplugins/enhance/hotpixels
        ${CMAKE_CURRENT_SOURCE_DIR}/imageplugins/filters
        ${CMAKE_CURRENT_SOURCE_DIR}/imageplugins/transform
        ${CMAKE_CURRENT_SOURCE_DIR}/kioslave
        ${CMAKE_CURRENT_SOURCE_DIR}/libs
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dklcms
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/cimg
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/sqlite2
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/kmemoryinfo
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/kinotify
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/core
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/haar
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagehistory
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/faces
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/auto
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/bcg
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/bw
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/cb
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/curves
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/decorate
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/film
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/fx
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/greycstoration
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/hsl
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/icc
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lc
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lens
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/levels
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/nr
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/sharp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/transform
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/wb
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/imagehistory
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/kgeomaphelper
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/jpegutils
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/jpegutils/${DIGIKAM_LIBJPEG_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/template
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/thememanager
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/progressmanager
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threads
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/versionmanager
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/q3support
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/itemview
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/iccprofiles
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/modeltest
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/advancedrename
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/advancedrename/common
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/advancedrename/parser
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/advancedrename/parser/modifiers
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/advancedrename/parser/options
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/advancedrename/parser/options/database
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/advancedrename/parser/options/database/keys
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/controller
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/devices
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/dialogs
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/items
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/views
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/main
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/q3support
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/facedetection
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/firstrun
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/fuzzysearch
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/gpssearch
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/hotplug
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/printiface
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/rawimport
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/lighttable
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/nepomuk
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/main
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/manager
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/views
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/color
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/convert
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/decorate
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/enhance
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/filters
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/metadata
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/queuemanager/basetools/transform
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/scriptiface
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/scripts
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/scripts/cleanup_digikamdb
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/scripts/digitaglinktree
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/searchwindow
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/slideshow

        # version.h, gitversion.h config-digikam.h
        ${CMAKE_CURRENT_BINARY_DIR}/digikam/utils

        # dbconfigversion.h
        ${CMAKE_CURRENT_BINARY_DIR}/digikam/database

        # external library paths
        ${JPEG_INCLUDE_DIR}
        ${PNG_PNG_INCLUDE_DIR}
        ${TIFF_INCLUDE_DIR}
        ${LCMS_INCLUDE_DIR}
    )

    ADD_SUBDIRECTORY(data)
    ADD_SUBDIRECTORY(libs)
    ADD_SUBDIRECTORY(utilities)
    ADD_SUBDIRECTORY(digikam)
    ADD_SUBDIRECTORY(kioslave)
    ADD_SUBDIRECTORY(showfoto)
    ADD_SUBDIRECTORY(imageplugins)
    IF (ENABLE_INTERNALMYSQL)
        ADD_SUBDIRECTORY(databaseserver)
    ENDIF (ENABLE_INTERNALMYSQL)

    IF(KDE4_BUILD_TESTS)
        MACRO_OPTIONAL_ADD_SUBDIRECTORY(tests)
    ENDIF(KDE4_BUILD_TESTS)

    IF(NOT WIN32)
        CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/digikam.lsm.cmake ${CMAKE_CURRENT_BINARY_DIR}/digikam.lsm)
    ENDIF(NOT WIN32)

ENDIF(DIGIKAM_CAN_BE_COMPILED)

# ==================================================================================================
# API documentation generation

IF(DOXYGEN_FOUND)

    SET(API_DIR ${CMAKE_BINARY_DIR}/api)
    SET(SOURCE_DIR ${CMAKE_SOURCE_DIR})
    SET(DOXYFILE ${CMAKE_BINARY_DIR}/Doxyfile)
    SET(WARNFILE ${CMAKE_BINARY_DIR}/doxygen-warn.log)

    CONFIGURE_FILE(Doxyfile.cmake ${DOXYFILE})

    ADD_CUSTOM_TARGET(doc ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
                      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
ENDIF(DOXYGEN_FOUND)

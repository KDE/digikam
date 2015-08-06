#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

cmake_minimum_required(VERSION 2.8.9)
project(digikam)

message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "Starting CMake configuration for: digiKam")

option(ENABLE_INTERNALMYSQL     "Build digiKam with internal MySQL server executable (default=OFF)"              OFF)
option(ENABLE_LCMS2             "Build digiKam with LCMS2 instead LCMS1 (default=OFF)"                           OFF)
option(ENABLE_BALOOSUPPORT      "Build digiKam with Baloo support (default=ON)"                                  ON)
option(ENABLE_KDEPIMLIBSSUPPORT "Build digiKam with Kdepimlibs support (default=OFF)"                            OFF)

# Mysql support options (experimental):
option(ENABLE_MYSQLSUPPORT      "Build digiKam with MySQL dabatase support (default=OFF)"                        OFF)
option(ENABLE_INTERNALMYSQL     "Build digiKam with internal MySQL server executable (default=OFF)"              OFF)

# Debug options:
option(ENABLE_DIGIKAM_MODELTEST "Enable ModelTest on some models for debugging (default=OFF)"                    OFF)
option(ENABLE_IMGLOADERDEBUGMSG "Build digiKam with more internal debug message into image loader (default=OFF)" OFF)

# ==================================================================================================
# Information to update before to release this package.

# digiKam version
set(DIGIKAM_MAJOR_VERSION "4")
set(DIGIKAM_MINOR_VERSION "13")
set(DIGIKAM_PATCH_VERSION "0")

#digiKam release date
set(DIGIKAM_RELEASE_DATE "2015-08-30")

# Suffix to add at end of version string. Usual values are:
# "-git"   : alpha code unstable from git. Do not use in production
# "-beta1" : beta1 release.
# "-beta2" : beta2 release.
# "-beta3" : beta3 release.
# "-rc"    : release candidate.
# ""       : final release. Can be used in production.
set(DIGIKAM_SUFFIX_VERSION "")

# ==================================================================================================
# Set env. variables accordingly.

# NOTE: This string is used to set libdigikamcore and libdigikamdatabase SO version ID
set(DIGIKAM_VERSION_SHORT
    "${DIGIKAM_MAJOR_VERSION}.${DIGIKAM_MINOR_VERSION}.${DIGIKAM_PATCH_VERSION}"
   )

set(DIGIKAM_VERSION_STRING
    "${DIGIKAM_VERSION_SHORT}${DIGIKAM_SUFFIX_VERSION}"
   )

# Databse Schema version
# We must set this variable here at top level because it is used in both digikam/database and data/database
# Version history:
# 1 : Original database schema
# 2 : 08-08-2014 : Fix Images.names field size (see bug #327646)
set(DBCONFIG_XML_VERSION "2")

# ==================================================================================================
# Debug area codes
# These codes will be set for every project specific path automatically, so
# that you can use kDebug() without any usage of the area code throughout the
# digiKam code base.

set(AREA_CODE_GENERAL        "50003")
set(AREA_CODE_KIOSLAVES      "50004")
set(AREA_CODE_SHOWFOTO       "50005")
set(AREA_CODE_IMAGEPLUGINS   "50006")
set(AREA_CODE_DATABASESERVER "50007")
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=${AREA_CODE_GENERAL})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

macro(SET_DEBUG_AREA_CODE _areacode)
    remove_definitions(-DKDE_DEFAULT_DEBUG_AREA=${AREA_CODE_GENERAL})
    add_definitions(-DKDE_DEFAULT_DEBUG_AREA=${_areacode})
endmacro()

# ==================================================================================================
# Dependencies

set(QT_MIN_VERSION   "4.6.0")
set(KDE_MIN_VERSION  "4.4.0")
find_package(Qt4  ${QT_MIN_VERSION}  REQUIRED)
find_package(KDE4 ${KDE_MIN_VERSION} REQUIRED)

include(KDE4Defaults)
include(MacroLibrary)
include(FindPackageHandleStandardArgs)
include(FindPkgConfig)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

include(MacroUtils)
include(MacroOpenCV)
include(MacroJPEG)

# mandatory

set(PGF_MIN_VERSION   "6.12.24")
find_package(TIFF                    REQUIRED)
find_package(PNG                     REQUIRED)
find_package(PGF ${PGF_MIN_VERSION}  REQUIRED)
find_package(Kexiv2                  REQUIRED)
find_package(Kdcraw                  REQUIRED)

# optional

set(KIPI_MIN_VERSION  "2.0.0")
find_package(Kipi ${KIPI_MIN_VERSION})
find_package(Kface)
find_package(KGeoMap)
find_package(PkgConfig)
find_package(Jasper)
find_package(Eigen3)
find_package(Boost)

# -- check OpenCV --------------------------------------------------------------------------------

set(OPENCV_MIN_VERSION "2.4.9")
DETECT_OPENCV(${OPENCV_MIN_VERSION}  core highgui objdetect contrib legacy imgproc)

# -- check the libkdcraw version -----------------------------------------------------------------

set(KDCRAW_MIN_VERSION "2.2.0")

if(KDCRAW_VERSION)

    message(STATUS "libkdcraw: Found version ${KDCRAW_VERSION} (required: ${KDCRAW_MIN_VERSION})")
    if(${KDCRAW_VERSION} VERSION_LESS ${KDCRAW_MIN_VERSION})
        set(KDCRAW_FOUND FALSE)
    endif()

else()

    message(WARNING "libkdcraw: Version information not found, your version is probably too old.")
    set(KDCRAW_FOUND FALSE)

endif()

# -- check the libkgeomap version -----------------------------------------------------------------

set(KGEOMAP_MIN_VERSION "3.0.0")

if(KGEOMAP_VERSION)

    message(STATUS "libkgeomap: Found version ${KGEOMAP_VERSION} (required: ${KGEOMAP_MIN_VERSION})")
    if(${KGEOMAP_VERSION} VERSION_LESS ${KGEOMAP_MIN_VERSION})
        SET(KGEOMAP_FOUND FALSE)
    endif()

else()

    message(WARNING "libkgeomap: Version information not found, your version is probably too old.")
    set(KGEOMAP_FOUND FALSE)

endif()

# -- LCMS version 1 and 2 library handling (See bug #294105) -------------------------------------
# Note: if ENABLE_LCMS2 is on and no LCMS2 is available, build will fail, even if LCMS1 is present

if(ENABLE_LCMS2)

    message(STATUS "Check for LCMS2 availability...")
    find_package(LCMS2)

    if(LCMS2_FOUND AND (LCMS2_VERSION VERSION_EQUAL 2.1 OR LCMS2_VERSION VERSION_GREATER 2.1))

        message(STATUS "Found LCMS2: ${LCMS2_LIBRARIES} ${LCMS2_INCLUDE_DIR}")
        set(LCMS_FOUND ${LCMS2_FOUND})
        set(LCMS_LIBRARIES ${LCMS2_LIBRARIES})
        set(LCMS_INCLUDE_DIR ${LCMS2_INCLUDE_DIR})
        set(USE_LCMS_VERSION_2000 1)

    endif()

else()

    message(STATUS "Check for LCMS1 availability...")
    find_package(LCMS)

    if(LCMS_FOUND)

        message(STATUS "Found LCMS1: ${LCMS_LIBRARIES} ${LCMS_INCLUDE_DIR}")
        set(USE_LCMS_VERSION_1000 1)

    else()

        message(STATUS "Check for LCMS2 availability instead LCMS1...")
        find_package(LCMS2)

        if(LCMS2_FOUND AND (LCMS2_VERSION VERSION_EQUAL 2.1 OR LCMS2_VERSION VERSION_GREATER 2.1))
            message(STATUS "Found LCMS2 instead LCMS1: ${LCMS2_LIBRARIES} ${LCMS2_INCLUDE_DIR}")
            set(LCMS_FOUND ${LCMS2_FOUND})
            set(LCMS_LIBRARIES ${LCMS2_LIBRARIES})
            set(LCMS_INCLUDE_DIR ${LCMS2_INCLUDE_DIR})
            set(USE_LCMS_VERSION_2000 1)
        endif()

    endif()

endif()

# -- To link under Solaris (see bug #274484) ------------------------------------------------------

if(NOT WIN32)
    find_library(MATH_LIBRARY m)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
    find_library(KVM_LIBRARY kvm)
endif()

# -- Libjpeg rules --------------------------------------------------------------------------------

DETECT_JPEG()
set(DIGIKAM_LIBJPEG_DIR libjpeg-${JPEG_LIB_VERSION})

# -- Mysql rules ----------------------------------------------------------------------------------

if(ENABLE_MYSQLSUPPORT AND ENABLE_INTERNALMYSQL)

    # Check MySQL executables

    find_program(SERVERCMD_MYSQL mysqld
                 PATHS /usr/sbin
                       /usr/local/sbin
                       /usr/libexec
                       /usr/local/libexec
                       /opt/local/libexec
                       /opt/mysql/libexec
                       /usr/mysql/bin
                       /opt/mysql/sbin
                       ${MYSQLD_PATH}
                 ENV MYSQLD_PATH )
    message(STATUS "Found MySQL server executable at: ${SERVERCMD_MYSQL}")

    # NOTE: under windows, mysql_install_db PERL script is named "mysql_install_db.pl"
    #       under MACOS-X, through macports project, mysql_install_db PERL script is named "mysql_install_db5"

    find_program(INITCMD_MYSQL mysql_install_db mysql_install_db5 mysql_install_db.pl
                 PATHS /usr/sbin
                       /usr/local/sbin
                       /usr/libexec
                       /usr/local/libexec
                       /opt/mysql/libexec
                       /usr/mysql/bin
                       /opt/mysql/bin
                       /usr/share/mysql/scripts
                       ${MYSQL_TOOLS_PATH}
                 ENV MYSQL_TOOLS_PATH)
    message(STATUS "Found MySQL install_db executable at: ${INITCMD_MYSQL}")
    set(DATABASESERVERDIR "${CMAKE_CURRENT_SOURCE_DIR}/databaseserver")

else()

    set(DATABASESERVERDIR "")

endif()

# ==================================================================================================
# Optional dependencies

MACRO_OPTIONAL_FIND_PACKAGE(Doxygen)
MACRO_OPTIONAL_FIND_PACKAGE(Sqlite2)
MACRO_OPTIONAL_FIND_PACKAGE(Lqr-1)

# -- libgphoto2 rules --------------------------------------------------------------------------

MACRO_OPTIONAL_FIND_PACKAGE(Gphoto2)

if(NOT WIN32 AND GPHOTO2_FOUND)
    # See bug #268267 : digiKam need to be linked to libusb to prevent crash at gphoto2 init if opencv is linked with libdc1394
    MACRO_OPTIONAL_FIND_PACKAGE(USB)
endif()

if(GPHOTO2_FOUND)
    exec_program(gphoto2-config ARGS --version RETURN_VALUE _return_VALUE OUTPUT_VARIABLE GPHOTO2_VERSION)
    string(REPLACE "libgphoto2" "" GPHOTO2_VERSION "${GPHOTO2_VERSION}")
    MACRO_ENSURE_VERSION("2.4.0" "${GPHOTO2_VERSION}" VERSION_GPHOTO2)

    if(VERSION_GPHOTO2 AND LIBUSB_FOUND)
        set(GPHOTO2_FOUND true)
    else()
        set(GPHOTO2_FOUND false)
    endif()

    MACRO_ENSURE_VERSION("2.5.0" "${GPHOTO2_VERSION}" VERSION_GPHOTO25)

    if(VERSION_GPHOTO25)
        set(HAVE_GPHOTO25 1)
    else()
        set(HAVE_GPHOTO25 0)
    endif()
endif()

# -- KDEPim libraries rules -------------------------------------------------------------------

if(ENABLE_KDEPIMLIBSSUPPORT)
    MACRO_OPTIONAL_FIND_PACKAGE(KdepimLibs)
endif()

# -- Check LensFun library for Lens auto-correction tool --------------------------------------

MACRO_OPTIONAL_FIND_PACKAGE(LensFun)

set(LENSFUN_MIN_VERSION "0.2.6.0")

if(LENSFUN_VERSION)
    message(STATUS "liblensfun: Found version ${LENSFUN_VERSION} (required: ${LENSFUN_MIN_VERSION})")
    if(${LENSFUN_VERSION} VERSION_LESS ${LENSFUN_MIN_VERSION})
        set(LENSFUN_FOUND FALSE)
    endif()
else()
    message(WARNING "liblensfun: Version information not found, your version is probably too old.")
    set(LENSFUN_FOUND FALSE)
endif()

# -- Baloo rules -----------------------------------------------------------------------------

if(ENABLE_BALOOSUPPORT)
    # Baloo support from 4.12
    if(${KDE_VERSION} VERSION_GREATER "4.11.99")

        MACRO_OPTIONAL_FIND_PACKAGE(KFileMetaData CONFIG)
        MACRO_LOG_FEATURE(KFileMetaData_FOUND "KFileMetaData Library"
                          "A library for extracting file metadata" "https://projects.kde.org/kfilemetadata" TRUE)

        MACRO_OPTIONAL_FIND_PACKAGE(Baloo)
        MACRO_LOG_FEATURE(Baloo_FOUND "Baloo" "Baloo"
                          "http://www.kde.org" TRUE "" "Baloo extends the search and tagging")

        if(NOT KFileMetaData_FOUND)
            set(Baloo_FOUND FALSE)
        endif()

    endif()
endif()

# -- compilation options definitions --------------------------------------------------------

MACRO_BOOL_TO_01(GPHOTO2_FOUND            HAVE_GPHOTO2)
MACRO_BOOL_TO_01(KDEPIMLIBS_FOUND         HAVE_KDEPIMLIBS)
MACRO_BOOL_TO_01(LENSFUN_FOUND            HAVE_LENSFUN)
MACRO_BOOL_TO_01(Baloo_FOUND              HAVE_BALOO)
MACRO_BOOL_TO_01(SQLITE2_FOUND            HAVE_SQLITE2)
MACRO_BOOL_TO_01(LQR-1_FOUND              HAVE_LIBLQR_1)
MACRO_BOOL_TO_01(ENABLE_MYSQLSUPPORT      HAVE_MYSQLSUPPORT)
MACRO_BOOL_TO_01(ENABLE_INTERNALMYSQL     HAVE_INTERNALMYSQL)
MACRO_BOOL_TO_01(JASPER_FOUND             HAVE_JASPER)
MACRO_BOOL_TO_01(EIGEN3_FOUND             HAVE_EIGEN3)
MACRO_BOOL_TO_01(KIPI_FOUND               HAVE_KIPI)
MACRO_BOOL_TO_01(KFACE_FOUND              HAVE_KFACE)
MACRO_BOOL_TO_01(KGEOMAP_FOUND            HAVE_KGEOMAP)

# Whether to use Qt's scaling to downscale previews. Under MacOSX, Qt
# can make use of the higher physical resolution of Retina
# displays. However, it seems that other Qt renderers perform badly at
# this, so disable. If other renderers switch to coarser logical
# pixels, one probably needs this feature on these platforms as well.
MACRO_BOOL_TO_01(APPLE                    USE_QT_SCALING)

MACRO_BOOL_TO_01(ENABLE_IMGLOADERDEBUGMSG USE_IMGLOADERDEBUGMSG)

# Set config files accordingly with optional dependencies

configure_file(app/utils/config-digikam.h.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/app/utils/config-digikam.h)

# ==================================================================================================
# Log messages

message(STATUS "")
message(STATUS "----------------------------------------------------------------------------------")
message(STATUS " digiKam ${DIGIKAM_VERSION_STRING} dependencies results   <http://www.digikam.org>")
message(STATUS "")

PRINT_QTMODULE_STATUS("Qt4 SQL" QT_QTSQL_FOUND)
PRINT_COMPONENT_COMPILE_STATUS("MySQL Database Support" ENABLE_MYSQLSUPPORT)

if(ENABLE_MYSQLSUPPORT)

    if (ENABLE_INTERNALMYSQL)
        PRINT_EXECUTABLE_STATUS("MySQL Server" mysqld MYSQLD_PATH SERVERCMD_MYSQL)
        if (NOT WIN32)
            PRINT_EXECUTABLE_STATUS("MySQL install_db tool" mysql_install_db MYSQL_TOOLS_PATH INITCMD_MYSQL)
        endif()
    else()
        message(STATUS " MySQL server found....................... NO  (optional - disabled by user)")
    endif()

endif()

# ================================================================

PRINT_LIBRARY_STATUS("libqt"                  "http://www.qtsoftware.com"                                        "(version >= ${QT_MIN_VERSION})"     QT_FOUND)
PRINT_LIBRARY_STATUS("libkde"                 "http://www.kde.org"                                               "(version >= ${KDE_MIN_VERSION})"    KDE4_FOUND)
PRINT_LIBRARY_STATUS("libjpeg"                "http://www.ijg.org"                                               "(version >= 6b)"                    JPEG_FOUND)
PRINT_LIBRARY_STATUS("libtiff"                "http://www.remotesensing.org/libtiff"                             "(version >= 3.8.2)"                 TIFF_FOUND)
PRINT_LIBRARY_STATUS("libpng"                 "http://www.libpng.org/pub/png/libpng.html"                        "(version >= 1.2.7)"                 PNG_FOUND)
PRINT_LIBRARY_STATUS("liblcms"                "http://www.littlecms.com"                                         "(version >= 1.0.0)"                 LCMS_FOUND)
PRINT_LIBRARY_STATUS("libboostgraph"          "http://www.boost.org/doc/libs"                                    "(version >= 1.43.0)"                Boost_FOUND)
PRINT_LIBRARY_STATUS("libkexiv2"              "https://projects.kde.org/projects/kde/kdegraphics/libs/libkexiv2" "(version >= 1.2.0)"                 KEXIV2_FOUND)
PRINT_LIBRARY_STATUS("libkdcraw"              "https://projects.kde.org/projects/kde/kdegraphics/libs/libkdcraw" "(version >= ${KDCRAW_MIN_VERSION})" KDCRAW_FOUND)
PRINT_LIBRARY_STATUS("libpgf"                 "http://www.libpgf.org"                                            "(version >= ${PGF_MIN_VERSION})"    PGF_FOUND)
PRINT_LIBRARY_STATUS("libopencv"              "http://opencv.willowgarage.com"                                   "(version >= ${OPENCV_MIN_VERSION})" OpenCV_FOUND)

# ================================================================

PRINT_OPTIONAL_LIBRARY_STATUS("libkipi"       "https://projects.kde.org/projects/kde/kdegraphics/libs/libkipi" "(version >= ${KIPI_MIN_VERSION})"    "digiKam will be compiled without Kipi-plugins support."                      KIPI_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libkface"      "https://projects.kde.org/projects/extragear/libs/libkface"      "(version >= 2.0.0)"                  "digiKam will be compiled without faces detection and recognition support."   KFACE_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libkgeomap"    "https://projects.kde.org/projects/extragear/libs/libkgeomap"    "(version >= ${KGEOMAP_MIN_VERSION})" "digiKam will be compiled without geolocation maps support."                  KGEOMAP_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libkdepimlibs" "http://pim.kde.org"                                             "(version >= 4.4.0)"                  "digiKam will be compiled without KDE address book support."                  KDEPIMLIBS_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libjasper"     "http://www.ece.uvic.ca/~mdadams/jasper"                         "(version >= 1.7.0)"                  "digiKam will be compiled without JPEG2000 support."                          JASPER_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("liblensfun"    "http://lensfun.sourceforge.net"                                 "(version >= 0.2.6)"                  "digiKam will be compiled without Lens Auto Correction tool support."         LENSFUN_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("liblqr-1"      "http://liblqr.wikidot.com"                                      "(version >= 0.4.1)"                  "digiKam will be compiled without Contents Aware Resizer tool support."       LQR-1_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libeigen3"     "http://eigen.tuxfamily.org"                                     "(version >= 3.0.0)"                  "digiKam will be compiled without Refocus tool support."                      EIGEN3_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libgphoto2"    "http://www.gphoto.org"                                          "(version >= 2.4.0)"                  "digiKam will be compiled without GPhoto2 camera drivers support."            GPHOTO2_FOUND AND LIBUSB_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libbaloo"      "https://projects.kde.org/projects/kde/kdelibs/baloo"            "(version >= 4.12.0)"                 "digiKam will be compiled without KDE files indexing support."                Baloo_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libsqlite2"    "http://www.sqlite.org"                                          "(version >= 2.0.0)"                  "digiKam will be compiled without database version <= 0.7 importing support." SQLITE2_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("doxygen"       "http://www.doxygen.org"                                         "(version >= 1.8.0)"                  "digiKam will be compiled without API documentation building support."        DOXYGEN_FOUND)

# ================================================================

if(QT_FOUND AND KDE4_FOUND AND JPEG_FOUND AND TIFF_FOUND AND PNG_FOUND AND LCMS_FOUND AND Boost_FOUND AND KEXIV2_FOUND AND KDCRAW_FOUND AND PGF_FOUND AND OpenCV_FOUND)
    message(STATUS " digiKam can be compiled.................. YES")
    set(DIGIKAM_CAN_BE_COMPILED true)
else()
    message(FATAL_ERROR " digiKam will be compiled................. NO  (Look README file for more details about dependencies)")
    set(DIGIKAM_CAN_BE_COMPILED false)
endif()

message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "")

if(DIGIKAM_CAN_BE_COMPILED)

    # ==================================================================================================
    # Create git version header

    # We only do this IF we are in a .git dir
    find_file(GIT_MARKER entries PATHS ${CMAKE_SOURCE_DIR}/.git)

    if(NOT GIT_MARKER)
        set(GIT_MARKER ${CMAKE_SOURCE_DIR}/CMakeLists.txt)  # Dummy file
    endif()

    # Add a custom command to drive the git script whenever the git entries
    # file changes.
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/gitscript.cmake.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/gitscript.cmake"
                   @ONLY)

    # Add a custom target to drive the custom command.
    add_custom_target(digikam-gitversion ALL COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/gitscript.cmake")

    # ==================================================================================================
    # get the gcc version

    # CImg.h version 1.2.8 do not compile fine with gcc 4.3.x
    # See bug #163118: digikam-0.9.4_beta5 compilation hangs with gcc 4.3
    # Using -fno-tree-pre is work around this problem.

    if(CMAKE_COMPILER_IS_GNUCXX)
        exec_program(${CMAKE_CXX_COMPILER} ARGS ${CMAKE_CXX_COMPILER_ARG1} -dumpversion OUTPUT_VARIABLE GCC_VERSION)

        MACRO_ENSURE_VERSION("4.3.0" "${GCC_VERSION}" GCC_VERSION_BAD)

        if(GCC_VERSION_BAD)
            message(STATUS "Adjusting compilation flags for GCC version (${GCC_VERSION} )")
            add_definitions(-fno-tree-pre)
        endif()
    endif()

    # ==================================================================================================
    # To prevent warnings from M$ compiler

    if(WIN32 AND MSVC)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        add_definitions(-D_ATL_SECURE_NO_WARNINGS)
        add_definitions(-D_AFX_SECURE_NO_WARNINGS)
    endif()

    # ==================================================================================================
    # Definitions and includes rules

    add_definitions(${QT_DEFINITIONS}
                    ${KDE4_DEFINITIONS}
                    ${KDCRAW_DEFINITIONS}
                    ${KEXIV2_DEFINITIONS}
                   )

    # Enable C++ Exceptions support, require by Greycstoration algorithm (CImg.h)
    if(NOT MSVC)
        add_definitions(${KDE4_ENABLE_EXCEPTIONS})
    endif()

    include_directories(${KEXIV2_INCLUDE_DIR}
                        ${KDCRAW_INCLUDE_DIR}
                        ${KDE4_INCLUDES}
                        ${QT_INCLUDES}
                        ${PGF_INCLUDE_DIRS}
                        ${CMAKE_CURRENT_SOURCE_DIR}/app
                        ${CMAKE_CURRENT_BINARY_DIR}/app
                       )

    # JPEG2000 support

    if(JASPER_FOUND)
        include_directories(${JASPER_INCLUDE_DIR})
    endif()

    # libgphoto2 library rules

    if(GPHOTO2_FOUND)
        include_directories(${GPHOTO2_INCLUDE_DIRS})
    endif()

    # Baloo library rules

    if(Baloo_FOUND)
        include_directories(${BALOO_INCLUDE_DIR} ${KFILEMETADATA_INCLUDE_DIR})
    endif()

    # Boost library rules

    IF(Boost_FOUND)
        INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    ENDIF()

    # LibLqr-1 library rules

    if(LQR-1_FOUND)
        include_directories(${LQR-1_INCLUDE_DIRS})
        set(liblqr_LIBS ${LQR-1_LIBRARIES})
    endif()

    # Lensfun library rules

    if(LENSFUN_FOUND)
        include_directories(${LENSFUN_INCLUDE_DIRS})
        set(liblensfun_LIBS ${LENSFUN_LIBRARIES})
    endif()

    # Refocus tool dependency

    if(EIGEN3_FOUND)
        include_directories(${EIGEN3_INCLUDE_DIR})
    endif()

    # Kipi-plugins support

    if(KIPI_FOUND)
        include_directories(${KIPI_INCLUDE_DIR})
        add_definitions(${KIPI_DEFINITIONS})
    endif()

    # Faces detection and recognition support

    if(KFACE_FOUND)
        include_directories(${KFACE_INCLUDE_DIR})
        add_definitions(${KFACE_DEFINITIONS})
    endif()

    # Geolocation maps support

    if(KGEOMAP_FOUND)
        include_directories(${KGEOMAP_INCLUDE_DIR})
        add_definitions(${KGEOMAP_DEFINITIONS})
    endif()

    # ==================================================================================================
    # Common targets parts

    set(libdimg_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dimg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/drawdecoding.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dimgscale.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dcolor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dcolorcomposer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/imagehistory/dimagehistory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/imagehistory/filteraction.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/imagehistory/historyimageid.cpp
       )

    set(libdimgloaders_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/dimgloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/pngloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/jpegloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/tiffloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/rawloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/ppmloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/qimageloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/pgfloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/jpegsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/pngsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/tiffsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/pgfsettings.cpp
       )

    if(JASPER_FOUND)
        set(libdimgloaders_SRCS
            ${libdimgloaders_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/jp2kloader.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/jp2ksettings.cpp
           )
    endif()

    set(libdimgfilters_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dklcms/digikam-lcms.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/dimgbuiltinfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/dimgthreadedfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/dimgthreadedanalyser.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/fx/colorfxsettings.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/nr/nrestimate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/nr/nrsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/sharp/sharpenfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/sharp/unsharpmaskfilter.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/wb/wbcontainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/wb/wbfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/wb/wbsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/transform/freerotationfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/transform/freerotationsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/transform/shearfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/transform/autocrop.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/greycstoration/greycstorationfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/greycstoration/greycstorationsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lens/antivignettingfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lens/antivignettingsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lens/lensdistortionfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lens/pixelaccess.cpp
       )

    if(LQR-1_FOUND)
        set(libdimgfilters_SRCS
            ${libdimgfilters_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/transform/contentawarefilter.cpp
           )
    endif()

    if(LENSFUN_FOUND)
        set(libdimgfilters_SRCS
            ${libdimgfilters_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lens/lensfunfilter.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lens/lensfuncameraselector.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lens/lensfuniface.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/lens/lensfunsettings.cpp
           )
    endif()

    if(EIGEN3_FOUND)
        set(libdimgfilters_SRCS
            ${libdimgfilters_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/sharp/refocusfilter.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/sharp/matrix.cpp
           )
    endif()

    set(libdmetadata_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/metadatasettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/metadatasettingscontainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/metadatainfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/infocontainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/dmetadata.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/geodetictools.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/template.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/captionvalues.cpp
       )

    set(libversionmanager_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/versionmanager/versionmanager.cpp
       )

    set(libimageproperties_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiessidebar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiestab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiesmetadatatab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiescolorstab.cpp
       )

    if(KGEOMAP_FOUND)
        set(libkgeomaphelper_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/kgeomaphelper/digikam2kgeomap.cpp
           )

        set(libimageproperties_SRCS
            ${libimageproperties_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiesgpstab.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagegpsmodelhelper.cpp
           )
    endif()

    # --enable-final triggers: http://bugs.kde.org/show_bug.cgi?id=126326
    # digikam: camera download: auto-rotated images lose EXIF info ...
    # So make sure nofinal is always used here!
    KDE4_NO_ENABLE_FINAL(libjpegutils)

    # Define libjpeg_EXPORTS: kde-win emerged libjpeg uses this define to
    # decide wether to make dllimport (by default) or dllexport. So, we need to export.
    add_definitions(-Dlibjpeg_EXPORTS)

    set(libjpegutils_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/jpegutils/jpegutils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/jpegutils/iccjpeg.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/jpegutils/${DIGIKAM_LIBJPEG_DIR}/transupp.c
       )

    if(MSVC)
        set(libjpegutils_SRCS
            ${libjpegutils_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/jpegutils/jpegwin.cpp
           )
    endif()

    set(libthreadimageio_SRCS
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
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/previewsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/thumbnailbasic.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/thumbnailcreator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/thumbnailloadthread.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/thumbnailtask.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/thumbnailsize.cpp
       )

    set(libcommonwidgets_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/abstractitemdragdrophandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/buttonicondisabler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/colorgradientwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/comboboxutilities.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dfontselect.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dgradientslider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dragdropimplementations.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/filesaveoptionsbox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/fullscreensettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/modelcompletion.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/searchtextbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/splashscreen.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/statesavingobject.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/visibilitycontroller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/workingwidget.cpp
       )

    set(libnotificationmanager_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/notificationmanager/dnotificationwrapper.cpp
       )

    if(APPLE)
        set(libnotificationmanager_SRCS
            ${libnotificationmanager_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/notificationmanager/macnotification.mm
           )
    endif()

    set(libmainview_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/mainview/dxmlguiwindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/mainview/daboutdata.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/mainview/dzoombar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/mainview/dlogoaction.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/mainview/ditemtooltip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/mainview/dhistoryview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/mainview/dcursortracker.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/mainview/thumbbardock.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/mainview/thememanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/mainview/sidebar.cpp
       )

    set(libitemview_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/itemview/digikamkcategorizedview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/itemview/dcategorizedview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/itemview/ditemdelegate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/itemview/imagedelegateoverlay.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/itemview/itemviewhoverbutton.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/itemview/itemviewimagedelegate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/itemview/itemviewtooltip.cpp
       )

    set(libgraphicsviewwidgets_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/clickdragreleaseitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/dimgchilditem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/dimgpreviewitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/regionframeitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/graphicsdimgitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/graphicsdimgview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/imagezoomsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/previewlayout.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/paniconwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview/itemvisibilitycontroller.cpp
       )

    set(libiccprofileswidgets_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/iccprofiles/iccpreviewwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/iccprofiles/iccprofilewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/iccprofiles/cietonguewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/iccprofiles/iccprofilescombobox.cpp
       )

    set(libmetadatawidgets_SRCS
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
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/ratingwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/colorlabelwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/picklabelwidget.cpp
       )

    set(libslideshow_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/slideshow/slidetoolbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/slideshow/slideosd.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/slideshow/slideproperties.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/slideshow/slideimage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/slideshow/slideerror.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/slideshow/slideend.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/slideshow/slideshow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/slideshow/slidehelp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/slideshow/slideshowsettings.cpp
       )

    set(libeditorwidgets_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/widgets/imageguidewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/widgets/imagepreviewitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/widgets/previewtoolbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/widgets/previewlist.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/widgets/imageregionwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/widgets/imageregionitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/widgets/rubberitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/widgets/canvas.cpp
       )

    set(libeditordlg_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/dialogs/colorcorrectiondlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/dialogs/softproofdialog.cpp
       )

    set(libeditorplugin_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/plugin/imageiface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/plugin/imageplugin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/plugin/imagepluginloader.cpp
       )

    set(libeditorcore_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/core/undocache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/core/undoaction.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/core/undomanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/core/editorcore.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/core/fileoperation.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/core/iccpostloadingmanager.cpp
       )

    set(libeditorprintiface_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/printiface/printhelper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/printiface/printoptionspage.cpp
       )

    set(libeditorrawimport_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/rawimport/rawimport.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/rawimport/rawpreview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/rawimport/rawsettingsbox.cpp
       )

    set(libprogressmanager_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/progressmanager/overlaywidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/progressmanager/progressview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/progressmanager/progressmanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/progressmanager/statusbarprogresswidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/progressmanager/statusprogressbar.cpp
       )

    set(libtthread_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threads/threadmanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threads/workerobject.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threads/dynamicthread.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threads/parallelworkers.cpp
       )

    # ==================================================================================================
    # digiKam target parts

    set(libtemplate_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/template/templatemanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/template/templatelist.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/template/templateselector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/template/templateviewer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/template/templatepanel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/template/subjectedit.cpp
       )

    set(libdialogs_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/dprogressdlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/dbusydlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/filesaveoptionsdlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/iccprofileinfodlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/imagedialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/infodlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/libsinfodlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/rawcameradlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/triplechoicedialog.cpp
       )

    set(libdeletedialog_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/deletedialog.cpp
       )

    set(libsetup_SRCS
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
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupraw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupcamera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/cameraselection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/cameralist.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/cameratype.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupdatabase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupversioning.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/importfilters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupimagequalitysorter.cpp
       )

    if(KIPI_FOUND)
        set(libkipiiface_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface/kipipluginloader.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface/kipiimageinfo.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface/kipiimagecollection.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface/kipiimagecollectionselector.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface/kipiuploadwidget.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface/kipiinterface.cpp
           )
    endif()

    set(libbaloowrap_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/baloo/baloowrap.cpp
       )

    if(KFACE_FOUND)
        set(libfacemanagement_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/facemanagement/faceutils.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/facemanagement/facegroup.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/facemanagement/facepipeline.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/facemanagement/facescandialog.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/facemanagement/assignnamewidget.cpp
           )
    endif()

    set(libimgqsort_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imgqsort/imgqsort.cpp
       )

    set(libfirstrun_SRCS
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

    set(libimagepropertiesdigikam_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiessidebarcamgui.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiessidebardb.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/cameraitempropertiestab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagedescedittab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/captionedit.cpp
       )

    set(libeditoriface_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/editortool.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/editortooliface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/editorstackview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/editortoolsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/editorwindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/actioncategorizedview.cpp
       )

    set(libeditorgui_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/main/imagewindow.cpp
       )

    set(libmaintenance_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/imageinfojob.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/imageinfoalbumsjob.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/metadatatask.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/metadatasynchronizer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/duplicatesfinder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/newitemsfinder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/thumbsgenerator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/thumbstask.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/fingerprintsgenerator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/fingerprintstask.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/imagequalitysorter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/imagequalitysettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/imagequalitytask.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/maintenancedlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/maintenancemngr.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/maintenancetool.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/maintenancesettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/maintenancethread.cpp
       )

    if(KFACE_FOUND)
        set(libmaintenance_SRCS
            ${libmaintenance_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/maintenance/facedetector.cpp
           )
    endif()

    set(libdatabasecore_SRCS
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

    set(libdatabase_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/albumdb.cpp
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

    set(libdatabaseui_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/databasewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/migrationdlg.cpp
       )

    set(pgfutils_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/pgfutils.cpp
       )

    set(libkde3rdparty_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/kmemoryinfo/kmemoryinfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/kinotify/kinotify.cpp
       )

    set(libhaar_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/haar/haar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/haar/haariface.cpp
       )

    set(libgenericmodels_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/categorizeditemmodel.cpp
       )

    set(libdatabasemodels_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagemodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagefiltermodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagefiltersettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagelistmodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagesortsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagethumbnailmodel.cpp
       )

    set(libalbummodels_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagealbummodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imagealbumfiltermodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/abstractalbummodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/albummodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/albumfiltermodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/albummodeldragdrophandler.cpp
       )

    set(liblighttable_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/lighttable/lighttablepreview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/lighttable/lighttableview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/lighttable/lighttablethumbbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/lighttable/lighttablewindow.cpp
       )

    set(libsearchwindow_SRCS
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

    set(libfuzzysearch_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/fuzzysearch/fuzzysearchview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/fuzzysearch/findduplicatesview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/fuzzysearch/findduplicatesalbum.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/fuzzysearch/findduplicatesalbumitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/fuzzysearch/sketchwidget.cpp
       )

    if(KGEOMAP_FOUND)
        set(libgpssearch_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/gpssearch/gpssearchview.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/gpssearch/gpsmarkertiler.cpp
           )
    endif()

    set(libimageversionssidebar_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/models/imageversionsmodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiesversionstab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/versionsdelegate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/versionstreeview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/versionsoverlays.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/imagehistory/versionswidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/imagehistory/filtershistorywidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/imagehistory/imagefiltershistorytreeitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/imagehistory/imagefiltershistorymodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/imagehistory/imagefiltershistoryitemdelegate.cpp
       )

    set(libdigikamgui_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/app/main/digikamapp.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/app/album/album.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/album/albumhistory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/album/albummanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/album/albummodificationhelper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/album/albumpropsedit.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/album/albumselectors.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/album/albumselectcombobox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/album/albumselectdialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/album/albumselectiontreeview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/album/albumselectwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/album/albumthumbnailloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/album/albumtreeview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/album/albumwatch.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/album/albumlabelstreeview.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/app/database/databaseguierrorhandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/database/dbstatdlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/database/scancontroller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/database/dio.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/database/syncjob.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/database/imageattributeswatch.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/app/date/ddateedit.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/date/ddatetimeedit.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/date/ddatepickerpopup.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/date/datefolderview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/date/monthwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/date/timelinewidget.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/app/dragdrop/importdragdrop.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/dragdrop/albumdragdrop.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/dragdrop/ddragobjects.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/dragdrop/imagedragdrop.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/dragdrop/tagdragdrop.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/app/filters/filterstatusbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/filters/filtersidebarwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/filters/geolocationfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/filters/textfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/filters/mimefilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/filters/colorlabelfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/filters/picklabelfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/filters/ratingfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/filters/tagfilterview.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/app/items/digikamimageview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/items/digikamimageview_p.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/items/digikamimagedelegate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/items/imagecategorizedview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/items/imagecategorydrawer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/items/imagedelegate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/items/imagethumbnailbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/items/imagethumbnaildelegate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/items/imageviewutilities.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/app/items/overlays/groupindicatoroverlay.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/items/overlays/imageratingoverlay.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/items/overlays/imagerotationoverlay.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/items/overlays/imagecoordinatesoverlay.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/items/overlays/imageselectionoverlay.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/app/fileaction/metadatahub.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/fileaction/fileactionprogress.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/fileaction/fileactionmngr.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/fileaction/fileactionmngr_p.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/fileaction/imageinfotasksplitter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/fileaction/databaseworkeriface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/fileaction/fileworkeriface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/fileaction/fileactionimageinfolist.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/addtagscombobox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/addtagscompletionbox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/addtagslineedit.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/tagcheckview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/tagfolderview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/taggingaction.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/tagslineeditoverlay.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/tagspopupmenu.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/tagsactionmngr.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/tageditdlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/tagmodificationhelper.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/tagsmanager/tagsmanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/tagsmanager/tagpropwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/tagsmanager/taglist.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/tagsmanager/tagmngrtreeview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/tagsmanager/models/tagmngrlistview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/tagsmanager/models/tagmngrlistmodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/tagsmanager/models/tagmngrlistitem.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/app/settings/applicationsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/settings/applicationsettings_p.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/settings/applicationsettings_albums.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/settings/applicationsettings_database.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/settings/applicationsettings_iconview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/settings/applicationsettings_mime.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/settings/applicationsettings_miscs.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/settings/applicationsettings_tooltips.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/app/utils/contextmenuhelper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/utils/tooltipfiller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/utils/searchmodificationhelper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/utils/editablesearchtreeview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/utils/sidebarwidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/utils/slideshowbuilder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/utils/digikammodelcollection.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/stackedview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/digikamview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/imagepreviewviewitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/imagepreviewview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/mediaplayerview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/welcomepageview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/leftsidebarwidgets.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/tableview/tableview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/tableview/tableview_treeview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/tableview/tableview_treeview_delegate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/tableview/tableview_column_configuration_dialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/tableview/tableview_model.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/tableview/tableview_columns.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/tableview/tableview_column_audiovideo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/tableview/tableview_column_file.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/tableview/tableview_column_geo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/tableview/tableview_column_digikam.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/tableview/tableview_column_item.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/tableview/tableview_column_photo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/tableview/tableview_column_thumbnail.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/tableview/tableview_columnfactory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/tableview/tableview_selection_model_syncer.cpp
       )

    if(KFACE_FOUND)
        set(libdigikamgui_SRCS
            ${libdigikamgui_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/app/items/digikamimagefacedelegate.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/app/items/overlays/assignnameoverlay.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/app/items/overlays/facerejectionoverlay.cpp
           )
    endif()

    if(KGEOMAP_FOUND)
        set(libdigikamgui_SRCS
            ${libdigikamgui_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/app/views/mapwidgetview.cpp
           )
    endif()

    set(libdigikamhelpers_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/app/utils/uifilevalidator.cpp
       )

    set(libalbumsqlite2_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/albumdb_sqlite2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/upgradedb_sqlite2tosqlite3.cpp
       )


    if(ENABLE_DIGIKAM_MODELTEST)
        message(STATUS "Modeltest enabled")
        set(libdigikamgui_SRCS ${libdigikamgui_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/modeltest/modeltest.cpp
           )
        add_definitions(-DENABLE_DIGIKAM_MODELTEST)
    endif()

    include_directories(
        ${DATABASESERVERDIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/app
        ${CMAKE_CURRENT_SOURCE_DIR}/app/album
        ${CMAKE_CURRENT_SOURCE_DIR}/app/database
        ${CMAKE_CURRENT_SOURCE_DIR}/app/date
        ${CMAKE_CURRENT_SOURCE_DIR}/app/dragdrop
        ${CMAKE_CURRENT_SOURCE_DIR}/app/filters
        ${CMAKE_CURRENT_SOURCE_DIR}/app/items
        ${CMAKE_CURRENT_SOURCE_DIR}/app/items/overlays
        ${CMAKE_CURRENT_SOURCE_DIR}/app/main
        ${CMAKE_CURRENT_SOURCE_DIR}/app/fileaction
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/tagsmanager
        ${CMAKE_CURRENT_SOURCE_DIR}/app/tags/tagsmanager/models
        ${CMAKE_CURRENT_SOURCE_DIR}/app/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/app/settings
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views
        ${CMAKE_CURRENT_SOURCE_DIR}/app/views/tableview
        ${CMAKE_CURRENT_SOURCE_DIR}/imageplugins
        ${CMAKE_CURRENT_SOURCE_DIR}/imageplugins/color
        ${CMAKE_CURRENT_SOURCE_DIR}/imageplugins/decorate
        ${CMAKE_CURRENT_SOURCE_DIR}/imageplugins/enhance
        ${CMAKE_CURRENT_SOURCE_DIR}/imageplugins/enhance/hotpixels
        ${CMAKE_CURRENT_SOURCE_DIR}/imageplugins/filters
        ${CMAKE_CURRENT_SOURCE_DIR}/imageplugins/transform
        ${CMAKE_CURRENT_SOURCE_DIR}/kioslave
        ${CMAKE_CURRENT_SOURCE_DIR}/libs
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/dklcms
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/cimg
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/kmemoryinfo
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/3rdparty/kinotify
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/core
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/haar
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imgqsort
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
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/progressmanager
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/threads
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/versionmanager
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/notificationmanager
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/mainview
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/graphicsview
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/itemview
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/iccprofiles
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/imagehistory
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
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/importui
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/importui/backend
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/importui/dialogs
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/importui/items
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/importui/models
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/importui/views
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/importui/widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/importui/main
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/facemanagement
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/firstrun
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/fuzzysearch
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/gpssearch
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/hotplug
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/core
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/main
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/dialogs
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/plugin
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/printiface
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/rawimport
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/kipiiface
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/lighttable
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/baloo
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
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/searchwindow
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/slideshow

        # version.h, gitversion.h config-digikam.h
        ${CMAKE_CURRENT_BINARY_DIR}/app/utils

        # dbconfigversion.h
        ${CMAKE_CURRENT_BINARY_DIR}/app/database

        # external library paths
        ${JPEG_INCLUDE_DIR}
        ${PNG_PNG_INCLUDE_DIR}
        ${TIFF_INCLUDE_DIR}
        ${LCMS_INCLUDE_DIR}
    )

    add_subdirectory(data)
    add_subdirectory(utilities)
    add_subdirectory(app)
    add_subdirectory(kioslave)
    add_subdirectory(showfoto)
    add_subdirectory(imageplugins)

    if(ENABLE_INTERNALMYSQL)
        add_subdirectory(databaseserver)
    endif()

    if(KDE4_BUILD_TESTS)
        MACRO_OPTIONAL_ADD_SUBDIRECTORY(tests)
    endif()

    if(NOT WIN32)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/digikam.lsm.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/digikam.lsm)
    endif()

endif()

# ==================================================================================================
# API documentation generation

if(DOXYGEN_FOUND)

    set(API_DIR    ${CMAKE_BINARY_DIR}/api)
    set(SOURCE_DIR ${CMAKE_SOURCE_DIR})
    set(DOXYFILE   ${CMAKE_BINARY_DIR}/Doxyfile)
    set(WARNFILE   ${CMAKE_BINARY_DIR}/doxygen-warn.log)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/Doxyfile.cmake.in ${DOXYFILE})

    add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
                      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
# Copyright (c) 2015      by Mohamed Anwer, <m dot anwer at gmx dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

project(digikam)

message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "Starting CMake configuration for: ${PROJECT_NAME}")

# ==============================================================================
# Information to update before to release this package.

# digiKam version
set(DIGIKAM_MAJOR_VERSION "5")
set(DIGIKAM_MINOR_VERSION "9")
set(DIGIKAM_PATCH_VERSION "0")

# Suffix to add at end of version string. Usual values are:
# "-git"   : alpha code unstable from git. Do not use in production
# "-beta1" : beta1 release.
# "-beta2" : beta2 release.
# "-beta3" : beta3 release.
# "-rc"    : release candidate.
# ""       : final release. Can be used in production.
set(DIGIKAM_SUFFIX_VERSION "")

# ==============================================================================
# Set env. variables accordingly.

# NOTE: This string is used to set libdigikamcore and libdigikamdatabase SO version ID
set(DIGIKAM_VERSION_SHORT
    "${DIGIKAM_MAJOR_VERSION}.${DIGIKAM_MINOR_VERSION}.${DIGIKAM_PATCH_VERSION}"
)

set(DIGIKAM_VERSION_STRING
    "${DIGIKAM_VERSION_SHORT}${DIGIKAM_SUFFIX_VERSION}"
)

# Core Database XML version
# We must set this variable here at top level because it is used in both
# libs/database/core and data/database
# Version history:
# 1 : Original database XML file, published in production.
# 2 : 08-08-2014 : Fix Images.names field size (see bug #327646).
# 3 : 05/11/2015 : Add Face DB schema.
set(DBCORECONFIG_XML_VERSION "3")

# ==============================================================================

set(CMAKE_MIN_VERSION    "3.0.0")
set(ECM_MIN_VERSION      "1.7.0")
set(KF5_MIN_VERSION      "5.1.0")
set(QT_MIN_VERSION       "5.6.0")
set(KIPI_MIN_VERSION     "5.0.0")
set(KSANE_MIN_VERSION    "5.0.0")
set(LENSFUN_MIN_VERSION  "0.2.6.0")
set(EXIV2_MIN_VERSION    "0.26")
set(AKONADI_MIN_VERSION  "4.89.0")        # Akonadi Contact dependency
set(CALENDAR_MIN_VERSION "4.81.0")        # Calendar Core dependency
set(QTAV_MIN_VERSION     "1.12.0")

cmake_minimum_required(VERSION ${CMAKE_MIN_VERSION})

############## ECM setup ######################

find_package(ECM ${ECM_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${digikam_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

# Cmake macros
include(GenerateExportHeader)
include(CheckFunctionExists)
include(FeatureSummary)
# ECM macros
include(ECMOptionalAddSubdirectory)
include(ECMAddTests)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(ECMSetupVersion)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMPoQmTools)
# KDE macros
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)
include(KDECompilerSettings)
# Local macros
include(MacroUtils)
include(MacroLocalLibs)
include(MacroOpenCV)
include(MacroJPEG)
include(MacroBoolTo01)

# ==============================================================================

option(ENABLE_OPENCV3               "Build digiKam with OpenCV3 instead OpenCV2 (default=ON)"                            ON)
option(ENABLE_KFILEMETADATASUPPORT  "Build digiKam with KDE files indexer support (default=OFF)"                         OFF)
option(ENABLE_AKONADICONTACTSUPPORT "Build digiKam with KDE Mail Contacts support (default=OFF)"                         OFF)
option(ENABLE_MEDIAPLAYER           "Build digiKam with Media Player support (default=OFF)"                              OFF)
option(ENABLE_DBUS                  "Build digiKam with DBUS support (default=ON)"                                       ON)
option(ENABLE_APPSTYLES             "Build digiKam with support for changing the widget application style (default=OFF)" OFF)

# Mysql support options (experimental):
option(ENABLE_MYSQLSUPPORT          "Build digiKam with MySQL dabatase support (default=ON)"                             ON)
option(ENABLE_INTERNALMYSQL         "Build digiKam with internal MySQL server executable (default=ON)"                   ON)


# Debug options:
option(ENABLE_DIGIKAM_MODELTEST     "Enable ModelTest on some models for debugging (default=OFF)"                        OFF)

############## Find Packages ###################

find_package(Qt5 ${QT_MIN_VERSION}
             NO_MODULE COMPONENTS
             Core
             Concurrent
             Widgets
             Gui
             Sql
             Xml
             PrintSupport
             WebKitWidgets
             Network
)

find_package(Qt5 ${QT_MIN_VERSION}
             OPTIONAL_COMPONENTS
             DBus
             OpenGL
)

if(ENABLE_DBUS)
    if(NOT Qt5DBus_FOUND)
        set(ENABLE_DBUS OFF)
    endif()
endif()

if(BUILD_TESTING)
    find_package(Qt5 ${QT_MIN_VERSION} NO_MODULE COMPONENTS Test)
endif()

find_package(KF5 ${KF5_MIN_VERSION}
             COMPONENTS
             XmlGui
             CoreAddons
             Config
             Service
             WindowSystem
             Solid
             I18n
)

find_package(KF5 ${KF5_MIN_VERSION} QUIET
             OPTIONAL_COMPONENTS
             KIO                    # For Desktop integration (Widgets only).
             IconThemes             # For Desktop integration.
             FileMetaData           # For KDE file indexer support.
             ThreadWeaver           # For Panorama tool.
             NotifyConfig           # KDE desktop application notify configuration.
             Notifications          # KDE desktop notifications integration.
)

find_package(KF5 ${AKONADI_MIN_VERSION} QUIET
             OPTIONAL_COMPONENTS
             AkonadiContact         # For KDE Mail Contacts support.
)

find_package(KF5 ${CALENDAR_MIN_VERSION} QUIET
             OPTIONAL_COMPONENTS
             CalendarCore           # For Calendar tool.
)
if ("${KF5CalendarCore_VERSION}" VERSION_GREATER 5.6.40)
    set(HAVE_KCALENDAR_QDATETIME TRUE)
endif()

if(ENABLE_AKONADICONTACTSUPPORT AND NOT KF5AkonadiContact_FOUND)
    set(ENABLE_AKONADICONTACTSUPPORT OFF)
endif()

if(ENABLE_KFILEMETADATASUPPORT AND NOT KF5FileMetaData_FOUND)
    set(ENABLE_KFILEMETADATASUPPORT OFF)
endif()

# Check if KIO have been compiled with KIOWidgets. digiKam only needs this one.
if(KF5KIO_FOUND)
    get_target_property(KIOWidgets_INCLUDE_DIRS KF5::KIOWidgets
                        INTERFACE_INCLUDE_DIRECTORIES)
    message(STATUS "KF5::KIOWidgets include dirs: ${KIOWidgets_INCLUDE_DIRS}")

    if(NOT KIOWidgets_INCLUDE_DIRS)
        message(STATUS "KF5::KIOWidgets not available in shared KIO library. KIO support disabled.")
        set(KF5KIO_FOUND FALSE)
    endif()
endif()

# ==============================================================================
# Dependencies Rules

# mandatory

DETECT_JPEG()
set(DIGIKAM_LIBJPEG_DIR libjpeg-${JPEG_LIB_VERSION})

find_package(TIFF)
find_package(PNG)
find_package(Boost)
find_package(LCMS2)
find_package(EXPAT)    # For DNGWriter: XMP SDK need Expat library to compile.
find_package(Threads)  # For DNGWriter and LibRaw which needs native threads support.

find_package(Exiv2 ${EXIV2_MIN_VERSION})

set_package_properties("Exiv2"     PROPERTIES
                       DESCRIPTION "Required to build digiKam"
                       URL         "http://www.exiv2.org"
                       TYPE        RECOMMENDED
                       PURPOSE     "Library to manage image metadata"
)

# -- check Media player --------------------------------------------------------

find_package(QtAV)

if(ENABLE_MEDIAPLAYER)

    if(NOT ${QtAV_FOUND} OR ${QTAV_VERSION_STRING} VERSION_LESS ${QTAV_MIN_VERSION})

        set(ENABLE_MEDIAPLAYER OFF)
        set(QtAV_FOUND OFF)
        message(STATUS "ENABLE_MEDIAPLAYER option is enabled but QtAV cannot be found. Media player support is disabled.")

    else()

        include_directories(${QTAV_INCLUDE_DIRS})
        message(STATUS "Media player support is enabled.")

    endif()

endif()

# -- check OpenCV --------------------------------------------------------------

if(ENABLE_OPENCV3)

    set(OPENCV_MIN_VERSION "3.1.0")
    DETECT_OPENCV(${OPENCV_MIN_VERSION} core objdetect imgproc imgcodecs face)

    if(${OpenCV_FOUND})
        if(${OpenCV_VERSION} VERSION_LESS 3.1.0)
            message(STATUS "ENABLE_OPENCV3 option is enabled and OpenCV < 3.1.0 have been found. Disabled ENABLE_OPENCV3")
            set(OpenCV_FOUND FALSE)
        endif()
    endif()

else()

    set(OPENCV_MIN_VERSION "2.4.9")
    DETECT_OPENCV(${OPENCV_MIN_VERSION} core objdetect imgproc contrib legacy)

    if(${OpenCV_FOUND})
        if(${OpenCV_VERSION} VERSION_GREATER 2.4.99)
            message(STATUS "ENABLE_OPENCV3 option is disabled and OpenCV >= 3.0.0 have been found. Enabled ENABLE_OPENCV3")
            set(OpenCV_FOUND FALSE)
        endif()
    endif()

endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/app/utils/libopencv.h.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/app/utils/libopencv.h)

# -- optionals -----------------------------------------------------------------

find_package(FLEX)                                 # For Panorama tool.
find_package(BISON)                                # For Panorama tool.

find_package(LibXslt)                              # For HTMLGallery tool.
find_package(LibXml2)                              # For HTMLGallery tool.

DETECT_LIBKIPI(${KIPI_MIN_VERSION})                # For plugins support.
DETECT_LIBKSANE(${KSANE_MIN_VERSION})              # For digital scanner device support.

find_package(Marble)                               # For geolocation support.

find_package(PkgConfig)
find_package(Jasper)                               # For JPEG 2000 support.
find_package(Eigen3)                               # For Refocus tool.
find_package(OpenGL)                               # For Presentation tool.

# For Monitor Profiles management with LCMS
find_package(X11)

if(X11_FOUND)
    find_package(Qt5 ${QT_MIN_VERSION} NO_MODULE COMPONENTS X11Extras)
    set(HAVE_X11 TRUE)
else()
    set(HAVE_X11 FALSE)
endif()

# decide if Presentation tool can be built with OpenGL
if(OPENGL_FOUND AND OPENGL_GLU_FOUND AND Qt5OpenGL_FOUND)
    set(HAVE_OPENGL TRUE)
elseif()
    set(HAVE_OPENGL FALSE)
endif()

# For LibRaw
if(WIN32)
    find_library(WSOCK32_LIBRARY wsock32)
    find_library(WS2_32_LIBRARY ws2_32)
endif()

# -- To link under Solaris (see bug #274484) -----------------------------------

if(NOT WIN32)
    find_library(MATH_LIBRARY m)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
    find_library(KVM_LIBRARY kvm)
endif()

# ==============================================================================
# More Optional Dependencies

find_package(Doxygen)
find_package(Lqr-1)

# -- libgphoto2 rules ----------------------------------------------------------

if(NOT WIN32)

    find_package(Gphoto2)

    if(Gphoto2_FOUND)

        if("${GPHOTO2_VERSION_STRING}" VERSION_GREATER "2.4.0")
            set(VERSION_GPHOTO2 true)
        else()
            set(VERSION_GPHOTO2 false)
        endif()

        if("${GPHOTO2_VERSION_STRING}" VERSION_GREATER "2.5.0")
            set(VERSION_GPHOTO25 true)
            message(STATUS "libgphoto2 API version >= 2.5")
        else()
            set(VERSION_GPHOTO25 false)
            message(STATUS "libgphoto2 API version < 2.5")
        endif()

        if(VERSION_GPHOTO25)
            set(HAVE_GPHOTO25 1)
        else()
            set(HAVE_GPHOTO25 0)
        endif()

    endif()

endif()

# -- Check LensFun library for Lens auto-correction tool -----------------------

find_package(LensFun)

if(LENSFUN_VERSION)
    message(STATUS "liblensfun: Found version ${LENSFUN_VERSION} (required: ${LENSFUN_MIN_VERSION})")
    if(${LENSFUN_VERSION} VERSION_LESS ${LENSFUN_MIN_VERSION})
        set(LensFun_FOUND FALSE)
    endif()
else()
    message(STATUS "liblensfun: Version information not found, your version is probably too old.")
    set(LensFun_FOUND FALSE)
endif()

# -- Check dependencies for Panorama tool --------------------------------------

if(FLEX_FOUND AND BISON_FOUND AND KF5ThreadWeaver_FOUND)
    set(HAVE_PANORAMA 1)
else()
    set(HAVE_PANORAMA 0)
endif()

# -- Check dependencies for HTMLGallery tool -----------------------------------

if(LibXml2_FOUND AND LibXslt_FOUND)
    set(HAVE_HTMLGALLERY 1)
else()
    set(HAVE_HTMLGALLERY 0)
endif()

# -- compilation options definitions -------------------------------------------

MACRO_BOOL_TO_01(KF5Kipi_FOUND           HAVE_KIPI)
MACRO_BOOL_TO_01(KF5Sane_FOUND           HAVE_KSANE)
MACRO_BOOL_TO_01(KF5FileMetaData_FOUND   HAVE_KFILEMETADATA)
MACRO_BOOL_TO_01(KF5AkonadiContact_FOUND HAVE_AKONADICONTACT)
MACRO_BOOL_TO_01(KF5CalendarCore_FOUND   HAVE_KCALENDAR)
MACRO_BOOL_TO_01(KF5Notifications_FOUND  HAVE_KNOTIFICATIONS)
MACRO_BOOL_TO_01(KF5NotifyConfig_FOUND   HAVE_KNOTIFYCONFIG)
MACRO_BOOL_TO_01(KF5KIO_FOUND            HAVE_KIO)
MACRO_BOOL_TO_01(KF5IconThemes_FOUND     HAVE_KICONTHEMES)
MACRO_BOOL_TO_01(LensFun_FOUND           HAVE_LENSFUN)
MACRO_BOOL_TO_01(Lqr-1_FOUND             HAVE_LIBLQR_1)
MACRO_BOOL_TO_01(Gphoto2_FOUND           HAVE_GPHOTO2)
MACRO_BOOL_TO_01(Jasper_FOUND            HAVE_JASPER)
MACRO_BOOL_TO_01(Eigen3_FOUND            HAVE_EIGEN3)
MACRO_BOOL_TO_01(Marble_FOUND            HAVE_MARBLE)
MACRO_BOOL_TO_01(ENABLE_MYSQLSUPPORT     HAVE_MYSQLSUPPORT)
MACRO_BOOL_TO_01(ENABLE_INTERNALMYSQL    HAVE_INTERNALMYSQL)
MACRO_BOOL_TO_01(ENABLE_MEDIAPLAYER      HAVE_MEDIAPLAYER)
MACRO_BOOL_TO_01(ENABLE_DBUS             HAVE_DBUS)
MACRO_BOOL_TO_01(ENABLE_APPSTYLES        HAVE_APPSTYLE_SUPPORT)

# Whether to use Qt's scaling to downscale previews. Under MacOSX, Qt
# can make use of the higher physical resolution of Retina
# displays. However, it seems that other Qt renderers perform badly at
# this, so disable. If other renderers switch to coarser logical
# pixels, one probably needs this feature on these platforms as well.
MACRO_BOOL_TO_01(APPLE USE_QT_SCALING)

# Set config files accordingly with optional dependencies

configure_file(app/utils/digikam_config.h.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/app/utils/digikam_config.h)

# ==============================================================================
# Log messages

message(STATUS "")
message(STATUS "----------------------------------------------------------------------------------")
message(STATUS " digiKam ${DIGIKAM_VERSION_STRING} dependencies results   <http://www.digikam.org>")
message(STATUS "")

PRINT_COMPONENT_COMPILE_STATUS("MySQL Database Support" ENABLE_MYSQLSUPPORT)
PRINT_COMPONENT_COMPILE_STATUS("MySQL Internal Support" ENABLE_INTERNALMYSQL)
PRINT_COMPONENT_COMPILE_STATUS("DBUS Support"           ENABLE_DBUS)
PRINT_COMPONENT_COMPILE_STATUS("App. Style Support"     ENABLE_APPSTYLES)

# ==============================================================================

PRINT_LIBRARY_STATUS("libboostgraph" "http://www.boost.org/doc/libs"                    "(version >= 1.43.0)"                Boost_FOUND)
PRINT_LIBRARY_STATUS("libexiv2"      "http://www.exiv2.org"                             "(version >= ${EXIV2_MIN_VERSION}"   Exiv2_FOUND)
PRINT_LIBRARY_STATUS("libexpat"      "http://expat.sourceforge.net"                     "(version >= 2.0.0)"                 EXPAT_FOUND)
PRINT_LIBRARY_STATUS("libjpeg"       "http://www.ijg.org"                               "(version >= 6b)"                    JPEG_FOUND)
PRINT_LIBRARY_STATUS("libkde"        "http://www.kde.org"                               "(version >= ${KF5_MIN_VERSION})"    KF5_FOUND)
PRINT_LIBRARY_STATUS("liblcms"       "http://www.littlecms.com"                         "(version >= 1.0.0)"                 LCMS2_FOUND)
PRINT_LIBRARY_STATUS("libopencv"     "http://opencv.willowgarage.com"                   "(version >= ${OPENCV_MIN_VERSION})" OpenCV_FOUND)
PRINT_LIBRARY_STATUS("libpng"        "http://www.libpng.org/pub/png/libpng.html"        "(version >= 1.2.7)"                 PNG_FOUND)
PRINT_LIBRARY_STATUS("libpthread"    "http://www.gnu.org/software/hurd/libpthread.html" "(version >= 2.0.0)"                 CMAKE_USE_PTHREADS_INIT OR CMAKE_USE_WIN32_THREADS_INIT)
PRINT_LIBRARY_STATUS("libqt"         "http://www.qtsoftware.com"                        "(version >= ${QT_MIN_VERSION})"     Qt5_FOUND)
PRINT_LIBRARY_STATUS("libtiff"       "http://www.remotesensing.org/libtiff"             "(version >= 3.8.2)"                 TIFF_FOUND)

# ==============================================================================

PRINT_OPTIONAL_LIBRARY_STATUS("bison"             "https://www.gnu.org/software/bison/"                             "(version >= 2.5.0)"                   "digiKam will be compiled without Panorama support."                          BISON_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("doxygen"           "http://www.doxygen.org"                                          "(version >= 1.8.0)"                   "digiKam will be compiled without API documentation building support."        Doxygen_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("flex"              "http://flex.sourceforge.net/"                                    "(version >= 2.5.0)"                   "digiKam will be compiled without Panorama support."                          FLEX_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libakonadicontact" "https://projects.kde.org/projects/kde/kdepimlibs"                "(version >= ${AKONADI_MIN_VERSION})"  "digiKam will be compiled without KDE address book support."                  KF5AkonadiContact_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libeigen3"         "http://eigen.tuxfamily.org"                                      "(version >= 3.0.0)"                   "digiKam will be compiled without Refocus tool support."                      Eigen3_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libgphoto2"        "http://www.gphoto.org"                                           "(version >= 2.4.0)"                   "digiKam will be compiled without GPhoto2 camera drivers support."            Gphoto2_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libjasper"         "http://www.ece.uvic.ca/~mdadams/jasper"                          "(version >= 1.7.0)"                   "digiKam will be compiled without JPEG2000 support."                          Jasper_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libkcalcore"       "https://projects.kde.org/projects/kde/pim/kcalcore"              "(version >= ${CALENDAR_MIN_VERSION})" "digiKam will be compiled without KDE calendar support."                      KF5CalendarCore_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libkfilemetadata"  "https://projects.kde.org/projects/kde/kdelibs/baloo"             "(version >= ${KF5_MIN_VERSION})"      "digiKam will be compiled without KDE file metadata support."                 KF5FileMetaData_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libkiconthemes"    "https://projects.kde.org/projects/frameworks/kiconthemes"        "(version >= ${KF5_MIN_VERSION})"      "digiKam will be compiled without KDE desktop icon themes support."           KF5IconThemes_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libkio"            "https://projects.kde.org/projects/frameworks/kio"                "(version >= ${KF5_MIN_VERSION})"      "digiKam will be compiled without KDE desktop integration support."           KF5KIO_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libkipi"           "https://projects.kde.org/projects/kde/kdegraphics/libs/libkipi"  "(version >= ${KIPI_MIN_VERSION})"     "digiKam will be compiled without Kipi-plugins support."                      KF5Kipi_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libknotifications" "https://projects.kde.org/projects/frameworks/knotifications"     "(version >= ${KF5_MIN_VERSION})"      "digiKam will be compiled without KDE desktop notifications support."         KF5Notifications_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libknotifyconfig"  "https://projects.kde.org/projects/frameworks/knotifyconfig"      "(version >= ${KF5_MIN_VERSION})"      "digiKam will be compiled without KDE desktop notify configuration support."  KF5NotifyConfig_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libksane"          "https://projects.kde.org/projects/kde/kdegraphics/libs/libksane" "(version >= ${KSANE_MIN_VERSION})"    "digiKam will be compiled without libksane support."                          KF5Sane_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("liblensfun"        "http://lensfun.sourceforge.net"                                  "(version >= 0.2.6)"                   "digiKam will be compiled without Lens Auto Correction tool support."         LensFun_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("liblqr-1"          "http://liblqr.wikidot.com"                                       "(version >= 0.4.1)"                   "digiKam will be compiled without Contents Aware Resizer tool support."       Lqr-1_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libmarble"         "https://projects.kde.org/projects/kde/kdeedu/marble"             "(version >= 0.22.0)"                  "digiKam will be compiled without geolocation maps support."                  Marble_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libqtav"           "http://www.qtav.org/"                                            "(version >= ${QTAV_MIN_VERSION})"     "digiKam will be compiled without Media Player support."                      QtAV_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libthreadweaver"   "https://projects.kde.org/projects/frameworks/threadweaver"       "(version >= ${KF5_MIN_VERSION})"      "digiKam will be compiled without Panorama support."                          KF5ThreadWeaver_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libxml2"           "http://xmlsoft.org"                                              "(version >= 2.7.0)"                    "digiKam will be compiled without HTMLGallery support."                      LibXml2_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libxslt"           "http://xmlsoft.org/XSLT"                                         "(version >= 1.1.0)"                    "digiKam will be compiled without HTMLGallery support."                      LibXslt_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("OpenGL"            "http://www.mesa3d.org"                                           "(version >= 11.0.0)"                  "digiKam will be compiled without OpenGL support."                            HAVE_OPENGL)

# ==============================================================================

if(Boost_FOUND                          AND
   Exiv2_FOUND                          AND
   EXPAT_FOUND                          AND
   JPEG_FOUND                           AND
   KF5_FOUND                            AND
   LCMS2_FOUND                          AND
   OpenCV_FOUND                         AND
   PNG_FOUND                            AND
   Qt5_FOUND                            AND
   TIFF_FOUND                           AND
   Threads_FOUND                        AND
   (Qt5Test_FOUND OR NOT BUILD_TESTING) AND
   (Qt5X11Extras_FOUND OR NOT HAVE_X11) AND
   (CMAKE_USE_PTHREADS_INIT OR CMAKE_USE_WIN32_THREADS_INIT)
  )
    message(STATUS " digiKam can be compiled.................. YES")
    set(DIGIKAM_CAN_BE_COMPILED true)
else()
    message(FATAL_ERROR " digiKam will be compiled.................. NO  (See the README file for more details about dependencies)")
    set(DIGIKAM_CAN_BE_COMPILED false)
endif()

message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "")

if(DIGIKAM_CAN_BE_COMPILED)

    # ==========================================================================
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
    add_custom_target(digikam-gitversion ALL COMMAND ${CMAKE_COMMAND} -P
                      "${CMAKE_CURRENT_BINARY_DIR}/gitscript.cmake")

    # ==========================================================================
    # To prevent warnings from M$ compiler

    if(WIN32 AND MSVC)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        add_definitions(-D_ATL_SECURE_NO_WARNINGS)
        add_definitions(-D_AFX_SECURE_NO_WARNINGS)
    endif()

    # ==========================================================================
    # Definitions rules

    # Remove pedantic GCC flag which generate a lots of warnings on the console
    # with qCDebug()
    while(CMAKE_CXX_FLAGS MATCHES "-pedantic")
        string(REPLACE "-pedantic" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endwhile()

    # Remove Wsuggest-override GCC flag which generate a lots of compile warnings
    while(CMAKE_CXX_FLAGS MATCHES "-Wsuggest-override")
        string(REPLACE "-Wsuggest-override" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endwhile()

    # Remove Wdate-time GCC flag which generate a lots of compile warnings
    while(CMAKE_CXX_FLAGS MATCHES "-Wdate-time")
        string(REPLACE "-Wdate-time" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endwhile()

    # translations catalog
    add_definitions(-DTRANSLATION_DOMAIN=\"digikam\")

    # NOTE: with libpgf 6.11.24 OpenMP is not very well supported. We disable
    # it to be safe. See B.K.O #273765 for details.
    add_definitions(-DLIBPGF_DISABLE_OPENMP)

    # To force to declare as exported classes in all sub components.
    # This will only done near to library declaration in Cmake files.
    if(WIN32)
        add_definitions(-Ddigikamcore_EXPORTS)
        add_definitions(-Ddigikamdatabase_EXPORTS)
    endif()

    # Enable C++ Exceptions support, require by Greycstoration algorithm
    # (CImg.h) and PGF codec
    kde_enable_exceptions()

    # ==========================================================================
    # Includes rules

    # Recursively get all directories which contain header files
    set(DK_INCLUDES_ALL "")

    HEADER_DIRECTORIES(DK_LOCAL_INCLUDES_RAW)

    # This macro will set all paths which do not contain libjpeg-
    # We will add later the directory we need

    foreach(var ${DK_LOCAL_INCLUDES_RAW})
        string(REGEX MATCH "libjpeg-" item ${var})
        if(item STREQUAL "")
            list(APPEND DK_LOCAL_INCLUDES ${var})
        endif()
    endforeach()

    set(DK_LOCAL_INCLUDES ${DK_LOCAL_INCLUDES}
                          libs/jpegutils/${DIGIKAM_LIBJPEG_DIR})

    include_directories(${DK_LOCAL_INCLUDES})

    # for config headers digikam_version.h gitversion.h digikam_config.h
    # digikam_dbconfig.h libopencv.h
    include_directories(${CMAKE_CURRENT_BINARY_DIR}/app/utils)

    include_directories(${OpenCV_INCLUDE_DIRS})

    # ==========================================================================
    # Common targets parts

    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    add_subdirectory(data)
    add_subdirectory(libs)
    add_subdirectory(utilities)
    add_subdirectory(app)
    add_subdirectory(showfoto)

    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()

endif()

# ==============================================================================
# API documentation generation

if(Doxygen_FOUND)

    set(API_DIR    ${CMAKE_BINARY_DIR}/api)
    set(SOURCE_DIR ${CMAKE_SOURCE_DIR})
    set(DOXYFILE   ${CMAKE_BINARY_DIR}/Doxyfile)
    set(WARNFILE   ${CMAKE_BINARY_DIR}/doxygen-warn.log)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/Doxyfile.cmake.in
                   ${DOXYFILE})

    add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
                      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

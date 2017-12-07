#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

kde_enable_exceptions()

# Boost uses operator names (and, not, ...)
string(REPLACE "-fno-operator-names" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(libdimg_SRCS
    dimg.cpp
    drawdecoding.cpp
    dimgscale.cpp
    dcolor.cpp
    dcolorcomposer.cpp
    imagehistory/dimagehistory.cpp
    imagehistory/filteraction.cpp
    imagehistory/historyimageid.cpp
)

set(libdimgfilters_SRCS
    dklcms/digikam-lcms.cpp
    filters/dimgbuiltinfilter.cpp
    filters/dimgthreadedfilter.cpp
    filters/dimgthreadedanalyser.cpp
    filters/dimgfiltermanager.cpp
    filters/dimgfiltergenerator.cpp
    filters/filteractionfilter.cpp
    filters/randomnumbergenerator.cpp
    filters/rawprocessingfilter.cpp
    filters/decorate/borderfilter.cpp
    filters/decorate/bordersettings.cpp
    filters/decorate/texturefilter.cpp
    filters/film/filmfilter.cpp
    filters/fx/blurfilter.cpp
    filters/fx/blurfxfilter.cpp
    filters/fx/colorfxfilter.cpp
    filters/fx/colorfxsettings.cpp
    filters/fx/distortionfxfilter.cpp
    filters/fx/charcoalfilter.cpp
    filters/fx/embossfilter.cpp
    filters/fx/filmgrainfilter.cpp
    filters/fx/filmgrainsettings.cpp
    filters/fx/invertfilter.cpp
    filters/fx/pixelsaliasfilter.cpp
    filters/fx/oilpaintfilter.cpp
    filters/fx/raindropfilter.cpp
    filters/auto/autolevelsfilter.cpp
    filters/auto/autoexpofilter.cpp
    filters/auto/equalizefilter.cpp
    filters/auto/stretchfilter.cpp
    filters/auto/normalizefilter.cpp
    filters/cb/cbfilter.cpp
    filters/cb/cbsettings.cpp
    filters/bcg/bcgfilter.cpp
    filters/bcg/bcgsettings.cpp
    filters/bw/bwsepiafilter.cpp
    filters/bw/bwsepiasettings.cpp
    filters/bw/tonalityfilter.cpp
    filters/bw/infraredfilter.cpp
    filters/bw/mixerfilter.cpp
    filters/bw/mixersettings.cpp
    filters/hsl/hslfilter.cpp
    filters/hsl/hslsettings.cpp
    filters/hsl/hspreviewwidget.cpp
    filters/icc/iccmanager.cpp
    filters/icc/iccprofile.cpp
    filters/icc/iccprofilesettings.cpp
    filters/icc/icctransform.cpp
    filters/icc/icctransformfilter.cpp
    filters/icc/iccsettingscontainer.cpp
    filters/icc/iccsettings.cpp
    filters/lc/localcontrastfilter.cpp
    filters/lc/localcontrastsettings.cpp
    filters/lc/localcontrastcontainer.cpp
    filters/nr/nrfilter.cpp
    filters/nr/nrestimate.cpp
    filters/nr/nrsettings.cpp
    filters/sharp/sharpenfilter.cpp
    filters/sharp/unsharpmaskfilter.cpp
    filters/sharp/sharpsettings.cpp
    filters/levels/imagelevels.cpp
    filters/levels/levelsfilter.cpp
    filters/levels/imagehistogram.cpp
    filters/levels/histogrambox.cpp
    filters/levels/histogramwidget.cpp
    filters/levels/histogrampainter.cpp
    filters/curves/curvescontainer.cpp
    filters/curves/imagecurves.cpp
    filters/curves/curvesfilter.cpp
    filters/curves/curvessettings.cpp
    filters/curves/curveswidget.cpp
    filters/curves/curvesbox.cpp
    filters/wb/wbcontainer.cpp
    filters/wb/wbfilter.cpp
    filters/wb/wbsettings.cpp
    filters/transform/freerotationfilter.cpp
    filters/transform/freerotationsettings.cpp
    filters/transform/shearfilter.cpp
    filters/transform/autocrop.cpp
    filters/greycstoration/greycstorationfilter.cpp
    filters/greycstoration/greycstorationsettings.cpp
    filters/lens/antivignettingfilter.cpp
    filters/lens/antivignettingsettings.cpp
    filters/lens/lensdistortionfilter.cpp
    filters/lens/pixelaccess.cpp
    filters/redeye/redeyecorrectionfilter.cpp
    filters/redeye/redeyecorrectionsettings.cpp
    filters/redeye/redeyecorrectioncontainer.cpp
)

# ==================================================================================================
# get the gcc version

# CImg.h version 1.2.8 do not compile fine with gcc 4.3.x
# See bug #163118: digikam-0.9.4_beta5 compilation hangs with gcc 4.3
# Using -fno-tree-pre is work around this problem.

# TODO is this hack anymore required?
if(CMAKE_COMPILER_IS_GNUCXX)
    exec_program(${CMAKE_CXX_COMPILER} ARGS ${CMAKE_CXX_COMPILER_ARG1} -dumpversion OUTPUT_VARIABLE GCC_VERSION)

    if (${GCC_VERSION} VERSION_LESS "4.3.0")
        message(STATUS "Adjusting compilation flags for GCC version (${GCC_VERSION} )")
        add_definitions(-fno-tree-pre)
    endif()
endif()


if(Lqr-1_FOUND)
    set(libdimgfilters_SRCS
        ${libdimgfilters_SRCS}
        filters/transform/contentawarefilter.cpp
    )
    include_directories(${LQR-1_INCLUDE_DIRS})
endif()

if(LensFun_FOUND)
    set(libdimgfilters_SRCS
        ${libdimgfilters_SRCS}
        filters/lens/lensfunfilter.cpp
        filters/lens/lensfuncameraselector.cpp
        filters/lens/lensfuniface.cpp
        filters/lens/lensfunsettings.cpp
    )
    include_directories(${LENSFUN_INCLUDE_DIRS})
endif()

if(Eigen3_FOUND)
    set(libdimgfilters_SRCS
        ${libdimgfilters_SRCS}
        filters/sharp/refocusfilter.cpp
        filters/sharp/matrix.cpp
    )
    include_directories(${EIGEN3_INCLUDE_DIR})
endif()

set(libdimgloaders_SRCS
    loaders/dimgloader.cpp
    loaders/pngloader.cpp
    loaders/jpegloader.cpp
    loaders/tiffloader.cpp
    loaders/rawloader.cpp
    loaders/ppmloader.cpp
    loaders/qimageloader.cpp
    loaders/pgfloader.cpp
    loaders/jpegsettings.cpp
    loaders/pngsettings.cpp
    loaders/tiffsettings.cpp
    loaders/pgfsettings.cpp
)

# JPEG2000 support

if(Jasper_FOUND)
    # this should be compiled separately to the lib, cleans if checks
    set(libdimgloaders_SRCS
        ${libdimgloaders_SRCS}
        loaders/jp2kloader.cpp
        loaders/jp2ksettings.cpp
    )
endif()

include_directories(
    $<TARGET_PROPERTY:Qt5::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::WidgetsAddons,INTERFACE_INCLUDE_DIRECTORIES>
)

if(X11_FOUND)
    include_directories($<TARGET_PROPERTY:Qt5::X11Extras,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

add_library(dimg_src OBJECT
            ${libdimgloaders_SRCS}
            ${libdimgfilters_SRCS}
            ${libdimg_SRCS}
            ${libdlib_SRCS}
)

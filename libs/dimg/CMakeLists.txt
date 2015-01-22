set(libdimg_SRCS
    dimg.cpp
    dimg_debug.cpp
    drawdecoding.cpp
    dimgscale.cpp
    dcolor.cpp
    dcolorcomposer.cpp
    imagehistory/dimagehistory.cpp
    imagehistory/filteraction.cpp
    imagehistory/historyimageid.cpp
)

set(libdimgfilters_SRCS
    ../dklcms/digikam-lcms.cpp
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
)

# ugly hack, but better than listing these in main cmakelists or polluting includes
include_directories(imagehistory/ 
                      filters/
                      filters/decorate/
                      filters/film/
                      filters/fx/
                      filters/auto/
                      filters/cb/
                      filters/bcg/
                      filters/bw/
                      filters/hsl/
                      filters/icc/
                      filters/lc/
                      filters/nr/
                      filters/sharp/
                      filters/levels/
                      filters/curves/
                      filters/wb/
                      filters/transform/
                      filters/greycstoration/
                      filters/lens/
                      loaders/
)

if(LQR-1_FOUND)
    set(libdimgfilters_SRCS
        ${libdimgfilters_SRCS}
        filters/transform/contentawarefilter.cpp
    )
endif()

if(LENSFUN_FOUND)
    set(libdimgfilters_SRCS
        ${libdimgfilters_SRCS}
        filters/lens/lensfunfilter.cpp
        filters/lens/lensfuncameraselector.cpp
        filters/lens/lensfuniface.cpp
        filters/lens/lensfunsettings.cpp
    )
    include_directories(${LENSFUN_INCLUDE_DIRS})
endif()

if(EIGEN3_FOUND)
    set(libdimgfilters_SRCS
        ${libdimgfilters_SRCS}
        filters/sharp/refocusfilter.cpp
        filters/sharp/matrix.cpp
    )
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

if(JASPER_FOUND)
    # this should be compiled separately to the lib, cleans if checks
    set(libdimgloaders_SRCS
        ${libdimgloaders_SRCS}
        loaders/jp2kloader.cpp
        loaders/jp2ksettings.cpp
    )
endif()

add_library(dimg STATIC ${libdimgloaders_SRCS} ${libdimgfilters_SRCS} ${libdimg_SRCS})

if(X11_FOUND)
    target_link_libraries(dimg Qt5::X11Extras ${X11_LIBRARIES})
endif()

if(JASPER_FOUND)
    target_link_libraries(dimg ${JASPER_LIBRARIES})
endif()


# LibLqr-1 library rules for content-aware filter
if(LQR-1_FOUND)
    include_directories(${LQR-1_INCLUDE_DIRS})
    set(liblqr_LIBS ${LQR-1_LIBRARIES})
endif()


target_link_libraries(dimg
                      dmetadata
                      dthread
                      jpegutils
                      
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Concurrent # filters
                      Qt5::Widgets
                      Qt5::Xml

                      KF5::KDELibs4Support # required for KColorButton
                      
                      ${LCMS_LIBRARIES} # filters
                      
                      ${TIFF_LIBRARIES}
                      ${PNG_LIBRARIES}
                      
                      KF5::KExiv2
                      KF5::KDcraw
)

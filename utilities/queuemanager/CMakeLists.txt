SET(libqueuemanager_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/actionthread.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/assignedlist.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/color/autocorrection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/color/bcgcorrection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/color/bwconvert.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/color/channelmixer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/color/colorbalance.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/color/convert16to8.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/color/convert8to16.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/color/curvesadjust.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/color/hslcorrection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/color/iccconvert.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/color/invert.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/color/whitebalance.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/convert/convert2jp2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/convert/convert2jpeg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/convert/convert2pgf.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/convert/convert2png.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/convert/convert2tiff.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/decorate/border.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/decorate/watermark.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/enhance/antivignetting.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/enhance/blur.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/enhance/localcontrast.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/enhance/noisereduction.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/enhance/restoration.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/enhance/sharpen.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/filters/filmgrain.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/metadata/assigntemplate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/metadata/removemetadata.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/transform/flip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/transform/resize.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/transform/rotate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/batchtool.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/batchtoolsmanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/queuelist.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/queuemgrwindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/queuepool.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/queuesettingsview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/queuetooltip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/toolsettingsview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/toolslistview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/toolsview.cpp
)

IF(GLIB2_FOUND)
    SET(libqueuemanager_SRCS ${libqueuemanager_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/basetools/enhance/lensautofix.cpp
    )
ENDIF(GLIB2_FOUND)

KDE4_ADD_LIBRARY(queuemanager STATIC ${libqueuemanager_SRCS})

INSTALL( FILES queuemgrwindowui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam )

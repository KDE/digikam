#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libqueuemanager_SRCS
    basetools/color/autocorrection.cpp
    basetools/color/bcgcorrection.cpp
    basetools/color/bwconvert.cpp
    basetools/color/channelmixer.cpp
    basetools/color/colorbalance.cpp
    basetools/color/convert16to8.cpp
    basetools/color/convert8to16.cpp
    basetools/color/curvesadjust.cpp
    basetools/color/hslcorrection.cpp
    basetools/color/iccconvert.cpp
    basetools/color/invert.cpp
    basetools/color/whitebalance.cpp
    basetools/convert/convert2jpeg.cpp
    basetools/convert/convert2pgf.cpp
    basetools/convert/convert2png.cpp
    basetools/convert/convert2tiff.cpp
    basetools/decorate/border.cpp
    basetools/decorate/watermark.cpp
    basetools/enhance/antivignetting.cpp
    basetools/enhance/blur.cpp
    basetools/enhance/localcontrast.cpp
    basetools/enhance/noisereduction.cpp
    basetools/enhance/restoration.cpp
    basetools/enhance/sharpen.cpp
    basetools/filters/filmgrain.cpp
    basetools/filters/colorfx.cpp
    basetools/metadata/assigntemplate.cpp
    basetools/metadata/removemetadata.cpp
    basetools/transform/flip.cpp
    basetools/transform/resize.cpp
    basetools/transform/rotate.cpp
    basetools/transform/crop.cpp
    manager/actionthread.cpp
    manager/task.cpp
    manager/batchtool.cpp
    manager/batchtoolutils.cpp
    manager/batchtoolsmanager.cpp
    manager/workflowmanager.cpp
    views/workflowdlg.cpp
    views/assignedlist.cpp
    views/queuelist.cpp
    views/queuepool.cpp
    views/queuesettingsview.cpp
    views/queuetooltip.cpp
    views/toolsettingsview.cpp
    views/toolslistview.cpp
    views/toolsview.cpp
    views/workflowlist.cpp
    main/queuemgrwindow.cpp
   )

if(JASPER_FOUND)
    set(libqueuemanager_SRCS ${libqueuemanager_SRCS}
        basetools/convert/convert2jp2.cpp
       )
endif()

if(LENSFUN_FOUND)
    set(libqueuemanager_SRCS ${libqueuemanager_SRCS}
        basetools/enhance/lensautofix.cpp
    )
    include_directories(${LENSFUN_INCLUDE_DIRS})
endif()

add_library(queuemanager STATIC ${libqueuemanager_SRCS})

# TODO instead of including subdirs, just modify the sources
include_directories(
    manager/
    basetools/
    basetools/color/
    basetools/convert/
    basetools/decorate/
    basetools/enhance/
    basetools/filters/
    basetools/metadata/
    basetools/transform/
    views/
    main/
)

target_include_directories(queuemanager PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/manager>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/views>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/main>
)

target_link_libraries(queuemanager


                      KF5::I18n # verify if needed
                      KF5::KExiv2
                      KF5::KDcraw
)

if(LENSFUN_FOUND)
    message(INFO " move lensfun to its own library")
    target_link_libraries(queuemanager dimg)
endif()
                     
install( FILES main/queuemgrwindowui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam )

#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libqueuemanager_SRCS
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
    ${CMAKE_CURRENT_SOURCE_DIR}/basetools/filters/colorfx.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/basetools/metadata/assigntemplate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/basetools/metadata/removemetadata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/basetools/transform/flip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/basetools/transform/resize.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/basetools/transform/rotate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/basetools/transform/crop.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/actionthread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/task.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/batchtool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/batchtoolutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/batchtoolsmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/workflowmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/workflowdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/assignedlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/queuelist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/queuepool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/queuesettingsview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/queuetooltip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/toolsettingsview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/toolslistview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/toolsview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/workflowlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main/queuemgrwindow.cpp
   )

if(JASPER_FOUND)
    set(libqueuemanager_SRCS ${libqueuemanager_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/convert/convert2jp2.cpp
       )
endif()

if(LENSFUN_FOUND)
    set(libqueuemanager_SRCS ${libqueuemanager_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/basetools/enhance/lensautofix.cpp
       )
endif()

KDE4_ADD_LIBRARY(queuemanager STATIC ${libqueuemanager_SRCS})

install( FILES main/queuemgrwindowui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam )

#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(libqueuemanager_SRCS
    tools/color/autocorrection.cpp
    tools/color/bcgcorrection.cpp
    tools/color/bwconvert.cpp
    tools/color/channelmixer.cpp
    tools/color/colorbalance.cpp
    tools/color/convert16to8.cpp
    tools/color/convert8to16.cpp
    tools/color/curvesadjust.cpp
    tools/color/hslcorrection.cpp
    tools/color/iccconvert.cpp
    tools/color/invert.cpp
    tools/color/whitebalance.cpp
    tools/convert/convert2jpeg.cpp
    tools/convert/convert2pgf.cpp
    tools/convert/convert2png.cpp
    tools/convert/convert2tiff.cpp
    tools/convert/convert2dng.cpp
    tools/custom/userscript.cpp
    tools/decorate/border.cpp
    tools/decorate/watermark.cpp
    tools/enhance/antivignetting.cpp
    tools/enhance/blur.cpp
    tools/enhance/localcontrast.cpp
    tools/enhance/noisereduction.cpp
    tools/enhance/restoration.cpp
    tools/enhance/sharpen.cpp
    tools/enhance/redeyecorrection.cpp
    tools/filters/filmgrain.cpp
    tools/filters/colorfx.cpp
    tools/metadata/assigntemplate.cpp
    tools/metadata/removemetadata.cpp
    tools/metadata/timeadjust.cpp
    tools/metadata/timeadjustcontainer.cpp
    tools/metadata/timeadjustsettings.cpp
    tools/metadata/clockphotodialog.cpp
    tools/metadata/detbyclockphotobutton.cpp
    tools/transform/flip.cpp
    tools/transform/resize.cpp
    tools/transform/rotate.cpp
    tools/transform/crop.cpp
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


if(Jasper_FOUND)
    set(libqueuemanager_SRCS ${libqueuemanager_SRCS}
        tools/convert/convert2jp2.cpp
       )
endif()

if(LensFun_FOUND)
    set(libqueuemanager_SRCS ${libqueuemanager_SRCS}
        tools/enhance/lensautofix.cpp
    )
    include_directories(${LENSFUN_INCLUDE_DIRS})
endif()

if(Gphoto2_FOUND)
    include_directories(${GPHOTO2_INCLUDE_DIRS})
endif()

include_directories($<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::WidgetsAddons,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::Service,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::Solid,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(queuemanager_src OBJECT ${libqueuemanager_SRCS} ${libdlib_SRCS})

install(FILES main/queuemgrwindowui5.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam)

#
# Copyright (c) 2010-2016 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

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
    basetools/convert/convert2dng.cpp
    basetools/custom/userscript.cpp
    basetools/decorate/border.cpp
    basetools/decorate/watermark.cpp
    basetools/enhance/antivignetting.cpp
    basetools/enhance/blur.cpp
    basetools/enhance/localcontrast.cpp
    basetools/enhance/noisereduction.cpp
    basetools/enhance/restoration.cpp
    basetools/enhance/sharpen.cpp
    basetools/enhance/redeyecorrection.cpp
    basetools/filters/filmgrain.cpp
    basetools/filters/colorfx.cpp
    basetools/metadata/assigntemplate.cpp
    basetools/metadata/removemetadata.cpp
    basetools/metadata/timeadjust.cpp
    basetools/metadata/timeadjustcontainer.cpp
    basetools/metadata/timeadjustsettings.cpp
    basetools/metadata/clockphotodialog.cpp
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

set(libdlib_SRCS
    ../../dlibdk/dlib/all/source.cpp
)

set(DLIB_INCLUDE_DIRS
    ../../dlibdk/
)

include_directories(${DLIB_INCLUDE_DIRS})

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

if(GPHOTO2_FOUND)
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

if(KF5Kipi_FOUND)
    include_directories($<TARGET_PROPERTY:KF5::Kipi,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

add_library(queuemanager_src OBJECT ${libqueuemanager_SRCS} ${libdlib_SRCS})

install( FILES main/queuemgrwindowui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam )

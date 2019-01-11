#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(libqueuemanager_SRCS
    manager/actionthread.cpp
    manager/task.cpp
    manager/batchtool.cpp
    manager/batchtoolutils.cpp
    manager/batchtoolsfactory.cpp
    manager/workflowmanager.cpp
    dplugins/dpluginbqm.cpp
    dplugins/dpluginbqmsetup.cpp
    dplugins/dpluginbqmconfview.cpp
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

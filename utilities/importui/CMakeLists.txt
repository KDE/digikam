#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories($<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::Solid,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::Service,INTERFACE_INCLUDE_DIRECTORIES>
)

if(KF5KIO_FOUND)
    include_directories($<TARGET_PROPERTY:KF5::KIOWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

if(Gphoto2_FOUND)
    include_directories(${GPHOTO2_INCLUDE_DIRS})
endif()

set(libimportuibackend_SRCS
    backend/cameracontroller.cpp
    backend/camerathumbsctrl.cpp
#   backend/camerahistoryupdater.cpp
    backend/dkcamera.cpp
    backend/gpcamera.cpp
    backend/camiteminfo.cpp
    backend/umscamera.cpp

    main/importsettings.cpp
)

set(libimportui_SRCS
    dialogs/camerafolderdialog.cpp
    dialogs/camerainfodialog.cpp
    dialogs/capturedlg.cpp
    dialogs/cameramessagebox.cpp

    widgets/capturewidget.cpp
    widgets/freespacewidget.cpp
    widgets/renamecustomizer.cpp
    widgets/albumcustomizer.cpp
    widgets/advancedsettings.cpp
    widgets/scriptingsettings.cpp
    widgets/filter.cpp
    widgets/filtercombo.cpp
    widgets/importcontextmenu.cpp
    widgets/dngconvertsettings.cpp

    models/importimagemodel.cpp
    models/importthumbnailmodel.cpp
    models/importfiltermodel.cpp
    models/camitemsortsettings.cpp

    items/camerafolderitem.cpp
    items/itemviewimportdelegate.cpp
    items/importcategorydrawer.cpp
    items/importdelegate.cpp
    items/importtooltipfiller.cpp
    items/importoverlays.cpp

    views/camerafolderview.cpp
    views/cameranamehelper.cpp
    views/freespacetooltip.cpp
    views/importcategorizedview.cpp
    views/importthumbnailbar.cpp
    views/importpreviewview.cpp
    views/importiconview.cpp
    views/importiconview_p.cpp
    views/importstackedview.cpp
    views/importview.cpp

    main/importui.cpp
)

add_library(importuibackend_src OBJECT ${libimportuibackend_SRCS})

add_library(importui_src OBJECT ${libimportui_SRCS})

install(FILES main/importui5.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam)

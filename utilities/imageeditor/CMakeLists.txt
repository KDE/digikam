#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories($<TARGET_PROPERTY:Qt5::PrintSupport,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Network,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::Service,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::Solid,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::WindowSystem,INTERFACE_INCLUDE_DIRECTORIES>
)

if(KF5KIO_FOUND)
    include_directories($<TARGET_PROPERTY:KF5::KIOWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

if(Gphoto2_FOUND)
    include_directories(${GPHOTO2_INCLUDE_DIRS})
endif()

set(libeditorwidgets_SRCS
    widgets/imageguidewidget.cpp
    widgets/imagebrushguidewidget.cpp
    widgets/imagepreviewitem.cpp
    widgets/previewtoolbar.cpp
    widgets/previewlist.cpp
    widgets/imageregionwidget.cpp
    widgets/imageregionitem.cpp
    widgets/rubberitem.cpp
    widgets/canvas.cpp
)

set(libeditordlg_SRCS
    dialogs/colorcorrectiondlg.cpp
    dialogs/softproofdialog.cpp
    dialogs/versioningpromptusersavedlg.cpp
)

set(libeditorgui_SRCS
    main/imagewindow.cpp
)

install(FILES main/imageeditorui5.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam)

set(libeditorcore_SRCS
    core/undocache.cpp
    core/undoaction.cpp
    core/undomanager.cpp
    core/editorcore.cpp
    core/iccpostloadingmanager.cpp
)

set(libeditorprintiface_SRCS
    printiface/printhelper.cpp
    printiface/printoptionspage.cpp
    printiface/printconfig.cpp
)

ki18n_wrap_ui(libeditorprintiface_SRCS
              printiface/printoptionspage.ui
)

set(libeditorrawimport_SRCS
    rawimport/rawimport.cpp
    rawimport/rawpreview.cpp
    rawimport/rawsettingsbox.cpp
)

set(libeditortools_SRCS
    tools/decorate/inserttextwidget.cpp
    tools/decorate/inserttexttool.cpp
    tools/decorate/bordertool.cpp
    tools/decorate/texturetool.cpp
    tools/filters/colorfxtool.cpp
    tools/filters/charcoaltool.cpp
    tools/filters/embosstool.cpp
    tools/filters/oilpainttool.cpp
    tools/filters/blurfxtool.cpp
    tools/filters/distortionfxtool.cpp
    tools/filters/raindroptool.cpp
    tools/filters/filmgraintool.cpp
    tools/colors/autocorrectiontool.cpp
    tools/colors/bcgtool.cpp
    tools/colors/bwsepiatool.cpp
    tools/colors/profileconversiontool.cpp
    tools/colors/cbtool.cpp
    tools/colors/whitebalancetool.cpp
    tools/colors/hsltool.cpp
    tools/colors/channelmixertool.cpp
    tools/colors/adjustcurvestool.cpp
    tools/colors/adjustlevelstool.cpp
    tools/colors/filmtool.cpp
    tools/enhance/restorationtool.cpp
    tools/enhance/blurtool.cpp
    tools/enhance/sharpentool.cpp
    tools/enhance/noisereductiontool.cpp
    tools/enhance/localcontrasttool.cpp
    tools/enhance/redeyetool.cpp
    tools/enhance/antivignettingtool.cpp
    tools/enhance/lensdistortiontool.cpp
    tools/enhance/healingclonetool.cpp
    tools/enhance/hotpixels/weights.cpp
    tools/enhance/hotpixels/blackframeparser.cpp
    tools/enhance/hotpixels/blackframelistview.cpp
    tools/enhance/hotpixels/hotpixelfixer.cpp
    tools/enhance/hotpixels/hotpixelstool.cpp
    tools/transform/freerotationtool.cpp
    tools/transform/sheartool.cpp
    tools/transform/resizetool.cpp
    tools/transform/perspectivetool.cpp
    tools/transform/perspectivewidget.cpp
    tools/transform/triangle.cpp
    tools/transform/matrix.cpp
    tools/transform/imageselectionwidget.cpp
    tools/transform/ratiocroptool.cpp
)

if(Lqr-1_FOUND)
    set(libeditortools_SRCS ${libeditortools_SRCS}
        tools/transform/contentawareresizetool.cpp
       )
endif()

if(LensFun_FOUND)
    set(libeditortools_SRCS ${libeditortools_SRCS}
        tools/enhance/lensautofixtool.cpp
       )
    include_directories(${LENSFUN_INCLUDE_DIRS})
endif()

set(libeditoriface_SRCS
    editor/editortool.cpp
    editor/editortooliface.cpp
    editor/editorstackview.cpp
    editor/editortoolsettings.cpp
    editor/editorwindow.cpp
    editor/imageiface.cpp
)

# this lib is used to build digikam core
add_library(imageeditor_src OBJECT ${libeditorcore_SRCS}
                                   ${libeditordlg_SRCS}
                                   ${libeditoriface_SRCS}
                                   ${libeditorprintiface_SRCS}
                                   ${libeditorrawimport_SRCS}
                                   ${libeditorwidgets_SRCS}
                                   ${libeditortools_SRCS}
)

add_library(imageeditorgui_src OBJECT ${libeditorgui_SRCS})

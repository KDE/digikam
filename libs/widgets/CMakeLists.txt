#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(libcommonwidgets_SRCS
    common/abstractitemdragdrophandler.cpp
    common/dexpanderbox.cpp
    common/dactivelabel.cpp
    common/dragdropimplementations.cpp
    common/modelcompleter.cpp
    common/searchtextbar.cpp
    common/statesavingobject.cpp
    common/visibilitycontroller.cpp
    common/dlayoutbox.cpp
)

set(librangetools_SRCS
    range/dsliderspinbox.cpp
    range/dnuminput.cpp
    range/drangebox.cpp
)

set(libcombotools_SRCS
    combo/comboboxutilities.cpp
    combo/dcombobox.cpp
    combo/squeezedcombobox.cpp
)

set(libcolorstools_SRCS
    colors/dcolorvalueselector.cpp
    colors/dhuesaturationselect.cpp
    colors/dcolorchoosermode.cpp
    colors/dcolorselector.cpp
    colors/colorgradientwidget.cpp
    colors/dgradientslider.cpp
)

set(libfontstools_SRCS
    fonts/dfontproperties.cpp
    fonts/dfontselect.cpp
)

set(libfilestools_SRCS
    files/filesaveoptionsbox.cpp
    files/filesaveconflictbox.cpp
    files/dfiledialog.cpp
    files/dfileselector.cpp
    files/dbinarysearch.cpp
    files/dbinaryiface.cpp
    files/drawdecoderwidget.cpp
)

set(libmainview_SRCS
    mainview/dxmlguiwindow.cpp
    mainview/daboutdata.cpp
    mainview/dzoombar.cpp
    mainview/dcursortracker.cpp
    mainview/thumbbardock.cpp
    mainview/thememanager.cpp
    mainview/schememanager.cpp
    mainview/sidebar.cpp
    mainview/fullscreensettings.cpp
    ../../app/utils/digikam_globals.cpp
   )

set(libitemview_SRCS
    itemview/dcategorizedview.cpp
    itemview/dcategorizedsortfilterproxymodel.cpp
    itemview/ditemdelegate.cpp
    itemview/ditemtooltip.cpp
    itemview/dcategorydrawer.cpp
    itemview/imagedelegateoverlay.cpp
    itemview/itemviewhoverbutton.cpp
    itemview/itemviewimagedelegate.cpp
    itemview/itemviewtooltip.cpp
    itemview/itemviewcategorized.cpp
    itemview/actioncategorizedview.cpp
)

set(libgraphicsviewwidgets_SRCS
    graphicsview/clickdragreleaseitem.cpp
    graphicsview/dimgchilditem.cpp
    graphicsview/dimgpreviewitem.cpp
    graphicsview/regionframeitem.cpp
    graphicsview/graphicsdimgitem.cpp
    graphicsview/graphicsdimgview.cpp
    graphicsview/imagezoomsettings.cpp
    graphicsview/previewlayout.cpp
    graphicsview/paniconwidget.cpp
    graphicsview/itemvisibilitycontroller.cpp
)

set(libmetadatawidgets_SRCS
    metadata/mdkeylistviewitem.cpp
    metadata/metadatalistview.cpp
    metadata/metadatalistviewitem.cpp
    metadata/metadatawidget.cpp
    metadata/metadataselector.cpp
    metadata/metadatapanel.cpp
    metadata/iptcwidget.cpp
    metadata/exifwidget.cpp
    metadata/makernotewidget.cpp
    metadata/xmpwidget.cpp
    metadata/ratingwidget.cpp
    metadata/colorlabelwidget.cpp
    metadata/picklabelwidget.cpp
    metadata/altlangstredit.cpp
    metadata/colorlabelwidget.cpp
    metadata/countryselector.cpp
    metadata/subjectwidget.cpp
)

set(libiccprofileswidgets_SRCS
    iccprofiles/iccpreviewwidget.cpp
    iccprofiles/iccprofilewidget.cpp
    iccprofiles/cietonguewidget.cpp
    iccprofiles/iccprofilescombobox.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt5::PrintSupport,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

if(KF5NotifyConfig_FOUND)
    include_directories($<TARGET_PROPERTY:KF5::NotifyConfig,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

if(Gphoto2_FOUND)
    include_directories(${GPHOTO2_INCLUDE_DIRS})
endif()

add_library(digikamwidgetscore_src OBJECT
    ${libmainview_SRCS}
    ${libcommonwidgets_SRCS}
    ${libgraphicsviewwidgets_SRCS}
    ${libiccprofileswidgets_SRCS}
    ${libitemview_SRCS}
    ${libmetadatawidgets_SRCS}
    ${libcolorstools_SRCS}
    ${libfontstools_SRCS}
    ${libfilestools_SRCS}
    ${libcombotools_SRCS}
    ${librangetools_SRCS}
)

#############################################################"

set(libimagehistory_SRCS
    imagehistory/filtershistorywidget.cpp
    imagehistory/imagefiltershistoryitemdelegate.cpp
    imagehistory/imagefiltershistorymodel.cpp
    imagehistory/imagefiltershistorytreeitem.cpp
    imagehistory/versionswidget.cpp
)

add_library(imagehistorywidgets_src OBJECT ${libimagehistory_SRCS})

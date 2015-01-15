set(libcommonwidgets_SRCS
    common/abstractitemdragdrophandler.cpp
    common/buttonicondisabler.cpp
    common/colorgradientwidget.cpp
    common/comboboxutilities.cpp
    common/dfontselect.cpp
    common/dgradientslider.cpp
    common/dragdropimplementations.cpp
    common/filesaveoptionsbox.cpp
    common/fullscreensettings.cpp
    common/modelcompletion.cpp
    common/searchtextbar.cpp
    common/splashscreen.cpp
    common/statesavingobject.cpp
    common/visibilitycontroller.cpp
    common/workingwidget.cpp
    common/widgets_debug.cpp
)

set(libmainview_SRCS
    mainview/dxmlguiwindow.cpp
    mainview/daboutdata.cpp
    mainview/dzoombar.cpp
    mainview/dlogoaction.cpp
    mainview/ditemtooltip.cpp
    mainview/dhistoryview.cpp
    mainview/dcursortracker.cpp
    mainview/thumbbardock.cpp
    mainview/thememanager.cpp
    mainview/sidebar.cpp
    ../../app/utils/digikam_globals.cpp
   )

set(libitemview_SRCS
    itemview/digikamkcategorizedview.cpp
    itemview/dcategorizedview.cpp
    itemview/ditemdelegate.cpp
    itemview/imagedelegateoverlay.cpp
    itemview/itemviewhoverbutton.cpp
    itemview/itemviewimagedelegate.cpp
    itemview/itemviewtooltip.cpp
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
)

set(libiccprofileswidgets_SRCS
    iccprofiles/iccpreviewwidget.cpp
    iccprofiles/iccprofilewidget.cpp
    iccprofiles/cietonguewidget.cpp
    iccprofiles/iccprofilescombobox.cpp
)

add_library(digikamwidgets ${libmainview_SRCS} ${libcommonwidgets_SRCS} ${libgraphicsviewwidgets_SRCS} ${libiccprofileswidgets_SRCS} ${libitemview_SRCS} ${libmetadatawidgets_SRCS})
target_link_libraries(digikamwidgets
                      digikamcore
                      digikamdialogs
                      dimg
                      
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      
                      KF5::KDELibs4Support # for KLineEdit
)

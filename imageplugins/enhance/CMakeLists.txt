#
# Copyright (c) 2010-2016 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(digikamimageplugin_enhance_PART_SRCS imageplugin_enhance.cpp
                                         restorationtool.cpp
                                         blurtool.cpp
                                         sharpentool.cpp
                                         noisereductiontool.cpp
                                         localcontrasttool.cpp
                                         redeyetool.cpp
                                         inpaintingtool.cpp
                                         antivignettingtool.cpp
                                         lensdistortiontool.cpp
                                         hotpixels/weights.cpp
                                         hotpixels/blackframeparser.cpp
                                         hotpixels/blackframelistview.cpp
                                         hotpixels/hotpixelfixer.cpp
                                         hotpixels/hotpixelstool.cpp
)

if(LENSFUN_FOUND)
    set(digikamimageplugin_enhance_PART_SRCS ${digikamimageplugin_enhance_PART_SRCS}
        lensautofixtool.cpp
       )
endif()

include_directories(
    $<TARGET_PROPERTY:KF5::KIOCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(digikamimageplugin_enhance MODULE ${digikamimageplugin_enhance_PART_SRCS})


target_link_libraries(digikamimageplugin_enhance
                      PRIVATE
                      digikamcore

                      Qt5::Core
                      Qt5::Gui

                      KF5::XmlGui
                      KF5::KIOCore
                      KF5::ConfigCore
                      KF5::I18n
)

install(TARGETS digikamimageplugin_enhance             DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES digikamimageplugin_enhance_ui.rc         DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam)
install(FILES digikamimageplugin_enhance.desktop       DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES digiKam-ImagePlugin_Enhance.metainfo.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata)

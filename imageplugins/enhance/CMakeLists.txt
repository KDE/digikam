#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
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

KDE4_ADD_PLUGIN(digikamimageplugin_enhance ${digikamimageplugin_enhance_PART_SRCS})

target_link_libraries(digikamimageplugin_enhance
                      digikamcore

                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${KDCRAW_LIBRARIES}
                      ${KEXIV2_LIBRARIES}
                     )

if(LENSFUN_FOUND)
   target_link_libraries(digikamimageplugin_enhance ${LENSFUN_LIBRARIES})
endif()

install(TARGETS digikamimageplugin_enhance             DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES digikamimageplugin_enhance_ui.rc         DESTINATION ${DATA_INSTALL_DIR}/digikam)
install(FILES digikamimageplugin_enhance.desktop       DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES digiKam-ImagePlugin_Enhance.metainfo.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata)

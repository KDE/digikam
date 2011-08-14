SET(digikamimageplugin_enhance_PART_SRCS imageplugin_enhance.cpp
                                         restorationtool.cpp
                                         blurtool.cpp
                                         clonetool.cpp
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

IF(GLIB2_FOUND)
    SET(digikamimageplugin_enhance_PART_SRCS ${digikamimageplugin_enhance_PART_SRCS}
        lensautofixtool.cpp
       )
ENDIF(GLIB2_FOUND)

KDE4_ADD_PLUGIN(digikamimageplugin_enhance ${digikamimageplugin_enhance_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_enhance
    digikamcore
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDCRAW_LIBRARIES}
    ${KEXIV2_LIBRARIES}
    ${QT_QT3SUPPORT_LIBRARY}
    )

IF(LENSFUN_LIBRARY)
   TARGET_LINK_LIBRARIES(digikamimageplugin_enhance ${LENSFUN_LIBRARY})
ENDIF(LENSFUN_LIBRARY)

INSTALL(TARGETS digikamimageplugin_enhance DESTINATION ${PLUGIN_INSTALL_DIR} )
INSTALL(FILES digikamimageplugin_enhance_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_enhance.desktop DESTINATION ${SERVICES_INSTALL_DIR})

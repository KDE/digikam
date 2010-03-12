
INCLUDE_DIRECTORIES(distortion
                    ${CMAKE_CURRENT_BINARY_DIR}/../../
                   )


SET(distortion_SRCS distortion/lensdistortiontool.cpp
   )

SET(digikamimageplugin_lenscorrection_PART_SRCS imageplugin_lenscorrection.cpp
                                                ${distortion_SRCS}
)

KDE4_ADD_PLUGIN(digikamimageplugin_lenscorrection ${digikamimageplugin_lenscorrection_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_lenscorrection
    digikamcore
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDCRAW_LIBRARIES}
    ${KEXIV2_LIBRARIES}
    )

INSTALL(TARGETS digikamimageplugin_lenscorrection DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES digikamimageplugin_lenscorrection_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_lenscorrection.desktop DESTINATION ${SERVICES_INSTALL_DIR})

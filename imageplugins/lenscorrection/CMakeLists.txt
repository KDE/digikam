
INCLUDE_DIRECTORIES(autocorrection
                    distortion
                    vignetting
                    ${CMAKE_CURRENT_BINARY_DIR}/../../
                   )

SET(autocorrection_SRCS autocorrection/autocorrectiontool.cpp 
                        autocorrection/klensfun.cpp
   )

SET(distortion_SRCS distortion/lensdistortiontool.cpp
                    distortion/lensdistortion.cpp
                    distortion/pixelaccess.cpp
   )

SET(vignetting_SRCS vignetting/antivignettingtool.cpp
                    vignetting/antivignetting.cpp)

IF(LENSFUN_LIBRARY)
   SET(digikamimageplugin_lenscorrection_PART_SRCS imageplugin_lenscorrection.cpp
                                                   ${distortion_SRCS}
                                                   ${vignetting_SRCS}
                                                   ${autocorrection_SRCS})
ELSE(LENSFUN_LIBRARY)
   SET(digikamimageplugin_lenscorrection_PART_SRCS imageplugin_lenscorrection.cpp
                                                   ${distortion_SRCS}
                                                   ${vignetting_SRCS})
ENDIF(LENSFUN_LIBRARY)

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

IF(LENSFUN_LIBRARY)
   INCLUDE_DIRECTORIES(${LENSFUN_INCLUDE_DIR})
   TARGET_LINK_LIBRARIES(digikamimageplugin_lenscorrection ${LENSFUN_LIBRARY})
ENDIF(LENSFUN_LIBRARY)

INSTALL(TARGETS digikamimageplugin_lenscorrection DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES digikamimageplugin_lenscorrection_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_lenscorrection.desktop DESTINATION ${SERVICES_INSTALL_DIR})

INCLUDE_DIRECTORIES(lensfuniface
                    distortion
                    vignetting
                   )

SET(lensfuniface_SRCS lensfuniface/imageeffect_lenscorrection.cpp 
                      lensfuniface/klensfun.cpp
   )

SET(distortion_SRCS distortion/imageeffect_lensdistortion.cpp
                    distortion/lensdistortion.cpp
                    distortion/pixelaccess.cpp
   )

SET(vignetting_SRCS vignetting/imageeffect_antivignetting.cpp
                    vignetting/antivignetting.cpp)

IF(LENSFUN_LIBRARY)
   SET(digikamimageplugin_lenscorrection_PART_SRCS imageplugin_lenscorrection.cpp
                                                   ${distortion_SRCS}
                                                   ${vignetting_SRCS}
                                                   ${lensfuniface_SRCS})
ELSEIF(LENSFUN_LIBRARY)
   SET(digikamimageplugin_lenscorrection_PART_SRCS imageplugin_lenscorrection.cpp
                                                   ${distortion_SRCS}
                                                   ${vignetting_SRCS})
ENDIF(LENSFUN_LIBRARY)

KDE4_ADD_PLUGIN(digikamimageplugin_lenscorrection ${digikamimageplugin_lenscorrection_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_lenscorrection digikam)

IF(LENSFUN_LIBRARY)
   TARGET_LINK_LIBRARIES(digikamimageplugin_lenscorrection lensfun)
ENDIF(LENSFUN_LIBRARY)

INSTALL(TARGETS digikamimageplugin_lenscorrection DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_lenscorrection_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_lenscorrection.desktop DESTINATION ${SERVICES_INSTALL_DIR})

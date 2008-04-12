
set(CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules
)

IF(LENSFUN_LIBRARY)
   SET(digikamimageplugin_lenscorrection_PART_SRCS imageplugin_lenscorrection.cpp
   					imageeffect_lenscorrection.cpp
   					klensfun.cpp
      )
   
   KDE4_ADD_PLUGIN(digikamimageplugin_lenscorrection ${digikamimageplugin_lenscorrection_PART_SRCS})
   
   TARGET_LINK_LIBRARIES(digikamimageplugin_lenscorrection digikam lensfun)
   
   INSTALL(TARGETS digikamimageplugin_lenscorrection DESTINATION ${PLUGIN_INSTALL_DIR})
   
   INSTALL(FILES digikamimageplugin_lenscorrection_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
   INSTALL(FILES digikamimageplugin_lenscorrection.desktop DESTINATION ${SERVICES_INSTALL_DIR})
ENDIF(LENSFUN_LIBRARY)


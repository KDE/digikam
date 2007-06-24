
include_directories( ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas ${CMAKE_SOURCE_DIR}/digikam/libs/histogram ${CMAKE_SOURCE_DIR}/digikam/libs/levels ${CMAKE_SOURCE_DIR}/digikam/libs/curves ${CMAKE_SOURCE_DIR}/digikam/libs/whitebalance ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/digikam   )


########### next target ###############

set(digikamimageplugin_distortionfx_PART_SRCS imageplugin_distortionfx.cpp imageeffect_distortionfx.cpp distortionfx.cpp ${libdimg_SRCS} ${libdimgloaders_SRCS} )


kde4_automoc(${digikamimageplugin_distortionfx_PART_SRCS})

kde4_add_plugin(digikamimageplugin_distortionfx ${digikamimageplugin_distortionfx_PART_SRCS})

target_link_libraries(digikamimageplugin_distortionfx ${KDE4_KDEUI_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${JPEG_LIBRARIES} ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JASPER_LIBRARIES}
)

install(TARGETS digikamimageplugin_distortionfx DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  digikamimageplugin_distortionfx_ui.rc      DESTINATION ${DATA_INSTALL_DIR}/digikam )
install( FILES  digikamimageplugin_distortionfx.desktop DESTINATION ${SERVICES_INSTALL_DIR} )



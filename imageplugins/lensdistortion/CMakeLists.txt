
include_directories( ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas ${CMAKE_SOURCE_DIR}/digikam/libs/histogram ${CMAKE_SOURCE_DIR}/digikam/libs/levels ${CMAKE_SOURCE_DIR}/digikam/libs/curves ${CMAKE_SOURCE_DIR}/digikam/libs/whitebalance ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/digikam   )


########### next target ###############

set(digikamimageplugin_lensdistortion_PART_SRCS  imageplugin_lensdistortion.cpp imageeffect_lensdistortion.cpp lensdistortion.cpp pixelaccess.cpp)

kde4_automoc(${digikamimageplugin_lensdistortion_PART_SRCS})

kde4_add_plugin(digikamimageplugin_lensdistortion ${digikamimageplugin_lensdistortion_PART_SRCS})

target_link_libraries(digikamimageplugin_lensdistortion  ${KDE4_KDECORE_LIBS} )

install(TARGETS digikamimageplugin_lensdistortion DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  digikamimageplugin_lensdistortion_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam )
install( FILES  digikamimageplugin_lensdistortion.desktop DESTINATION ${SERVICES_INSTALL_DIR} )



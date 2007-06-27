
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor
                    ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas
                    ${CMAKE_SOURCE_DIR}/digikam/libs/histogram ${CMAKE_SOURCE_DIR}/digikam/libs/levels
                    ${CMAKE_SOURCE_DIR}/digikam/libs/curves ${CMAKE_SOURCE_DIR}/digikam/libs/whitebalance
                    ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common
                    ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles
                    ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins
                    ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs ${CMAKE_SOURCE_DIR}/digikam/libs/dimg
                    ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters
                    ${CMAKE_SOURCE_DIR}/digikam/digikam   
                   )

SET(digikamimageplugin_whitebalance_PART_SRCS imageplugin_whitebalance.cpp )
#mageeffect_whitebalance.cpp)

KDE4_AUTOMOC(${digikamimageplugin_whitebalance_PART_SRCS})

KDE4_ADD_PLUGIN(digikamimageplugin_whitebalance ${digikamimageplugin_whitebalance_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_whitebalance  ${KDE4_KDECORE_LIBS})

INSTALL(TARGETS digikamimageplugin_whitebalance DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_whitebalance_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_whitebalance.desktop DESTINATION ${SERVICES_INSTALL_DIR})

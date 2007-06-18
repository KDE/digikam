
add_subdirectory( pics )
add_subdirectory( icons )
add_subdirectory( setup )

include_directories( ${CMAKE_SOURCE_DIR}/digikam/showfoto/setup ${CMAKE_SOURCE_DIR}/digikam/utilities/setup ${CMAKE_SOURCE_DIR}/digikam/libs/thumbbar ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/imageproperties ${CMAKE_SOURCE_DIR}/digikam/libs/histogram ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/utilities/slideshow ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/tools ${CMAKE_SOURCE_DIR}/digikam/utilities/splashscreen   )


########### next target ###############

set(showfoto_SRCS main.cpp showfoto.cpp )

kde4_automoc(${showfoto_SRCS})

kde4_add_executable(showfoto ${showfoto_SRCS})

target_link_libraries(showfoto  ${KDE4_KDECORE_LIBS} setup dialogshowfoto imagepropertiesshowfoto thumbbar showfoto splashscreen slideshow threadimageio jpegutils ${KDE4_KUTILS_LIBS} ${KDE4_KPARTS_LIBS} )

install(TARGETS showfoto DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES  showfoto.desktop DESTINATION ${XDG_APPS_DIR} )
install( FILES  showfotoui.rc DESTINATION ${DATA_INSTALL_DIR}/showfoto )


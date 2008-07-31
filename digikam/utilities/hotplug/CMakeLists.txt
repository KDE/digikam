set(digikamhelperdir  ${DATA_INSTALL_DIR}/digikam/utils)

configure_file(digikam-download.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/digikam-download.desktop @ONLY)
configure_file(digikam-gphoto2-camera.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/digikam-gphoto2-camera.desktop @ONLY)
configure_file(digikam-mount-and-download.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/digikam-mount-and-download.desktop @ONLY)

install( FILES  
	${CMAKE_CURRENT_BINARY_DIR}/digikam-download.desktop 
	${CMAKE_CURRENT_BINARY_DIR}/digikam-gphoto2-camera.desktop 
	${CMAKE_CURRENT_BINARY_DIR}/digikam-mount-and-download.desktop 
	DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus )

if(NOT WIN32)
  install( PROGRAMS  digikam-camera DESTINATION ${BIN_INSTALL_DIR} )
endif(NOT WIN32)

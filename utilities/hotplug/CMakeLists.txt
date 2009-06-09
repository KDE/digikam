SET(digikamhelperdir  ${DATA_INSTALL_DIR}/digikam/utils)

#configure_file(digikam-download.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/digikam-download.desktop @ONLY)
#configure_file(digikam-gphoto2-camera.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/digikam-gphoto2-camera.desktop @ONLY)
#configure_file(digikam-mount-and-download.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/digikam-mount-and-download.desktop @ONLY)

CONFIGURE_FILE(digikam-opencamera.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/digikam-opencamera.desktop @ONLY)

#install( FILES  
#	${CMAKE_CURRENT_BINARY_DIR}/digikam-download.desktop 
#	${CMAKE_CURRENT_BINARY_DIR}/digikam-gphoto2-camera.desktop 
#	${CMAKE_CURRENT_BINARY_DIR}/digikam-mount-and-download.desktop 
#	DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus )
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/digikam-opencamera.desktop DESTINATION ${DATA_INSTALL_DIR}/solid/actions )

IF(NOT WIN32)
  INSTALL( PROGRAMS digikam-camera DESTINATION ${digikamhelperdir} )
ENDIF(NOT WIN32)

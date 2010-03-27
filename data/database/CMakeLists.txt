CONFIGURE_FILE(dbconfig.xml.cmake ${CMAKE_CURRENT_BINARY_DIR}/dbconfig.xml)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/dbconfig.xml
			  mysql-global.conf
        DESTINATION ${DATA_INSTALL_DIR}/digikam/database )
        
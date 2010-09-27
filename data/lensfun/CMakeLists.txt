FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.xml")
INSTALL( FILES ${files}
         DESTINATION ${DATA_INSTALL_DIR}/digikam/lensfun )

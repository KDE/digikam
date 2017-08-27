#
# Ahmed Fathy
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


#file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
#install(FILES ${files}  DESTINATION ${DATA_INSTALL_DIR}/digikam/mediaserver/descriptions)
#file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/icon.png")
#install(FILES ${files} DESTINATION ${DATA_INSTALL_DIR}/digikam/mediaserver/descriptions)


#dont forget to copy.
file(COPY descriptions DESTINATION ${DATA_INSTALL_DIR}/digikam/mediaserver)
install(DIRECTORY descriptions DESTINATION ${DATA_INSTALL_DIR}/digikam/mediaserver)

#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Under Windows, Haar Cascade XML files used by Face Detection are not installed with OpenCV rules. Why ???
# Tested with OpenCV 2.4.9 (TODO: Check if it's always true with OpenCV 3.x)
# So we generalize installation of these files directly through digiKam.

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/haarcascade_*.xml")

install(FILES ${files}
        DESTINATION ${DATA_INSTALL_DIR}/digikam/facesengine
)

# ---------------------------------------------------------------

# NOTE: the DBFACECONFIG_XML_VERSION ID is configured in the toplevel CMakeLists.txt,
configure_file(dbfaceconfig.xml.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/dbfaceconfig.xml)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dbfaceconfig.xml
        DESTINATION ${DATA_INSTALL_DIR}/digikam/facesengine
)

# ---------------------------------------------------------------

# add_custom_command(OUTPUT face-funnel.data
#                    COMMAND ${CMAKE_COMMAND} -E tar xz ${CMAKE_CURRENT_SOURCE_DIR}/face-funnel.data.tgz)
#
# add_custom_target(extract_funnel_data ALL DEPENDS face-funnel.data)
#
# install(FILES ${CMAKE_CURRENT_BINARY_DIR}/face-funnel.data
#         DESTINATION ${DATA_INSTALL_DIR}/digikam/alignment-congealing
# )
#
# add_custom_command(OUTPUT flandmark_model.dat
#                    COMMAND ${CMAKE_COMMAND} -E tar xz ${CMAKE_CURRENT_SOURCE_DIR}/flandmark_model.dat.tgz)
#
# add_custom_target(extract_flandmark_data ALL DEPENDS flandmark_model.dat)
#
# install(FILES ${CMAKE_CURRENT_BINARY_DIR}/flandmark_model.dat
#         DESTINATION ${DATA_INSTALL_DIR}/digikam/alignment-flandmark)

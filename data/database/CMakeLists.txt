#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# NOTE: the DBCORECONFIG_XML_VERSION ID is configured in the toplevel CMakeLists.txt,
configure_file(dbconfig.xml.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/dbconfig.xml)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dbconfig.xml
              ${CMAKE_CURRENT_SOURCE_DIR}/mysql-global.conf
        DESTINATION ${DATA_INSTALL_DIR}/digikam/database
)

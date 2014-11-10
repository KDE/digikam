#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# the DBCONFIG_XML version number is configured in the / CMakeList.txt,
# and the dbconfigversion.h file from the main digikam/ CMakeLists.txt.
# We need the include file before this subdirectory here is reached.
configure_file(dbconfig.xml.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/dbconfig.xml)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dbconfig.xml
              mysql-global.conf
        DESTINATION ${DATA_INSTALL_DIR}/digikam/database )

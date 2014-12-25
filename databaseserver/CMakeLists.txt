#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(digikamdatabaseserver_SRCS ${lib..._SRCS}
                               databaseserver.cpp
                               pollthread.cpp
                               main.cpp
)

qt5_add_dbus_adaptor(digikamdatabaseserver_SRCS org.kde.digikam.DatabaseServer.xml
                     databaseserver.h Digikam::DatabaseServer)

add_executable(digikamdatabaseserver ${digikamdatabaseserver_SRCS})

target_link_libraries(digikamdatabaseserver
                      digikamdatabase
                      digikamcore

                      Qt5::Sql
                      KF5::Core
)

if(NOT WIN32)
    # To link under Solaris (see bug #274484)
    target_link_libraries(digikamdatabaseserver ${MATH_LIBRARY})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
    target_link_libraries(digikamdatabaseserver ${KVM_LIBRARY})
endif()

install(TARGETS digikamdatabaseserver DESTINATION ${LIBEXEC_INSTALL_DIR})

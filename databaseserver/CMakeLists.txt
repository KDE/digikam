#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015 by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
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
                      Qt5::DBus
                      KF5::I18n
)

if(NOT WIN32)
    # To link under Solaris (see bug #274484)
    target_link_libraries(digikamdatabaseserver ${MATH_LIBRARY})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
    target_link_libraries(digikamdatabaseserver ${KVM_LIBRARY})
endif()

install(TARGETS digikamdatabaseserver DESTINATION ${LIBEXEC_INSTALL_DIR})

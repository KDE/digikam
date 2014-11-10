#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

SET_DEBUG_AREA_CODE(${AREA_CODE_DATABASESERVER})

set(digikamdatabaseserver_SRCS ${lib..._SRCS}
                               databaseserver.cpp
                               pollthread.cpp
                               main.cpp
   )

QT4_ADD_DBUS_ADAPTOR(digikamdatabaseserver_SRCS org.kde.digikam.DatabaseServer.xml
                     databaseserver.h Digikam::DatabaseServer)

KDE4_ADD_EXECUTABLE(digikamdatabaseserver ${digikamdatabaseserver_SRCS})

target_link_libraries(digikamdatabaseserver
                      digikamdatabase
                      digikamcore

                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTSQL_LIBRARY}
                     )

if(NOT WIN32)
    # To link under Solaris (see bug #274484)
    target_link_libraries(digikamdatabaseserver ${MATH_LIBRARY})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
    target_link_libraries(digikamdatabaseserver ${KVM_LIBRARY})
endif()

install(TARGETS digikamdatabaseserver DESTINATION ${LIBEXEC_INSTALL_DIR})

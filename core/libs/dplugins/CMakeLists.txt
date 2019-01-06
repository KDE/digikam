#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::XmlPatterns,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

if(ENABLE_QWEBENGINE)
    include_directories($<TARGET_PROPERTY:Qt5::WebEngineWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
else()
    include_directories($<TARGET_PROPERTY:Qt5::WebKitWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

if(KF5KIO_FOUND)
    include_directories($<TARGET_PROPERTY:KF5::KIOCore,INTERFACE_INCLUDE_DIRECTORIES>
                        $<TARGET_PROPERTY:KF5::KIOFileWidgets,INTERFACE_INCLUDE_DIRECTORIES>
    )
endif()

# OAuth2 library ---------------------------------------------------

# The o2 library does not adhere to the flags we use
remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY)
# It also does not export symbols, so export all by default
unset(CMAKE_CXX_VISIBILITY_PRESET)

if(WIN32)
    add_definitions(-DO2_DLL_EXPORT)
endif()

# Copied from o2/src/CMakeLists.txt
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(libwso2_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o2reply.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o2replyserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o2requestor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o2simplecrypt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o0settingsstore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o0baseauth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o0abstractstore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o0globals.h
    # Enable when needed
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o1requestor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o1timedreply.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o1smugmug.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o1twitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/oxtwitter.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o1dropbox.h
    #${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o1flickr.h
    #${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o2gft.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o2facebook.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o2skydrive.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o2hubic.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/webservices/o2/src/o2uber.cpp
)

if(ENABLE_QWEBENGINE)
    set(libwso2_SRCS
        ${libwso2_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/webservices/webwidget_qwebengine.cpp
       )
else()
    set(libwso2_SRCS
        ${libwso2_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/webservices/webwidget.cpp
       )
endif()

# Helper classes ------------------------------------------------------------------


set(libdpluginsinterface_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/core/dplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/dpluginauthor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/dpluginconfview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/dpluginsetup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/dpluginaction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/dplugindialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/dpluginaboutdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/dpluginloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/dpluginloader_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iface/dinfointerface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iface/dmetainfoiface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dwizardpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dwizarddlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dsavesettingswidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dwizardpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dpreviewmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dpreviewimage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/ditemslist.cpp
    ${libwso2_SRCS}
)

add_library(dpluginsinterface_src OBJECT ${libdpluginsinterface_SRCS})

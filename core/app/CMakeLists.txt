#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# -----------------------------------------------------------------------------------
# Headers to configure

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/utils/digikam_version.h.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/utils/digikam_version.h)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/utils/digikam_dbconfig.h.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/utils/digikam_dbconfig.h)

# -----------------------------------------------------------------------------------
# Common includes rules

include_directories($<TARGET_PROPERTY:Qt5::PrintSupport,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Network,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::Solid,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::WindowSystem,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::Service,INTERFACE_INCLUDE_DIRECTORIES>)

if(ENABLE_QWEBENGINE)
    include_directories($<TARGET_PROPERTY:Qt5::WebEngineWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
else()
    include_directories($<TARGET_PROPERTY:Qt5::WebKitWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

if(KF5KIO_FOUND)
    include_directories($<TARGET_PROPERTY:KF5::KIOWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

if(Gphoto2_FOUND)
    include_directories(${GPHOTO2_INCLUDE_DIRS})
endif()

# -----------------------------------------------------------------------------------
# Targets to build

include(DigikamCoreTarget.cmake)
include(DigikamGuiTarget.cmake)
include(DigikamTarget.cmake)

# -----------------------------------------------------------------------------------
# Headers to install

install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/utils/digikam_core_export.h
            ${CMAKE_CURRENT_BINARY_DIR}/utils/digikam_database_export.h
            ${CMAKE_CURRENT_SOURCE_DIR}/utils/digikam_export.h
        DESTINATION
            ${CMAKE_INSTALL_INCLUDEDIR}/digikam
)

# -----------------------------------------------------------------------------------
# Others Files to install

install(PROGRAMS main/org.kde.digikam.desktop     DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES    main/org.kde.digikam.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES    main/digikamui5.rc               DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam)
install(FILES    main/digikam.notifyrc            DESTINATION ${KNOTIFYRC_INSTALL_DIR})

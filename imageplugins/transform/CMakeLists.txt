#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(digikamimageplugin_transform_PART_SRCS imageplugin_transform.cpp
                                           freerotationtool.cpp
                                           sheartool.cpp
                                           resizetool.cpp

                                           perspectivetool.cpp
                                           perspectivewidget.cpp
                                           triangle.cpp
                                           matrix.cpp

                                           imageselectionwidget.cpp
                                           ratiocroptool.cpp
   )

if(GLIB2_FOUND)
    set(digikamimageplugin_transform_PART_SRCS ${digikamimageplugin_transform_PART_SRCS}
        contentawareresizetool.cpp
       )
endif()

add_library(digikamimageplugin_transform MODULE ${digikamimageplugin_transform_PART_SRCS})

target_link_libraries(digikamimageplugin_transform
                      digikamcore

                      KF5::KDELibs4Support
                      
                      KF5::KIOCore
                      Qt5::Core
                      Qt5::Gui
                      KF5::KDcraw
                     )

install(TARGETS digikamimageplugin_transform             DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES digikamimageplugin_transform_ui.rc         DESTINATION ${DATA_INSTALL_DIR}/digikam)
install(FILES digikamimageplugin_transform.desktop       DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES digiKam-ImagePlugin_Transform.metainfo.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata)

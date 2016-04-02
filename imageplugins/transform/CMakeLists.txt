#
# Copyright (c) 2010-2016 by Gilles Caulier, <caulier dot gilles at gmail dot com>
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

if(LQR-1_FOUND)
    set(digikamimageplugin_transform_PART_SRCS ${digikamimageplugin_transform_PART_SRCS}
        contentawareresizetool.cpp
       )
endif()

add_library(digikamimageplugin_transform MODULE ${digikamimageplugin_transform_PART_SRCS})

target_link_libraries(digikamimageplugin_transform
                      PRIVATE
                      digikamcore

                      Qt5::Core
                      Qt5::Gui

                      KF5::I18n
                      KF5::XmlGui
                      KF5::KIOCore
)

# NOTE: no need to link with lqr-1 library here. DImg is already linked in core.

install(TARGETS digikamimageplugin_transform             DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES digikamimageplugin_transform_ui.rc         DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam)
install(FILES digikamimageplugin_transform.desktop       DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES digiKam-ImagePlugin_Transform.metainfo.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata)

#
# Copyright (c) 2010-2016 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(digikamimageplugin_decorate_PART_SRCS imageplugin_decorate.cpp
                                          inserttextwidget.cpp
                                          inserttexttool.cpp
                                          bordertool.cpp
                                          texturetool.cpp
   )

add_library(digikamimageplugin_decorate MODULE ${digikamimageplugin_decorate_PART_SRCS})

target_link_libraries(digikamimageplugin_decorate
                      PRIVATE
                      digikamcore

                      Qt5::Core
                      Qt5::Gui

                      KF5::I18n
                      KF5::XmlGui
                      KF5::KIOCore
)

install(TARGETS digikamimageplugin_decorate             DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES digikamimageplugin_decorate_ui.rc         DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam)
install(FILES digikamimageplugin_decorate.desktop       DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES digiKam-ImagePlugin_Decorate.metainfo.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata)

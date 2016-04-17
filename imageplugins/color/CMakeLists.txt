#
# Copyright (c) 2010-2016 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(digikamimageplugin_color_PART_SRCS autocorrectiontool.cpp
                                       bcgtool.cpp
                                       bwsepiatool.cpp
                                       profileconversiontool.cpp
                                       cbtool.cpp
                                       whitebalancetool.cpp
                                       hsltool.cpp
                                       channelmixertool.cpp
                                       adjustcurvestool.cpp
                                       adjustlevelstool.cpp
                                       imageplugin_color.cpp
                                       filmtool.cpp
)


add_library(digikamimageplugin_color MODULE ${digikamimageplugin_color_PART_SRCS})


target_link_libraries(digikamimageplugin_color
                      PRIVATE
                      digikamcore

                      Qt5::Core
                      Qt5::Gui

                      KF5::I18n
                      KF5::XmlGui
                     )

install(TARGETS digikamimageplugin_color             DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES digikamimageplugin_color_ui.rc         DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam)
install(FILES digikamimageplugin_color.desktop       DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES adjustlevelstool.upd                   DESTINATION ${DATA_INSTALL_DIR}/kconf_update)
install(FILES digiKam-ImagePlugin_Color.metainfo.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata)

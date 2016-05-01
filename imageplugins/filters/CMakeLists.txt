#
# Copyright (c) 2010-2016 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(digikamimageplugin_fxfilters_PART_SRCS imageplugin_fxfilters.cpp
                                           colorfxtool.cpp
                                           charcoaltool.cpp
                                           embosstool.cpp
                                           oilpainttool.cpp
                                           blurfxtool.cpp
                                           distortionfxtool.cpp
                                           raindroptool.cpp
                                           filmgraintool.cpp
   )

add_library(digikamimageplugin_fxfilters MODULE ${digikamimageplugin_fxfilters_PART_SRCS})

target_link_libraries(digikamimageplugin_fxfilters
                      PRIVATE
                      digikamcore

                      Qt5::Core
                      Qt5::Gui

                      KF5::I18n
                      KF5::XmlGui
)

install(TARGETS digikamimageplugin_fxfilters             DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES digikamimageplugin_fxfilters_ui.rc         DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam)
install(FILES digikamimageplugin_fxfilters.desktop       DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES digiKam-ImagePlugin_FxFilters.metainfo.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata)

#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(digikamimageplugin_decorate_PART_SRCS imageplugin_decorate.cpp
                                          inserttextwidget.cpp
                                          inserttexttool.cpp
                                          bordertool.cpp
                                          texturetool.cpp
   )

KDE4_ADD_PLUGIN(digikamimageplugin_decorate ${digikamimageplugin_decorate_PART_SRCS})

target_link_libraries(digikamimageplugin_decorate
                      digikamcore

                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KDEUI_LIBS}
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${KDCRAW_LIBRARIES}
                     )

install(TARGETS digikamimageplugin_decorate             DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES digikamimageplugin_decorate_ui.rc         DESTINATION ${DATA_INSTALL_DIR}/digikam)
install(FILES digikamimageplugin_decorate.desktop       DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES digiKam-ImagePlugin_Decorate.metainfo.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata)

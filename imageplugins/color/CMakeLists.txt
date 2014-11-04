#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
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


KDE4_ADD_PLUGIN(digikamimageplugin_color ${digikamimageplugin_color_PART_SRCS})

target_link_libraries(digikamimageplugin_color
                      digikamcore

                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${KDCRAW_LIBRARIES}
                      ${KEXIV2_LIBRARIES}
                     )

install(TARGETS digikamimageplugin_color             DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES digikamimageplugin_color_ui.rc         DESTINATION ${DATA_INSTALL_DIR}/digikam)
install(FILES digikamimageplugin_color.desktop       DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES adjustlevelstool.upd                   DESTINATION ${DATA_INSTALL_DIR}/kconf_update)
install(FILES digiKam-ImagePlugin_Color.metainfo.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata)

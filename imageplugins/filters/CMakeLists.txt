#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
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

KDE4_ADD_PLUGIN(digikamimageplugin_fxfilters ${digikamimageplugin_fxfilters_PART_SRCS})

target_link_libraries(digikamimageplugin_fxfilters
                      digikamcore

                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${KDCRAW_LIBRARIES}
                     )

install(TARGETS digikamimageplugin_fxfilters             DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES digikamimageplugin_fxfilters_ui.rc         DESTINATION ${DATA_INSTALL_DIR}/digikam)
install(FILES digikamimageplugin_fxfilters.desktop       DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES digiKam-ImagePlugin_FxFilters.metainfo.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata)

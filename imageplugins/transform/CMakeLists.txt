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

if(LQR-1_FOUND)
    set(digikamimageplugin_transform_PART_SRCS ${digikamimageplugin_transform_PART_SRCS}
        contentawareresizetool.cpp
       )
endif()

KDE4_ADD_PLUGIN(digikamimageplugin_transform ${digikamimageplugin_transform_PART_SRCS})

target_link_libraries(digikamimageplugin_transform
                      digikamcore

                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${KDCRAW_LIBRARIES}
                     )

install(TARGETS digikamimageplugin_transform             DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES digikamimageplugin_transform_ui.rc         DESTINATION ${DATA_INSTALL_DIR}/digikam)
install(FILES digikamimageplugin_transform.desktop       DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES digiKam-ImagePlugin_Transform.metainfo.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata)

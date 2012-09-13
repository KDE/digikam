SET(digikamimageplugin_transform_PART_SRCS imageplugin_transform.cpp
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

IF(GLIB2_FOUND)
    SET(digikamimageplugin_transform_PART_SRCS ${digikamimageplugin_transform_PART_SRCS}
        contentawareresizetool.cpp
       )
ENDIF(GLIB2_FOUND)

KDE4_ADD_PLUGIN(digikamimageplugin_transform ${digikamimageplugin_transform_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_transform
    digikamcore

    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDCRAW_LIBRARIES}

    ${QT_QT3SUPPORT_LIBRARY}
    )

INSTALL(TARGETS digikamimageplugin_transform DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES digikamimageplugin_transform_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_transform.desktop DESTINATION ${SERVICES_INSTALL_DIR})


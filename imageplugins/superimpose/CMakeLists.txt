
SET(digikamimageplugin_superimpose_PART_SRCS superimposewidget.cpp
                                             superimpose.cpp 
                                             dirselectwidget.cpp
                                             imageplugin_superimpose.cpp
                                             superimposetool.cpp 
   )

KDE4_ADD_PLUGIN(digikamimageplugin_superimpose ${digikamimageplugin_superimpose_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_superimpose
    digikamcore
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KDE3SUPPORT_LIBS}
    ${KDE4_KFILE_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QT3SUPPORT_LIBRARY}
    ${KDCRAW_LIBRARIES}
    )

INSTALL(TARGETS digikamimageplugin_superimpose DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES digikamimageplugin_superimpose_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_superimpose.desktop DESTINATION ${SERVICES_INSTALL_DIR})


INCLUDE_DIRECTORIES(../../imageplugins/coreplugin/hsl
                    ../../imageplugins/coreplugin/ratiocrop
                   )

SET(digikamimageplugin_core_PART_SRCS 
    ../../imageplugins/coreplugin/autocorrectiontool.cpp 
    ../../imageplugins/coreplugin/bcgtool.cpp
    ../../imageplugins/coreplugin/bwsepiatool.cpp
    ../../imageplugins/coreplugin/iccprooftool.cpp
    ../../imageplugins/coreplugin/blurtool.cpp
    ../../imageplugins/coreplugin/imageplugin_core.cpp
    ../../imageplugins/coreplugin/resizetool.cpp
    ../../imageplugins/coreplugin/redeyetool.cpp
    ../../imageplugins/coreplugin/rgbtool.cpp
    ../../imageplugins/coreplugin/sharpentool.cpp
    ../../imageplugins/coreplugin/hsl/hsltool.cpp
    ../../imageplugins/coreplugin/hsl/hspreviewwidget.cpp
    ../../imageplugins/coreplugin/ratiocrop/imageselectionwidget.cpp
    ../../imageplugins/coreplugin/ratiocrop/ratiocroptool.cpp
   )


KDE4_ADD_PLUGIN(digikamimageplugin_core ${digikamimageplugin_core_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_core
    digikamcore
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDCRAW_LIBRARIES}
    )

INSTALL(TARGETS digikamimageplugin_core DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES digikamimageplugin_core_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_core.desktop DESTINATION ${SERVICES_INSTALL_DIR})

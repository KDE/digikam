# Disable liblqr C code warnings.
FOREACH(_curentfile ${liblqr_SRCS})
    SET_SOURCE_FILES_PROPERTIES(${_curentfile} PROPERTIES COMPILE_FLAGS -w)
ENDFOREACH(_curentfile ${liblqr_SRCS})

SET(digikamimageplugin_contentawareresizing_PART_SRCS
    imageplugin_contentawareresizing.cpp
    contentawareresizer.cpp
    contentawareresizetool.cpp
    ${liblqr_SRCS}
   )

KDE4_ADD_PLUGIN(digikamimageplugin_contentawareresizing ${digikamimageplugin_contentawareresizing_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_contentawareresizing
                      digikamcore
                      kdcraw
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KDEUI_LIBS}
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${GLIB2_LIBRARIES}
                      ${liblqr_LIBS}
                     )

INSTALL(TARGETS digikamimageplugin_contentawareresizing DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES digikamimageplugin_contentawareresizing_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_contentawareresizing.desktop DESTINATION ${SERVICES_INSTALL_DIR})

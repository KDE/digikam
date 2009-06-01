
INCLUDE_DIRECTORIES(${GLIB2_INCLUDE_DIR})

IF(LQR-1_FOUND)

    INCLUDE_DIRECTORIES(${LQR-1_INCLUDE_DIRS})
    
    SET(liblqr_LIBS ${LQR-1_LIBRARIES})

ELSE(LQR-1_FOUND)

    INCLUDE_DIRECTORIES(lqr)

    SET(liblqr_SRCS
        lqr/lqr_carver_bias.c
        lqr/lqr_carver.c
        lqr/lqr_carver_list.c
        lqr/lqr_carver_rigmask.c
        lqr/lqr_cursor.c
        lqr/lqr_gradient.c
        lqr/lqr_progress.c
        lqr/lqr_vmap.c
        lqr/lqr_vmap_list.c
        lqr/lqr_energy.c
        lqr/lqr_rwindow.c
       )

    IF(WIN32)
        ADD_DEFINITIONS(-DLQR_DISABLE_DECLSPEC)
    ENDIF(WIN32)

    # Disable liblqr C code warnings.
    FOREACH(_curentfile ${liblqr_SRCS})
        SET_SOURCE_FILES_PROPERTIES(${_curentfile} PROPERTIES COMPILE_FLAGS -w)
    ENDFOREACH(_curentfile ${liblqr_SRCS})

ENDIF(LQR-1_FOUND)

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

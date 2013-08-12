SET_DEBUG_AREA_CODE(${AREA_CODE_SHOWFOTO})

# We need to include the setup dir for showfoto BEFORE the other include dirs,
# otherwise the one from the "utilities" dir will be used
INCLUDE_DIRECTORIES(BEFORE
        ${CMAKE_CURRENT_SOURCE_DIR}/setup
	${CMAKE_CURRENT_SOURCE_DIR}/thumbbar
        ${CMAKE_CURRENT_SOURCE_DIR}/main
)

#Require by Greycstoration algorithm (CImg.h)
IF(NOT MSVC)
    ADD_DEFINITIONS(${KDE4_ENABLE_EXCEPTIONS})
ENDIF(NOT MSVC)

SET(showfoto_SRCS
    ${libsetupshowfoto_SRCS}
    ${libshowfotosetup_SRCS}
    
    thumbbar/showfotoiteminfo.cpp
    thumbbar/showfotoimagemodel.cpp
    thumbbar/showfotothumbnailmodel.cpp
    thumbbar/showfotofiltermodel.cpp
    thumbbar/showfotomodel.cpp
    thumbbar/showfotoitemsortsettings.cpp
    thumbbar/showfotosettings.cpp
    thumbbar/itemviewshowfotodelegate.cpp
    thumbbar/showfotodelegate.cpp
    thumbbar/showfototooltipfiller.cpp
    thumbbar/showfotooverlays.cpp
    thumbbar/showfotodragdrop.cpp
    thumbbar/showfotocategorizedview.cpp
    thumbbar/showfotothumbnailbar.cpp
    
    main/main.cpp
    main/showfoto.cpp
   )

KDE4_ADD_APP_ICON(showfoto_SRCS "../data/icons/apps/hi*-app-showfoto.png")

KDE4_ADD_EXECUTABLE(showfoto ${showfoto_SRCS})

IF(CLAPACK_FOUND)
   TARGET_LINK_LIBRARIES(showfoto ${CLAPACK_LIBRARY})
ENDIF(CLAPACK_FOUND)

TARGET_LINK_LIBRARIES(showfoto
                      digikamcore
                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                      ${KGEOMAP_LIBRARIES}
                      ${KDE4_KIO_LIBS}
                      ${LCMS_LIBRARIES}
                      ${QT_QT3SUPPORT_LIBRARY}

                      # To link under Solaris (see B.K.O #274484)
                      ${MATH_LIBRARY}
                     )

INSTALL(TARGETS showfoto ${INSTALL_TARGETS_DEFAULT_ARGS})

INSTALL(PROGRAMS main/showfoto.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
INSTALL(FILES main/showfotoui.rc DESTINATION ${DATA_INSTALL_DIR}/showfoto )

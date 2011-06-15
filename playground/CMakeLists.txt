# SET_DEBUG_AREA_CODE(${AREA_CODE_SHOWFOTO})

# We need to include the setup dir for showfoto BEFORE the other include dirs,
# otherwise the one from the "utilities" dir will be used
#INCLUDE_DIRECTORIES(BEFORE
#        ${CMAKE_CURRENT_SOURCE_DIR}/setup
#        ${CMAKE_CURRENT_SOURCE_DIR}/main
#)

#Require by Greycstoration algorithm (CImg.h)
IF(NOT MSVC)
    ADD_DEFINITIONS(${KDE4_ENABLE_EXCEPTIONS})
ENDIF(NOT MSVC)

SET(read_QXmlStream_SRCS
    read_QXmlStream.cpp
   )

#KDE4_ADD_APP_ICON(playground_SRCS "../data/icons/apps/hi*-app-showfoto.png")

KDE4_ADD_EXECUTABLE(read_QXmlStream ${read_QXmlStream_SRCS})

#TARGET_LINK_LIBRARIES(playground
#                      digikamdatabase
#
#                      ${QT_QTCORE_LIBRARY}
#                      ${QT_QTGUI_LIBRARY}
#                      ${QT_QTSQL_LIBRARY}
#                      ${QT_QT3SUPPORT_LIBRARY}
#                      ${QT_LIBRARIES}
#                      ${KDE4_KIO_LIBS}
#                      ${KDE4_SOLID_LIBS}
#                      ${KDE4_KDEUI_LIBS}
#                      ${KDE4_KPARTS_LIBS}
#                     )

TARGET_LINK_LIBRARIES(read_QXmlStream
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTSQL_LIBRARY}
                      ${QT_LIBRARIES}
                      ${KDE4_KDEUI_LIBS}
                     )


#INSTALL(TARGETS showfoto ${INSTALL_TARGETS_DEFAULT_ARGS})

#INSTALL(PROGRAMS main/showfoto.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
#INSTALL(FILES main/showfotoui.rc DESTINATION ${DATA_INSTALL_DIR}/showfoto )

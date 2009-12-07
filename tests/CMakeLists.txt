SET( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

INCLUDE_DIRECTORIES(
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/database
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dmetadata
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/models
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/widgets/common
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/widgets/imageplugins
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/widgets/metadata
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/common
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/parser
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/parser/modifiers
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/parser/options
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/cameragui
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/canvas
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/editor
                    ${CMAKE_CURRENT_SOURCE_DIR}/../imageplugins/freerotation
                    ${CMAKE_CURRENT_SOURCE_DIR}/../digikam
                    ${CMAKE_CURRENT_SOURCE_DIR}/..
                    ${CMAKE_CURRENT_BINARY_DIR}/..
                    ${GPHOTO2_INCLUDE_DIRS}
)

#------------------------------------------------------------------------

SET(advancedrenametest_SRCS
    advancedrenametest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/common/modifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/common/option.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/common/parseobject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/common/parseobjectdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/common/parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/common/parseresults.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/common/token.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/parser/defaultrenameparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/parser/modifiers/defaultvaluemodifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/parser/modifiers/firstletterofeachworduppercasemodifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/parser/modifiers/lowercasemodifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/parser/modifiers/rangemodifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/parser/modifiers/replacemodifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/parser/modifiers/trimmedmodifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/parser/modifiers/uniquemodifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/parser/modifiers/uppercasemodifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/parser/options/cameranameoption.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/parser/options/dateoption.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/parser/options/directorynameoption.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/parser/options/filepropertiesoption.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/parser/options/metadataoption.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/advancedrename/parser/options/sequencenumberoption.cpp
)
KDE4_ADD_UNIT_TEST(advancedrenametest ${advancedrenametest_SRCS})
TARGET_LINK_LIBRARIES(advancedrenametest
                      ${KDE4_KDEUI_LIBS}
                      ${KEXIV2_LIBRARIES}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      digikamdatabase
                      digikamcore
                      )

#------------------------------------------------------------------------

SET(cameranamehelper_SRCS
    cameranamehelpertest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/cameragui/cameranamehelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/cameragui/gpcamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/cameragui/dkcamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../digikam/albumsettings.cpp
)
KDE4_ADD_UNIT_TEST(cameranamehelpertest ${cameranamehelper_SRCS})
TARGET_LINK_LIBRARIES(cameranamehelpertest
                      ${KDE4_KDEUI_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${GPHOTO2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                      digikamcore
                      digikamdatabase
                      )

#------------------------------------------------------------------------

SET(uifilevalidator_SRCS
    uifilevalidatortest.cpp
)

KDE4_ADD_UNIT_TEST(uifilevalidatortest ${uifilevalidator_SRCS})
TARGET_LINK_LIBRARIES(uifilevalidatortest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${GPHOTO2_LIBRARIES}
                      digikamcore
                      digikamdatabase
                      )

#------------------------------------------------------------------------

SET(freerotationtest_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../imageplugins/freerotation/freerotation.cpp
    freerotationtest.cpp
)
KDE4_ADD_UNIT_TEST(freerotationtest ${freerotationtest_SRCS})
TARGET_LINK_LIBRARIES(freerotationtest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      digikamcore
                      )
                      
#------------------------------------------------------------------------

SET(filesaveoptionsboxtest_SRCS
    filesaveoptionsboxtest.cpp
)
KDE4_ADD_UNIT_TEST(filesaveoptionsboxtest ${filesaveoptionsboxtest_SRCS})
TARGET_LINK_LIBRARIES(filesaveoptionsboxtest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      digikamcore
                      )

#------------------------------------------------------------------------

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../libs/threadimageio
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/3rdparty/libpgf
                   )

# Disable libpgf warnings.
SET_SOURCE_FILES_PROPERTIES(${libpgf_SRCS} PROPERTIES COMPILE_FLAGS "-w")

SET(qtpgftest_SRCS qtpgftest.cpp ${libpgf_SRCS} ${pgfutils_SRCS})
KDE4_ADD_EXECUTABLE(qtpgftest NOGUI ${qtpgftest_SRCS})
TARGET_LINK_LIBRARIES(qtpgftest ${KDE4_KIO_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

SET(pgfscaled_SRCS pgfscaled.cpp ${libpgf_SRCS} ${pgfutils_SRCS})
KDE4_ADD_EXECUTABLE(pgfscaled NOGUI ${pgfscaled_SRCS})
TARGET_LINK_LIBRARIES(pgfscaled ${KDE4_KIO_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})


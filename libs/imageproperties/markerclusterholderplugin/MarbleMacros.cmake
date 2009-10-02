# this file has been copied from Marble


# the place to put in common cmake macros
# this is needed to minimize the amount of errors to do
macro( marble_add_plugin _target_name )
set( _src ${ARGN} )
if( QTONLY )
    qt4_automoc( ${_src} )
    add_library( ${_target_name} MODULE ${_src} )
    target_link_libraries( ${_target_name} ${QT_QTCORE_LIBRARY}
                                           ${QT_QTDBUS_LIBRARY}
                                           ${QT_QTGUI_LIBRARY}
                                           ${QT_QTXML_LIBRARY}
                                           ${QT_QTSVG_LIBRARY}
                                           ${QT_QTNETWORK_LIBRARY}
                                           ${QT_QTMAIN_LIBRARY}
                                           ${${_target_name}_LIBS}
                                           marblewidget )
    install( TARGETS ${_target_name} DESTINATION ${MARBLE_PLUGIN_INSTALL_PATH} )
else( QTONLY )
    kde4_add_plugin( ${_target_name} ${_src} )
    target_link_libraries( ${_target_name} ${QT_QTCORE_LIBRARY}
                                           ${QT_QTDBUS_LIBRARY}
                                           ${QT_QTGUI_LIBRARY}
                                           ${QT_QTXML_LIBRARY}
                                           ${QT_QTSVG_LIBRARY}
                                           ${QT_QTNETWORK_LIBRARY}
                                           ${KDE4_KDECORE_LIBRARY}
                                           ${KDE4_KDEUI_LIBRARY}
                                           ${KDE4_KIO_LIBRARY}
                                           ${QT_QTMAIN_LIBRARY}
                                           ${${_target_name}_LIBS}
                                           marblewidget )
    install( TARGETS ${_target_name} DESTINATION ${MARBLE_PLUGIN_INSTALL_PATH} )
endif( QTONLY )

set_target_properties( ${_target_name} PROPERTIES 
                       INSTALL_RPATH_USE_LINK_PATH TRUE  
                       SKIP_BUILD_RPATH TRUE 
                       BUILD_WITH_INSTALL_RPATH TRUE 
                       INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${LIB_SUFFIX} )

endmacro( marble_add_plugin _target_name )

# these plugins are slightly different

macro( marble_add_designer_plugin _target_name )
set( _src ${ARGN} )

qt4_add_resources( _src ../../../marble.qrc )

if( QTONLY )
    qt4_automoc( ${_src} )
    add_library( ${_target_name} MODULE ${_src} )
    target_link_libraries( ${_target_name} ${QT_QTCORE_LIBRARY}
                                           ${QT_QTDBUS_LIBRARY}
                                           ${QT_QTGUI_LIBRARY}
                                           ${QT_QTXML_LIBRARY}
                                           ${QT_QTSVG_LIBRARY}
                                           ${QT_QTNETWORK_LIBRARY}
                                           ${QT_QTMAIN_LIBRARY}
                                           ${${_target_name}_LIBS}
                                           marblewidget )
    install( TARGETS ${_target_name} DESTINATION ${QT_PLUGINS_DIR}/designer )
else( QTONLY )
    kde4_add_plugin( ${_target_name} ${_src} )
    target_link_libraries( ${_target_name} ${QT_QTCORE_LIBRARY}
                                           ${QT_QTDBUS_LIBRARY}
                                           ${QT_QTGUI_LIBRARY}
                                           ${QT_QTXML_LIBRARY}
                                           ${QT_QTSVG_LIBRARY}
                                           ${QT_QTNETWORK_LIBRARY}
                                           ${KDE4_KDECORE_LIBRARY}
                                           ${KDE4_KDEUI_LIBRARY}
                                           ${KDE4_KIO_LIBRARY}
                                           ${QT_QTMAIN_LIBRARY}
                                           ${${_target_name}_LIBS}
                                           marblewidget )
    install( TARGETS ${_target_name} DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer )
endif( QTONLY )

set_target_properties( ${_target_name} PROPERTIES 
                       INSTALL_RPATH_USE_LINK_PATH TRUE  
                       SKIP_BUILD_RPATH TRUE 
                       BUILD_WITH_INSTALL_RPATH TRUE 
                       INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${LIB_SUFFIX} )

endmacro( marble_add_designer_plugin _target_name )

if( WIN32 )
    set( DATA_PATH ${CMAKE_INSTALL_PREFIX}/${MARBLE_DATA_PATH} )
    set( PLUGIN_PATH ${CMAKE_INSTALL_PREFIX}/${MARBLE_PLUGIN_PATH} )
else( WIN32 )
    set( DATA_PATH ${MARBLE_DATA_PATH} )
    set( PLUGIN_PATH ${MARBLE_PLUGIN_PATH} )
endif( WIN32 )

macro( marble_add_test TEST_NAME )
    if( BUILD_MARBLE_TESTS )
        set( ${TEST_NAME}_SRCS ${TEST_NAME}.cpp ${ARGN} )
        if( QTONLY )
            qt4_generate_moc( ${TEST_NAME}.cpp ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}.moc )
            include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
            set( ${TEST_NAME}_SRCS ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}.moc ${${TEST_NAME}_SRCS} )
          
            add_executable( ${TEST_NAME} ${${TEST_NAME}_SRCS} )
        else( QTONLY )
            kde4_add_executable( ${TEST_NAME} ${${TEST_NAME}_SRCS} )
        endif( QTONLY )
        target_link_libraries( ${TEST_NAME} ${QT_QTMAIN_LIBRARY}
                                            ${QT_QTCORE_LIBRARY} 
                                            ${QT_QTGUI_LIBRARY} 
                                            ${QT_QTTEST_LIBRARY} 
                                            marblewidget )
        set_target_properties( ${TEST_NAME} PROPERTIES 
                               COMPILE_FLAGS "-DDATA_PATH=\"\\\"${DATA_PATH}\\\"\" -DPLUGIN_PATH=\"\\\"${PLUGIN_PATH}\\\"\"" )
        add_test( ${TEST_NAME} ${TEST_NAME} )
    endif( BUILD_MARBLE_TESTS )
endmacro( marble_add_test TEST_NAME )

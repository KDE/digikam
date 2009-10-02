
IF(MARBLEWIDGET_SUPPORTS_PLUGINS)
  # NOTE: this has been copied from the Marble sources and has to be updated when necessary!
  SET(plugin_dir ${PLUGIN_INSTALL_DIR}/plugins/marble)
  IF (NOT MARBLE_PLUGIN_PATH)
    SET(MARBLE_PLUGIN_PATH ${plugin_dir})
  ENDIF(NOT MARBLE_PLUGIN_PATH)
  IF(NOT MARBLE_PLUGIN_INSTALL_PATH)
    SET(MARBLE_PLUGIN_INSTALL_PATH ${MARBLE_PLUGIN_PATH})
  ENDIF(NOT MARBLE_PLUGIN_INSTALL_PATH)
  INCLUDE(MarbleMacros.cmake)

  PROJECT(externaldraw)

  INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/markerclusterholderplugin/
  ${CMAKE_BINARY_DIR}/libs/imageproperties/markerclusterholderplugin/
  ${QT_INCLUDE_DIR}
  )
  INCLUDE(${QT_USE_FILE})

  SET( externaldraw_sources externaldraw.cpp )

  MARBLE_ADD_PLUGIN( ExternalDraw ${externaldraw_sources} )

ENDIF(MARBLEWIDGET_SUPPORTS_PLUGINS)

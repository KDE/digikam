
# determine whether we can actually build this plugin
# there is no variable for the Marble version, so we
# just test for the existance of a required header
IF(LIBMARBLEWIDGET_INCLUDE_DIR)
  # Digikam sets LIBMARBLEWIDGET_INCLUDE_DIR which the /marble, so we need to check both ways:
  IF(EXISTS ${LIBMARBLEWIDGET_INCLUDE_DIR}/RenderPlugin.h OR EXISTS ${LIBMARBLEWIDGET_INCLUDE_DIR}/marble/RenderPlugin.h)
    SET(MARBLEWIDGET_SUPPORTS_PLUGINS TRUE)
    MESSAGE(STATUS "Can build Marble plugin: yes")
  ELSE(EXISTS ${LIBMARBLEWIDGET_INCLUDE_DIR}/RenderPlugin.h OR EXISTS ${LIBMARBLEWIDGET_INCLUDE_DIR}/marble/RenderPlugin.h)
    SET(MARBLEWIDGET_SUPPORTS_PLUGINS FALSE)
    MESSAGE(STATUS "Can build Marble plugin: no")
  ENDIF(EXISTS ${LIBMARBLEWIDGET_INCLUDE_DIR}/RenderPlugin.h OR EXISTS ${LIBMARBLEWIDGET_INCLUDE_DIR}/marble/RenderPlugin.h)
ENDIF(LIBMARBLEWIDGET_INCLUDE_DIR)


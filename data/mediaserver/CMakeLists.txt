#
# Ahmed Fathy
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.



    file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/herqq_avtransport_scpd.xml")
    file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/herqq_connectionmanager_sink_scpd.xml")
    file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/herqq_connectionmanager_source_scpd.xml")
    file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/herqq_contentdirectory_scpd.xml")
    file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/herqq_mediarenderer_description.xml")
    file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/herqq_mediaserver_description.xml")
    file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/herqq_renderingcontrol_scpd.xml")
    file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/icon.png")



install(FILES ${files}
        DESTINATION ${DATA_INSTALL_DIR}/digikam/mediaserver/descriptions)


include_directories( ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas ${CMAKE_SOURCE_DIR}/digikam/libs/histogram ${CMAKE_SOURCE_DIR}/digikam/libs/levels ${CMAKE_SOURCE_DIR}/digikam/libs/curves ${CMAKE_SOURCE_DIR}/digikam/libs/whitebalance ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/digikam ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(digikamimageplugin_perspective_PART_SRCS ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp )

kde4_automoc(${digikamimageplugin_perspective_PART_SRCS})

kde4_add_plugin(digikamimageplugin_perspective ${digikamimageplugin_perspective_PART_SRCS})

target_link_libraries(digikamimageplugin_perspective  ${KDE4_KDECORE_LIBS} )

install(TARGETS digikamimageplugin_perspective DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  digikamimageplugin_perspective_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam )
install( FILES  digikamimageplugin_perspective.desktop DESTINATION ${SERVICES_INSTALL_DIR} )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
#	       -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
#	       -I$(top_srcdir)/digikam/libs/histogram \
#	       -I$(top_srcdir)/digikam/libs/levels \
#	       -I$(top_srcdir)/digikam/libs/curves \
#	       -I$(top_srcdir)/digikam/libs/whitebalance \
#	       -I$(top_srcdir)/digikam/libs/widgets/common \
#	       -I$(top_srcdir)/digikam/libs/widgets/iccprofiles \
#	       -I$(top_srcdir)/digikam/libs/widgets/imageplugins \
#	       -I$(top_srcdir)/digikam/libs/dialogs \
#	       -I$(top_srcdir)/digikam/libs/dimg \
#	       -I$(top_srcdir)/digikam/libs/dmetadata \
#	       -I$(top_srcdir)/digikam/libs/dimg/filters \
#	       -I$(top_srcdir)/digikam/digikam \
#	       $(all_includes)  
#
#digikamimageplugin_perspective_la_SOURCES = imageplugin_perspective.cpp \
#	                                        imageeffect_perspective.cpp \
#					                        perspectivewidget.cpp triangle.cpp matrix.cpp
#
#digikamimageplugin_perspective_la_LIBADD = $(LIB_KPARTS) \
#	                                       $(top_builddir)/digikam/digikam/libdigikam.la 
#	
#digikamimageplugin_perspective_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#
#kde_services_DATA = digikamimageplugin_perspective.desktop
#
#kde_module_LTLIBRARIES = digikamimageplugin_perspective.la
#
#rcdir = $(kde_datadir)/digikam
#rc_DATA = digikamimageplugin_perspective_ui.rc
#

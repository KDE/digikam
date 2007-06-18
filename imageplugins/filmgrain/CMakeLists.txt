
include_directories( ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas ${CMAKE_SOURCE_DIR}/digikam/libs/histogram ${CMAKE_SOURCE_DIR}/digikam/libs/levels ${CMAKE_SOURCE_DIR}/digikam/libs/curves ${CMAKE_SOURCE_DIR}/digikam/libs/whitebalance ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/digikam   )


########### next target ###############

set(digikamimageplugin_filmgrain_PART_SRCS ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp )

kde4_automoc(${digikamimageplugin_filmgrain_PART_SRCS})

kde4_add_plugin(digikamimageplugin_filmgrain ${digikamimageplugin_filmgrain_PART_SRCS})

target_link_libraries(digikamimageplugin_filmgrain  ${KDE4_KDECORE_LIBS} )

install(TARGETS digikamimageplugin_filmgrain DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  digikamimageplugin_filmgrain_ui.rc         DESTINATION ${DATA_INSTALL_DIR}/digikam )
install( FILES  digikamimageplugin_filmgrain.desktop DESTINATION ${SERVICES_INSTALL_DIR} )



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
#digikamimageplugin_filmgrain_la_SOURCES = imageplugin_filmgrain.cpp \
#	                                      imageeffect_filmgrain.cpp filmgrain.cpp 
#
#digikamimageplugin_filmgrain_la_LIBADD = $(LIB_KPARTS) \
#	                                     $(top_builddir)/digikam/digikam/libdigikam.la 
#
#digikamimageplugin_filmgrain_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 
#
#kde_services_DATA = digikamimageplugin_filmgrain.desktop
#
#kde_module_LTLIBRARIES = digikamimageplugin_filmgrain.la
#
#rcdir = $(kde_datadir)/digikam
#rc_DATA = digikamimageplugin_filmgrain_ui.rc        
#

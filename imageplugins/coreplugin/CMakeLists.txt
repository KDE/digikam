
add_subdirectory( sharpnesseditor )
add_subdirectory( hsl )
add_subdirectory( ratiocrop )
add_subdirectory( sharpnesseditor )
add_subdirectory( hsl )
add_subdirectory( ratiocrop )

include_directories( ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas ${CMAKE_SOURCE_DIR}/digikam/libs/histogram ${CMAKE_SOURCE_DIR}/digikam/libs/levels ${CMAKE_SOURCE_DIR}/digikam/libs/curves ${CMAKE_SOURCE_DIR}/digikam/libs/whitebalance ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/hsl ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/ratiocrop ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(digikamimageplugin_core_PART_SRCS ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp )

kde4_automoc(${digikamimageplugin_core_PART_SRCS})

kde4_add_plugin(digikamimageplugin_core ${digikamimageplugin_core_PART_SRCS})

target_link_libraries(digikamimageplugin_core  ${KDE4_KDECORE_LIBS} )

install(TARGETS digikamimageplugin_core DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  digikamimageplugin_core_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam )
install( FILES  digikamimageplugin_core.desktop DESTINATION ${SERVICES_INSTALL_DIR} )



#original Makefile.am contents follow:

#SUBDIRS = sharpnesseditor hsl ratiocrop 
#COMPILE_FIRST = sharpnesseditor hsl ratiocrop 
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
#	       -I$(top_srcdir)/digikam/imageplugins/coreplugin/sharpnesseditor \
#	       -I$(top_srcdir)/digikam/imageplugins/coreplugin/hsl \
#	       -I$(top_srcdir)/digikam/imageplugins/coreplugin/ratiocrop \
#	       $(all_includes) 
#
#digikamimageplugin_core_la_SOURCES = imageplugin_core.cpp imageeffect_bwsepia.cpp \
#	                                 imageeffect_autocorrection.cpp \
#	                                 imageeffect_rgb.cpp \
#	                                 imageeffect_redeye.cpp imageeffect_blur.cpp \
#	                                 imageeffect_iccproof.cpp imageeffect_bcg.cpp 
#
#noinst_HEADERS = imageeffect_autocorrection.h imageeffect_blur.h \
#                 imageeffect_rgb.h imageeffect_bcg.h \
#                 imageeffect_bwsepia.h imageeffect_iccproof.h imageeffect_redeye.h 
#
#digikamimageplugin_core_la_LIBADD = $(LIB_KPARTS) \
#                                    $(top_builddir)/digikam/imageplugins/coreplugin/sharpnesseditor/libsharpnesseditor.la \
#	                            $(top_builddir)/digikam/imageplugins/coreplugin/hsl/libhsl.la \
#	                            $(top_builddir)/digikam/imageplugins/coreplugin/ratiocrop/libratiocrop.la \
#	                            $(top_builddir)/digikam/digikam/libdigikam.la
#
#digikamimageplugin_core_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#
#kde_services_DATA = digikamimageplugin_core.desktop
#
#kde_module_LTLIBRARIES = digikamimageplugin_core.la
#
#rcdir = $(kde_datadir)/digikam
#rc_DATA = digikamimageplugin_core_ui.rc

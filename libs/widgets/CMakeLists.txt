include_directories( ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor/
		${CMAKE_SOURCE_DIR}/libs/widgets/common/
		${CMAKE_SOURCE_DIR}/libs/dmetadata/)

add_subdirectory( metadata )
add_subdirectory( imageplugins )
add_subdirectory( common )

########### next target ###############

SET(libcommonwidgets_SRCS 
	common/histogramwidget.cpp 
	common/colorgradientwidget.cpp 
	common/curveswidget.cpp 
	common/sidebar.cpp 
	common/squeezedcombobox.cpp 
	common/filesaveoptionsbox.cpp 
	common/dpopupmenu.cpp 
	common/statuszoombar.cpp 
	common/statusnavigatebar.cpp 
	common/statusprogressbar.cpp 
	common/dcursortracker.cpp 
	common/paniconwidget.cpp 
	common/previewwidget.cpp )


set(libiccprofileswidgets_SRCS
	iccprofiles/iccprofilewidget.cpp
	iccprofiles/cietonguewidget.cpp
	iccprofiles/iccpreviewwidget.cpp
	)

set(libmetadatawidgets_SRCS
	metadata/metadatalistview.cpp
	metadata/metadatalistviewitem.cpp
	metadata/metadatawidget.cpp
	metadata/iptcwidget.cpp
	metadata/exifwidget.cpp
	metadata/mdkeylistviewitem.cpp
	metadata/makernotewidget.cpp
	metadata/gpswidget.cpp
	metadata/worldmapwidget.cpp
	)

set(libimagepluginswidgets_SRCS
	imageplugins/imageregionwidget.cpp
	imageplugins/imagepaniconwidget.cpp
	imageplugins/imageguidewidget.cpp
	imageplugins/imagepannelwidget.cpp
	imageplugins/imagewidget.cpp
	imageplugins/listboxpreviewitem.cpp
	)


SET(libwidgets_STAT_SRCS ${libimagepluginswidgets_SRCS} ${libmetadatawidgets_SRCS} ${libiccprofileswidgets_SRCS} ${libcommonwidgets_SRCS})

KDE4_ADD_LIBRARY(libwidgets STATIC ${libwidgets_STAT_SRCS})



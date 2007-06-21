INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor/
                     ${CMAKE_SOURCE_DIR}/libs/widgets/common/
                     ${CMAKE_SOURCE_DIR}/libs/dmetadata/
                   )

ADD_SUBDIRECTORY( common )
ADD_SUBDIRECTORY( iccprofiles )
ADD_SUBDIRECTORY( metadata )
ADD_SUBDIRECTORY( imageplugins )


SET(libcommonwidgets_SRCS common/colorgradientwidget.cpp 
                          common/filesaveoptionsbox.cpp 
                          common/statuszoombar.cpp 
                          common/statusnavigatebar.cpp 
                          common/statusprogressbar.cpp 
                          common/dcursortracker.cpp 
                          common/sidebar.cpp 
                          common/dpopupmenu.cpp 
                          common/squeezedcombobox.cpp 
                          common/histogramwidget.cpp 
                          common/curveswidget.cpp 
                          common/paniconwidget.cpp 
                          common/previewwidget.cpp 
   )


SET(libiccprofileswidgets_SRCS iccprofiles/iccprofilewidget.cpp
                               iccprofiles/cietonguewidget.cpp
                               iccprofiles/iccpreviewwidget.cpp
   )

SET(libmetadatawidgets_SRCS metadata/metadatalistview.cpp
                            metadata/metadatalistviewitem.cpp
                            metadata/metadatawidget.cpp
                            metadata/iptcwidget.cpp
                            metadata/exifwidget.cpp
                            metadata/mdkeylistviewitem.cpp
                            metadata/makernotewidget.cpp
                            metadata/gpswidget.cpp
                            metadata/worldmapwidget.cpp
   )

SET(libimagepluginswidgets_SRCS imageplugins/imageregionwidget.cpp
                                imageplugins/imagepaniconwidget.cpp
                                imageplugins/imageguidewidget.cpp
                                imageplugins/imagepannelwidget.cpp
                                imageplugins/imagewidget.cpp
                                imageplugins/listboxpreviewitem.cpp
   )


SET(libwidgets_STAT_SRCS ${libcommonwidgets_SRCS}
                         ${libiccprofileswidgets_SRCS} 
                         ${libmetadatawidgets_SRCS} 
                         ${libimagepluginswidgets_SRCS} 
   )

KDE4_AUTOMOC(${libwidgets_STAT_SRCS})

KDE4_ADD_LIBRARY(libwidgets STATIC ${libwidgets_STAT_SRCS})

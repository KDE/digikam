
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/histogram 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/themeengine 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/metadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/database 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/cameragui 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/batch 
                   )


# Static library for Camera GUI #############################################

# SET(libimagepropertiescamgui_STAT_SRCS imagepropertiessidebarcamgui.cpp 
#                                        cameraitempropertiestab.cpp
#    )
# 
# KDE4_AUTOMOC(${libimagepropertiescamgui_STAT_SRCS})
# 
# KDE4_ADD_LIBRARY(libimagepropertiescamgui ${libimagepropertiescamgui_STAT_SRCS})


# Static library for Showfoto ###############################################

SET(libimagepropertiesshowfoto_STAT_SRCS navigatebarwidget.cpp 
                                         navigatebartab.cpp
                                         imagepropertiessidebar.cpp 
                                         imagepropertiestab.cpp 
                                         imagepropertiesmetadatatab.cpp 
                                         imagepropertiescolorstab.cpp 
   )

KDE4_AUTOMOC(${libimagepropertiesshowfoto_STAT_SRCS})

KDE4_ADD_LIBRARY(libimagepropertiesshowfoto ${libimagepropertiesshowfoto_STAT_SRCS})


# Static library for digiKam ################################################

SET(libimagepropertiesdigikam_STAT_SRCS navigatebarwidget.cpp 
                                        navigatebartab.cpp
                                        talbumlistview.cpp 
                                        imagepropertiessidebar.cpp 
                                        imagepropertiessidebardb.cpp 
                                        imagepropertiestab.cpp 
                                        imagedescedittab.cpp 
                                        imagepropertiesmetadatatab.cpp 
                                        imagepropertiescolorstab.cpp 
   )

KDE4_AUTOMOC(${libimagepropertiesdigikam_STAT_SRCS})

KDE4_ADD_LIBRARY(libimagepropertiesdigikam_STAT_SRCS ${libimagepropertiesdigikam_STAT_SRCS})


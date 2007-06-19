
INCLUDE_DIRECTORIES( ${KIPI_INCLUDE_DIR} 
                     ${GPHOTO2_INCLUDE_DIR} 
                     ${KDCRAW_INCLUDE_DIR} 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/batch 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/loaders 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common 
                   )

# Static library for digiKam ############################################

SET(libsetup_STAT_SRCS setup.cpp
                       setuptooltip.cpp 
                       setupidentity.cpp 
                       setupmime.cpp 
                       setupslideshow.cpp 
                       setupmetadata.cpp 
                       setupeditor.cpp
                       setupmisc.cpp 
                       setuplighttable.cpp
                       setupiofiles.cpp 
                       setupplugins.cpp 
                       setupdcraw.cpp 
                       gpiface.cpp 
                       cameraselection.cpp 
                       setupcamera.cpp 
                       setupgeneral.cpp 
                       setupcollections.cpp 
                       setupicc.cpp 
   )

KDE4_AUTOMOC(${libsetup_STAT_SRCS})

KDE4_ADD_LIBRARY(libsetup STATIC ${libsetup_STAT_SRCS})

TARGET_LINK_LIBRARIES(libsetup ${GPHOTO2_LIBRARIES})


# Static library for Showfoto ##########################################

SET(libsetupshowfoto_STAT_SRCS setupiofiles.cpp 
                               setupdcraw.cpp 
                               setupicc.cpp 
                               setupslideshow.cpp
   )

KDE4_AUTOMOC(${libsetupshowfoto_STAT_SRCS})

KDE4_ADD_LIBRARY(libsetupshowfoto STATIC ${libsetupshowfoto_STAT_SRCS})


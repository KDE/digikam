
INCLUDE_DIRECTORIES(  ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters 
                      ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                      ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs
                      ${CMAKE_SOURCE_DIR}/digikam/libs/histogram 
                      ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio
                      ${CMAKE_SOURCE_DIR}/digikam/utilities/splashscreen
                      ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins
                      ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common
                   )

SET(libdimgcanvas_STAT_SRCS imageplugin.cpp 
                            dimginterface.cpp 
                            colorcorrectiondlg.cpp 
                            canvas.cpp 
                            undocache.cpp 
                            undoaction.cpp
                            undomanager.cpp 
                            imagepluginloader.cpp 
   )

KDE4_AUTOMOC(${libdimgcanvas_STAT_SRCS})

KDE4_ADD_LIBRARY(libdimgcanvas STATIC ${libdimgcanvas_STAT_SRCS})

INSTALL( FILES imageplugin.h DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )


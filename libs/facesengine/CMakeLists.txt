#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

kde_enable_exceptions()


include_directories($<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/..
                    ${CMAKE_CURRENT_SOURCE_DIR}/database
                    ${CMAKE_CURRENT_SOURCE_DIR}/database/core
                    ${CMAKE_CURRENT_SOURCE_DIR}/detection
                    ${CMAKE_CURRENT_SOURCE_DIR}/recognition-opencv-lbph
                    ${CMAKE_CURRENT_SOURCE_DIR}/alignment-congealing

#                   ${CMAKE_CURRENT_SOURCE_DIR}/alignment-flandmark
#                   ${CMAKE_CURRENT_SOURCE_DIR}/preprocessing-tantriggs
)

set(facesengine_LIB_SRCS detection/opencvfacedetector.cpp
                         recognition-opencv-lbph/lbphfacemodel.cpp
                         recognition-opencv-lbph/opencvlbphfacerecognizer.cpp
                         recognition-opencv-lbph/facerec_borrowed.cpp
                         facedetector.cpp
                         identity.cpp
                         dataproviders.cpp
                         alignment-congealing/funnelreal.cpp
                         recognitiondatabase.cpp

#                        alignment-flandmark/flandmarkaligner.cpp
#                        preprocessing-tantriggs/tantriggspreprocessor.cpp
)

set(facesengine_database_LIB_SRCS database/trainingdb.cpp
                                  database/databasefaceparameters.cpp
                                  database/databasefacecontainers.cpp
                                  database/databasefaceoperationgroup.cpp
                                  database/databasefaceschemaupdater.cpp
                                  database/databasefacebackend.cpp
                                  database/databasefaceconfig.cpp
                                  database/databasefaceaccess.cpp
)

add_library(digikamfacesengine_src OBJECT ${facesengine_LIB_SRCS} ${facesengine_database_LIB_SRCS} )

add_library(digikamfacesengine STATIC $<TARGET_OBJECTS:digikamfacesengine_src>)

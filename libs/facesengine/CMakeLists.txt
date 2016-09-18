#
# Copyright (c) 2010-2016, Gilles Caulier, <caulier dot gilles at gmail dot com>
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
                    $<TARGET_PROPERTY:Qt5::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/..
                    ${CMAKE_CURRENT_SOURCE_DIR}/facedb
                    ${CMAKE_CURRENT_SOURCE_DIR}/detection
                    ${CMAKE_CURRENT_SOURCE_DIR}/recognition-opencv-lbph
                    ${CMAKE_CURRENT_SOURCE_DIR}/alignment-congealing
                    ${CMAKE_CURRENT_SOURCE_DIR}/shape-predictor

#                   ${CMAKE_CURRENT_SOURCE_DIR}/alignment-flandmark
#                   ${CMAKE_CURRENT_SOURCE_DIR}/preprocessing-tantriggs
)

if(ENABLE_OPENCV3)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/opencv3-face)

    set(opencv3face_LIB_SRCS opencv3-face/facerec.cpp
                             opencv3-face/lbph_faces.cpp
                             opencv3-face/eigen_faces.cpp
                             opencv3-face/fisher_faces.cpp
                             opencv3-face/predict_collector.cpp
    )
endif()

set(facesengine_LIB_SRCS detection/opencvfacedetector.cpp
                         shape-predictor/fullobjectdetection.cpp
                         shape-predictor/qdatastreamoverloads.cpp
                         facedetector.cpp
                         identity.cpp
                         dataproviders.cpp

#                        alignment-flandmark/flandmarkaligner.cpp
#                        preprocessing-tantriggs/tantriggspreprocessor.cpp
)

set(facesengine_database_LIB_SRCS recognitiondatabase.cpp
                                  recognition-opencv-lbph/lbphfacemodel.cpp
                                  recognition-opencv-lbph/opencvlbphfacerecognizer.cpp
                                  recognition-opencv-lbph/opencvmatdata.cpp
                                  recognition-opencv-lbph/facerec_borrowed.cpp
                                  alignment-congealing/funnelreal.cpp
                                  facedb/facedbaccess.cpp
                                  facedb/facedbbackend.cpp
                                  facedb/facedboperationgroup.cpp
                                  facedb/facedbschemaupdater.cpp
                                  facedb/facedb.cpp
)

add_library(digikamfacesengine_src
            OBJECT
            ${opencv3face_LIB_SRCS}             # Empty with OpenCV2
            ${facesengine_LIB_SRCS}
)

add_library(digikamfacesengine STATIC $<TARGET_OBJECTS:digikamfacesengine_src>)

add_library(digikamfacesenginedatabase_src
            OBJECT
            ${facesengine_database_LIB_SRCS}
)

add_library(digikamfacesenginedatabase STATIC $<TARGET_OBJECTS:digikamfacesenginedatabase_src>)

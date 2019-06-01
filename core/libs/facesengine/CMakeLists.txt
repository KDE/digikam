#
# Copyright (c) 2010-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

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
                    ${CMAKE_CURRENT_SOURCE_DIR}/detection/opencv3-face
                    ${CMAKE_CURRENT_SOURCE_DIR}/recognition/opencv-lbph
                    ${CMAKE_CURRENT_SOURCE_DIR}/recognition/opencv-eigenfaces
                    ${CMAKE_CURRENT_SOURCE_DIR}/recognition/opencv-fisherfaces
                    ${CMAKE_CURRENT_SOURCE_DIR}/alignment/congealing
                    ${CMAKE_CURRENT_SOURCE_DIR}/preprocessing/shape-predictor

#                   ${CMAKE_CURRENT_SOURCE_DIR}/alignment/flandmark
#                   ${CMAKE_CURRENT_SOURCE_DIR}/preprocessing/tantriggs
)

if(ENABLE_FACESENGINE_DNN)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/recognition/dlib-dnn/dnnface/dnn_base
                        ${CMAKE_CURRENT_SOURCE_DIR}/recognition/dlib-dnn/dnnface/image_processing
                        ${CMAKE_CURRENT_SOURCE_DIR}/recognition/dlib-dnn/dnnface/image_transforms
                        ${CMAKE_CURRENT_SOURCE_DIR}/recognition/dlib-dnn/dnnface/matrix
                        ${CMAKE_CURRENT_SOURCE_DIR}/recognition/dlib-dnn/dnnface/nn
                        ${CMAKE_CURRENT_SOURCE_DIR}/recognition/dlib-dnn/dnnface
                        ${CMAKE_CURRENT_SOURCE_DIR}/recognition/dlib-dnn
    )
endif()

set(facesengine_LIB_SRCS detection/opencvfacedetector.cpp
                         preprocessing/shape-predictor/fullobjectdetection.cpp
                         preprocessing/shape-predictor/qdatastreamoverloads.cpp
                         facedetector.cpp
                         identity.cpp
                         dataproviders.cpp

                         detection/opencv3-face/facerec.cpp
                         detection/opencv3-face/lbph_faces.cpp
                         detection/opencv3-face/eigen_faces.cpp
                         detection/opencv3-face/fisher_faces.cpp
                         detection/opencv3-face/predict_collector.cpp

#                        alignment/flandmark/flandmarkaligner.cpp
#                        preprocessing/tantriggs/tantriggspreprocessor.cpp
)

set(facesengine_database_LIB_SRCS recognitiondatabase.cpp
                                  simpletrainingdataprovider.cpp

                                  # Eigen Faces recognition module based on OpenCV
                                  recognition/opencv-eigenfaces/eigenfacemodel.cpp
                                  recognition/opencv-eigenfaces/opencveigenfacerecognizer.cpp
                                  recognition/opencv-eigenfaces/facerec_eigenborrowed.cpp

                                  # Fisher Faces recognition module based on OpenCV
                                  recognition/opencv-fisherfaces/fisherfacemodel.cpp
                                  recognition/opencv-fisherfaces/opencvfisherfacerecognizer.cpp
                                  recognition/opencv-fisherfaces/facerec_fisherborrowed.cpp


                                  # Lpbh Faces recognition module based on OpenCV
                                  recognition/opencv-lbph/lbphfacemodel.cpp
                                  recognition/opencv-lbph/opencvlbphfacerecognizer.cpp
                                  recognition/opencv-lbph/opencvmatdata.cpp
                                  recognition/opencv-lbph/facerec_borrowed.cpp

                                  alignment/congealing/funnelreal.cpp

                                  facedb/facedbaccess.cpp
                                  facedb/facedbbackend.cpp
                                  facedb/facedboperationgroup.cpp
                                  facedb/facedbschemaupdater.cpp
                                  facedb/facedb.cpp
)

if(ENABLE_FACESENGINE_DNN)

    if(NOT MSVC)

    message(STATUS "Before:${CMAKE_C_FLAGS}")
    message(STATUS "Before:${CMAKE_CXX_FLAGS}")

        # Dlib DNN generate plenty of wrong warnings. Remove these workaround later when code will be ported to OpenCV DNN.
        while(CMAKE_CXX_FLAGS MATCHES "-Wcast-align")
            string(REPLACE "-Wcast-align" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        endwhile()

        while(CMAKE_CXX_FLAGS MATCHES "-Wextra")
            string(REPLACE "-Wextra" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        endwhile()

        while(CMAKE_CXX_FLAGS MATCHES "-Wundef")
            string(REPLACE "-Wundef" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        endwhile()

        while(CMAKE_CXX_FLAGS MATCHES "-Wall")
            string(REPLACE "-Wall" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        endwhile()

    message(STATUS "After:${CMAKE_C_FLAGS}")
    message(STATUS "After:${CMAKE_CXX_FLAGS}")

    endif()

    set(facesengine_database_LIB_SRCS ${facesengine_database_LIB_SRCS}
                                      # Neural NetWork Faces recognition module based on Dlib
                                      recognition/dlib-dnn/dnnfacemodel.cpp
                                      recognition/dlib-dnn/opencvdnnfacerecognizer.cpp
                                      recognition/dlib-dnn/facerec_dnnborrowed.cpp
    )
endif()

add_library(digikamfacesengine_src
            OBJECT
            ${facesengine_LIB_SRCS}
)

add_library(digikamfacesengine STATIC $<TARGET_OBJECTS:digikamfacesengine_src>)

add_library(digikamfacesenginedatabase_src
            OBJECT
            ${facesengine_database_LIB_SRCS}
)

add_library(digikamfacesenginedatabase STATIC $<TARGET_OBJECTS:digikamfacesenginedatabase_src>)

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
                    ${CMAKE_CURRENT_SOURCE_DIR}/preprocessing
#                   ${CMAKE_CURRENT_SOURCE_DIR}/preprocessing/recognition

#                   ${CMAKE_CURRENT_SOURCE_DIR}/alignment/flandmark
#                   ${CMAKE_CURRENT_SOURCE_DIR}/preprocessing/tantriggs
)

# if(ENABLE_FACESENGINE_DNN)
#     include_directories(${CMAKE_CURRENT_SOURCE_DIR}/recognition/dlib-dnn/dnnface/dnn_base
#                         ${CMAKE_CURRENT_SOURCE_DIR}/recognition/dlib-dnn/dnnface/image_processing
#                         ${CMAKE_CURRENT_SOURCE_DIR}/recognition/dlib-dnn/dnnface/image_transforms
#                         ${CMAKE_CURRENT_SOURCE_DIR}/recognition/dlib-dnn/dnnface/matrix
#                         ${CMAKE_CURRENT_SOURCE_DIR}/recognition/dlib-dnn/dnnface/nn
#                         ${CMAKE_CURRENT_SOURCE_DIR}/recognition/dlib-dnn/dnnface
#                         ${CMAKE_CURRENT_SOURCE_DIR}/recognition/dlib-dnn
#     )
# endif()

set(facesengine_LIB_SRCS detection/opencvfacedetector.cpp
                         preprocessing/shape-predictor/fullobjectdetection.cpp
                         preprocessing/shape-predictor/qdatastreamoverloads.cpp
                         facedetector.cpp
                         identity.cpp
                         dataproviders.cpp
#                        preprocessing/recognition/recognitionpreprocessor.cpp
#                        preprocessing/recognition/openfacepreprocessor.cpp
                         preprocessing/shape-predictor/matrixoperations.cpp
                         preprocessing/shape-predictor/pointtransformaffine.cpp
                         preprocessing/shape-predictor/shapepredictor.cpp

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

        # Dlib DNN generate plenty of wrong warnings. Remove these workaround later when code will be ported to OpenCV DNN.
        REMOVE_GCC_COMPILER_WARNINGS("-Wextra")
        REMOVE_GCC_COMPILER_WARNINGS("-Wundef")
        REMOVE_GCC_COMPILER_WARNINGS("-Wall")

        DISABLE_CLANG_COMPILER_WARNINGS("4.99.99" "-Wno-cast-align")
        DISABLE_CLANG_COMPILER_WARNINGS("4.99.99" "-Wno-sign-compare")
        DISABLE_CLANG_COMPILER_WARNINGS("4.99.99" "-Wno-unused-parameter")
        DISABLE_CLANG_COMPILER_WARNINGS("4.99.99" "-Wno-unused-variable")

    endif()

    set(facesengine_database_LIB_SRCS ${facesengine_database_LIB_SRCS}
                                      # Neural NetWork Faces recognition module based on Dlib
                                      recognition/dlib-dnn/dnnfacemodel.cpp
                                      recognition/dlib-dnn/opencvdnnfacerecognizer.cpp
                                      recognition/dlib-dnn/facerec_dnnborrowed.cpp
                                      recognition/dlib-dnn/dnnfaceextractor.cpp
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

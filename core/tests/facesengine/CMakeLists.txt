#
# Copyright (c) 2010-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

kde_enable_exceptions()

# -----------------------------------------------------------------------------

set(detect_SRCS detect.cpp)
add_executable(detect ${detect_SRCS})
target_link_libraries(detect
                      digikamcore
                      digikamgui
                      digikamfacesengine
                      digikamdatabase

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Sql

                      ${OpenCV_LIBRARIES}
)

# -----------------------------------------------------------------------------

set(recognize_SRCS recognize.cpp)
add_executable(recognize ${recognize_SRCS})
target_link_libraries(recognize
                      digikamcore
                      digikamgui
                      digikamfacesengine
                      digikamdatabase

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Sql

                      ${OpenCV_LIBRARIES}
)

# -----------------------------------------------------------------------------

set(align_SRCS align.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/facesengine/alignment-congealing/funnelreal.cpp
)
add_executable(align ${align_SRCS})
target_link_libraries(align
                      digikamcore
                      digikamgui
                      digikamfacesengine
                      digikamdatabase

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      Qt5::Sql

                      ${OpenCV_LIBRARIES}
)

# -----------------------------------------------------------------------------

set(preprocess_SRCS preprocess.cpp)
add_executable(preprocess ${preprocess_SRCS})
target_link_libraries(preprocess
                      digikamcore
                      digikamgui
                      digikamfacesengine
                      digikamdatabase

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Sql

                      ${OpenCV_LIBRARIES}
)

# -----------------------------------------------------------------------------

set(traindb_SRCS traindb.cpp)
add_executable(traindb ${traindb_SRCS})
target_link_libraries(traindb
                      digikamcore
                      digikamgui
                      digikamfacesengine
                      digikamdatabase

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Sql

                      ${OpenCV_LIBRARIES}
)

# -----------------------------------------------------------------------------

set(facesenginedemo_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/demo/main.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/demo/mainwindow.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/demo/faceitem.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/demo/button.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/demo/marquee.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/demo/fancyrect.cpp
)

qt5_wrap_ui(facesenginedemo_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/demo/mainwindow.ui)
add_executable(facesenginedemo ${facesenginedemo_SRCS})

target_link_libraries(facesenginedemo
                      digikamcore
                      digikamgui
                      digikamfacesengine
                      digikamdatabase

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Sql

                      ${OpenCV_LIBRARIES}
)

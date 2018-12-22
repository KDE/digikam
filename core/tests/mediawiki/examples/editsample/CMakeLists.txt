#
# Copyright (c) 2010-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(editsample_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
)

qt5_wrap_ui(editsample_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.ui)

add_executable(editsample
               ${editsample_SRCS}
)


target_link_libraries(editsample
                      PRIVATE
                      Qt5::Core
                      Qt5::Network
                      Qt5::Widgets
                      digikamcore
)

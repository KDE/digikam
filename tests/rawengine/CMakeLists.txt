#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


include_directories(
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
)

set(raw2png_SRCS raw2png.cpp)
add_executable(raw2png ${raw2png_SRCS})
target_link_libraries(raw2png
                      digikamcore
                      Qt5::Gui
                      Qt5::Core
)

set(libinfo_SRCS libinfo.cpp)
add_executable(libinfo ${libinfo_SRCS})
target_link_libraries(libinfo
                      digikamcore
                      Qt5::Gui
                      Qt5::Core
)

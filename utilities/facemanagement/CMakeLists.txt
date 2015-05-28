#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libfacemanagement_SRCS
    faceutils.cpp
    facegroup.cpp
    facepipeline.cpp
    facescandialog.cpp
    assignnamewidget.cpp
)

add_library(facemanagement STATIC ${libfacemanagement_SRCS})

target_link_libraries(facemanagement

                      digikamdatabase
                      digikamtags
                      fileactionmanager

                      ${KF5KFace_LIBRARIES}
)

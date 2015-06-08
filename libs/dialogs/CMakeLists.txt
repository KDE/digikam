#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libdialogs_SRCS
    dprogressdlg.cpp
    dbusydlg.cpp
    filesaveoptionsdlg.cpp
    iccprofileinfodlg.cpp
    imagedialog.cpp
    infodlg.cpp
    libsinfodlg.cpp
    rawcameradlg.cpp
)

set(libdeletedialog_SRCS
    migrationdlg.cpp # used to belong to separate target (database dependency?) verify FIXME
    deletedialog.cpp
)

if(LENSFUN_FOUND)
    include_directories(${LENSFUN_INCLUDE_DIRS})
endif()

#used by digikamcore
add_library(digikamdialogscore_src OBJECT ${libdialogs_SRCS})

add_library(digikamdialogs_src OBJECT ${libdialogs_SRCS} ${libdeletedialog_SRCS})
add_library(digikamdialogs STATIC
                           $<TARGET_OBJECTS:digikamdialogs_src>
)

target_link_libraries(digikamdialogs
                      PRIVATE
                      threadimageio   # loadingdescription for imagedialog
                      digikamdatabase_static # migrationdlg needs this
                      versionmanager  # migrationdlg needs this too
                      dmetadata
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      Qt5::Sql # for databaseurl

                      KF5::KDcraw
)

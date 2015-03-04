#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libalbum_SRCS
    album.cpp
    albumhistory.cpp
    albummanager.cpp
    albummodificationhelper.cpp
    albumpropsedit.cpp
    albumselectors.cpp
    albumselectcombobox.cpp
    albumselectdialog.cpp
    albumselectiontreeview.cpp
    albumselectwidget.cpp
    albumthumbnailloader.cpp
    albumtreeview.cpp
    albumwatch.cpp
    albumlabelstreeview.cpp
)

add_library(digikamalbum STATIC ${libalbum_SRCS})

target_link_libraries(digikamalbum

                      digikamdatabase
                      setup            # for setupcollections.h, FIXME
                      digikamwidgets   # for fullscreensettings

                      Qt5::Core

                      KF5::I18n
)

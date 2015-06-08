#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libtags_SRCS
    addtagscombobox.cpp
    addtagscompletionbox.cpp
    addtagslineedit.cpp
    tagcheckview.cpp
    tagfolderview.cpp
    taggingaction.cpp
    tagslineeditoverlay.cpp
    tagspopupmenu.cpp
    tageditdlg.cpp
    tagmodificationhelper.cpp
    tagsactionmngr.cpp

    tagsmanager/tagsmanager.cpp
    tagsmanager/tagpropwidget.cpp
    tagsmanager/taglist.cpp
    tagsmanager/tagmngrtreeview.cpp
    tagsmanager/models/tagmngrlistview.cpp
    tagsmanager/models/tagmngrlistmodel.cpp
    tagsmanager/models/tagmngrlistitem.cpp
)

include_directories(tagsmanager/
                    tagsmanager/models/
)

add_library(digikamtags_src OBJECT ${libtags_SRCS})

add_library(digikamtags STATIC $<TARGET_OBJECTS:digikamtags_src>)

target_include_directories(digikamtags PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tagsmanager>
)

target_link_libraries(digikamtags
                      PRIVATE
                      digikamalbum
                      lighttable
                      imageeditor
                      maintenance

                      KF5::IconThemes
)

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

include_directories(tagsmanager/ tagsmanager/models/)

add_library(digikamtags STATIC ${libtags_SRCS})

target_include_directories(digikamtags PUBLIC 
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tagsmanager>
)

target_link_libraries(digikamtags
                         digikamalbum
)

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
                      setup # for setupcollections.h, FIXME
)

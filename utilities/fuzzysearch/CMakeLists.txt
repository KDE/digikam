set(libfuzzysearch_SRCS
    fuzzysearchview.cpp
    findduplicatesview.cpp
    findduplicatesalbum.cpp
    findduplicatesalbumitem.cpp
    sketchwidget.cpp
)

add_library(fuzzysearch STATIC ${libfuzzysearch_SRCS})
target_include_directories(fuzzysearch PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_link_libraries(fuzzysearch
                         digikamgui # for album..
                         digikamwidgets
                         Qt5::Widgets
)
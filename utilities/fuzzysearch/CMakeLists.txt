set(libfuzzysearch_SRCS
    fuzzysearchview.cpp
    findduplicatesview.cpp
    findduplicatesalbum.cpp
    findduplicatesalbumitem.cpp
    sketchwidget.cpp
)

add_library(fuzzysearch STATIC ${libfuzzysearch_SRCS})
target_link_libraries(fuzzysearch
                      digikamalbum
                      digikamwidgets
)

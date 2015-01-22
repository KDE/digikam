set(libfuzzysearch_SRCS
    fuzzysearchview.cpp
    findduplicatesview.cpp
    findduplicatesalbum.cpp
    findduplicatesalbumitem.cpp
    sketchwidget.cpp
)

add_library(fuzzysearch ${libfuzzysearch_SRCS})
target_link_libraries(fuzzysearch
                         )
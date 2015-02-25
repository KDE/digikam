set(libgpssearch_SRCS
    gpssearchview.cpp
    gpsmarkertiler.cpp
)

add_library(gpssearch ${libgpssearch_SRCS})
target_link_libraries(libgpssearch
                         
)
set(libgpssearch_SRCS
    gpssearchview.cpp
    gpsmarkertiler.cpp
)

add_library(gpssearch STATIC ${libgpssearch_SRCS})

target_link_libraries(gpssearch
                      KF5::KGeoMap
                      digikamdatabase
                      digikamtags
                      fileactionmanager
)
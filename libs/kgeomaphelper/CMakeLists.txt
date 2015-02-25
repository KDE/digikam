
set(libkgeomaphelper_SRCS digikam2kgeomap.cpp)

add_library(kgeomaphelper STATIC ${libkgeomaphelper_SRCS})
target_link_library(kgeomaphelper
                    KF5::KGeoMap
)

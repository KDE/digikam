set(libkde3rdparty_SRCS
    kmemoryinfo/kmemoryinfo.cpp
    kinotify/kinotify.cpp
)

add_library(3rdparty ${libkde3rdparty_SRCS})

target_link_libraries(3rdparty
                         Qt5::Core
)
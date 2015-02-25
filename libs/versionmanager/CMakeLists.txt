set(libversionmanager_SRCS
    versionmanager.cpp
)

add_library(versionmanager STATIC ${libversionmanager_SRCS})
target_link_libraries(versionmanager
                         dimg
)
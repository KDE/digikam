set(libdmetadata_SRCS
    metadatasettings.cpp
    metadatasettingscontainer.cpp
    metadatainfo.cpp
    infocontainer.cpp
    dmetadata.cpp
    geodetictools.cpp
    template.cpp
    captionvalues.cpp
)

add_library(dmetadata STATIC ${libdmetadata_SRCS})

target_link_libraries(dmetadata
                      Qt5::Core
                      dimg
)
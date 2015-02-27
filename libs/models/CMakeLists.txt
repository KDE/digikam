set(libdatabasemodels_SRCS
    imagemodel.cpp
    imagefiltermodel.cpp
    imagefiltermodelpriv.cpp
    imagefiltermodelthreads.cpp
    imagefiltersettings.cpp
    imagelistmodel.cpp
    imagesortsettings.cpp
    imagethumbnailmodel.cpp
    imageversionsmodel.cpp
)

set(libalbummodels_SRCS
    imagealbummodel.cpp
    imagealbumfiltermodel.cpp
    abstractalbummodel.cpp
    albummodel.cpp
    albumfiltermodel.cpp
    albummodeldragdrophandler.cpp
)

set(libgenericmodels_SRCS
    categorizeditemmodel.cpp
)

add_library(digikammodels STATIC ${libdatabasemodels_SRCS} ${libalbummodels_SRCS} ${libgenericmodels_SRCS})
target_link_libraries(digikammodels
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      Qt5::Sql
                      Qt5::DBus

                      filters
)

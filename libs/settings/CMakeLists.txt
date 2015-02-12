set(libsettings_SRCS
    applicationsettings.cpp
    applicationsettings_p.cpp
    applicationsettings_albums.cpp
    applicationsettings_database.cpp
    applicationsettings_iconview.cpp
    applicationsettings_mime.cpp
    applicationsettings_miscs.cpp
    applicationsettings_tooltips.cpp
)

add_library(digikamsettings STATIC ${libsettings_SRCS})

target_link_libraries(digikamsettings
                      Qt5::Gui
                      Qt5::Widgets

                      digikamwidgets #required for DMultiTabbar 
)

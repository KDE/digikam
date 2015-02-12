set(libfilters_SRCS
    filterstatusbar.cpp
    filtersidebarwidget.cpp
    geolocationfilter.cpp
    textfilter.cpp
    mimefilter.cpp
    colorlabelfilter.cpp
    picklabelfilter.cpp
    ratingfilter.cpp
    tagfilterview.cpp
)

add_library(filters STATIC ${libfilters_SRCS})

target_link_libraries(filters
                      Qt5::Gui
                      Qt5::Widgets
                      
                      digikammodels
                      digikamwidgets
                      digikamsettings
)

set(libfilters_SRCS
    filterstatusbar.cpp
    geolocationfilter.cpp
    textfilter.cpp
    mimefilter.cpp
    colorlabelfilter.cpp
    picklabelfilter.cpp
    ratingfilter.cpp
)

#TODO rename to digikamfilters?

add_library(filters STATIC ${libfilters_SRCS})

target_link_libraries(filters
                      Qt5::Gui
                      Qt5::Widgets
                      
                      digikamalbum
                      digikammodels
                      digikamwidgets
                      digikamsettings
)

set(libsearchwindow_SRCS
    searchwindow.cpp
    searchview.cpp
    searchgroup.cpp
    searchfieldgroup.cpp
    searchfields.cpp
    searchutilities.cpp
    ratingsearchutilities.cpp
    choicesearchutilities.cpp
    searchfolderview.cpp
    searchtabheader.cpp
)

add_library(searchwindow STATIC ${libsearchwindow_SRCS})
target_link_libraries(searchwindow
                         digikamgui # drop gui requirement..
                         digikamdatabase
                         
                         Qt5::Widgets
)

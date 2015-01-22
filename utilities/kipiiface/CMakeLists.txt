set(libkipiiface_SRCS
    kipiimagecollection.cpp
    kipiimagecollectionselector.cpp
    kipiuploadwidget.cpp
    kipiinterface.cpp
    kipiimageinfo.cpp
    kipipluginloader.cpp
)

add_library(kipiiface STATIC ${libkipiiface_SRCS})

target_link_libraries(kipiiface
                      digikamgui # for digikamapp
                      
                      KF5::Kipi                      
)


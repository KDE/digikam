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
                      digikamcore
                      digikamgui # for digikamapp
                      #Qt5::Core
                      #Qt5::Sql
                      
                      #KF5::KIOCore
                      
                      ${KF5Kipi_LIBRARIES}
)


set(libslideshow_SRCS
    slidetoolbar.cpp
    slideosd.cpp
    slideproperties.cpp
    slideimage.cpp
    slideerror.cpp
    slideend.cpp
    slideshow.cpp
    slidehelp.cpp
    slideshowsettings.cpp
)

add_library(slideshow STATIC ${libslideshow_SRCS})
target_link_libraries(slideshow
                      digikamwidgets
                      digikamdatabase
                      digikamalbum
                      
                      Qt5::Widgets
                      
                      KF5::KDELibs4Support
                      KF5::WidgetsAddons
                      KF5::KDcraw
                      KF5::KExiv2
)

set(libimageproperties_SRCS
    imagepropertiessidebar.cpp
    imagepropertiestab.cpp
    imagepropertiesmetadatatab.cpp
    imagepropertiescolorstab.cpp
   )

if(${KF5KGeoMap_FOUND})
    set(libimageproperties_SRCS
        ${libimageproperties_SRCS}
        imagepropertiesgpstab.cpp
        imagegpsmodelhelper.cpp
       )
endif()

set(libimagepropertiesdigikam_SRCS
    ${libimageproperties_SRCS}
    imagepropertiessidebarcamgui.cpp
    imagepropertiessidebardb.cpp
    cameraitempropertiestab.cpp
    imagedescedittab.cpp
    captionedit.cpp
)

add_library(imagepropertiesdigikam STATIC ${libimageproperties_SRCS} ${libimagepropertiesdigikam_SRCS})

if(${KF5KGeoMap_FOUND})
    target_link_libraries(imagepropertiesdigikam KF5::KGeoMap)
endif()
#add_library(imageproperties STATIC ${libimageproperties_SRCS})

target_link_libraries(imagepropertiesdigikam

                      digikamgui # required for Album FIXME
                      
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      Qt5::Sql
                      
                      KF5::KDELibs4Support # for KMultiTabBar
                      KF5::ConfigCore # KConfigGroup?
)
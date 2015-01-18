set(libsetupshowfoto_SRCS
    setupeditor.cpp
    setupiofiles.cpp
    setupraw.cpp
    setupicc.cpp
    setupslideshow.cpp
)

set(libsetup_SRCS
    setupicc.cpp
    setupcollections.cpp
    setupcollectionview.cpp
    setupcategory.cpp
    setupalbumview.cpp
    setup.cpp
    setuptooltip.cpp
    setuptemplate.cpp
    setupmime.cpp
    setupslideshow.cpp
    setupmetadata.cpp
    setupeditor.cpp
    setupmisc.cpp
    setuplighttable.cpp
    setupiofiles.cpp
    setupraw.cpp
    setupcamera.cpp
    cameraselection.cpp
    cameralist.cpp
    cameratype.cpp
    setupdatabase.cpp
    setupversioning.cpp
    importfilters.cpp
    setupimagequalitysorter.cpp
)

add_library(setup STATIC ${libsetup_SRCS} ${libsetupshowfoto_SRCS})
target_link_libraries(setup
                      digikamcore # is this needed?
                      digikamtemplate

                      #Qt5::Core
                      Qt5::Widgets
                      #Qt5::Sql
                      
                      #KF5::KDELibs4Support # for KLineEdit
)
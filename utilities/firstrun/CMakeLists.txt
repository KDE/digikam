set(libfirstrun_SRCS
    assistantdlg.cpp
    assistantdlgpage.cpp
    welcomepage.cpp
    collectionpage.cpp
    rawpage.cpp
    tooltipspage.cpp
    previewpage.cpp
    openfilepage.cpp
    metadatapage.cpp
    startscanpage.cpp
)

add_library(firstrun STATIC ${libfirstrun_SRCS})
target_link_libraries(firstrun
                      digikamdatabase
                      digikamcore # only for logging! FIXME
                      Qt5::Widgets
                      
                      KF5::WidgetsAddons
                      KF5::ConfigWidgets
                      KF5::KIOWidgets
                      
                      KF5::KDcraw
)
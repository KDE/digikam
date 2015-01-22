set(libprogressmanager_SRCS
    overlaywidget.cpp
    progressview.cpp
    progressmanager.cpp
    statusbarprogresswidget.cpp
    statusprogressbar.cpp
)

add_library(progressmanager STATIC ${libprogressmanager_SRCS})
target_link_libraries(progressmanager
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      
                      KF5::KDcraw # for rwidgetutils.h
                      KF5::KDELibs4Support # for KSqueezedLabel
)
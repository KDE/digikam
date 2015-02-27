set(libtemplate_SRCS
    templatemanager.cpp
    templatelist.cpp
    templateselector.cpp
    templateviewer.cpp
    templatepanel.cpp
    subjectedit.cpp
)

add_library(digikamtemplate STATIC ${libtemplate_SRCS})

target_link_libraries(digikamtemplate
                        dmetadata
                      Qt5::Core
                      Qt5::Widgets
                      Qt5::Xml
                      
                      KF5::WidgetsAddons
)
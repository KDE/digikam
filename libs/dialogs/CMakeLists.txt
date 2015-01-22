set(libdialogs_SRCS
    dprogressdlg.cpp
    dbusydlg.cpp
    filesaveoptionsdlg.cpp
    iccprofileinfodlg.cpp
    imagedialog.cpp
    infodlg.cpp
    libsinfodlg.cpp
    rawcameradlg.cpp
    triplechoicedialog.cpp
    
    migrationdlg.cpp # used to belong to separate target, verify FIXME
)

set(libdeletedialog_SRCS
    deletedialog.cpp
)

add_library(digikamdialogs STATIC ${libdialogs_SRCS} ${libdeletedialog_SRCS})

target_link_libraries(digikamdialogs
                      dmetadata
                      
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      Qt5::Sql # for databaseurl
                      
                      KF5::KDELibs4Support # # KSqueezedTextLabel
                      
                      KF5::KDcraw
)
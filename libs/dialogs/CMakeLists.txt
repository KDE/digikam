
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/metadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas 
                   )

# Static library for digiKam ####################################################

SET(libdialog_STAT_SRCS iccprofileinfodlg.cpp 
                        dprogressdlg.cpp
			deletedialog.cpp 
			imagedlgbase.cpp 
			imageguidedlg.cpp 
                        ctrlpaneldlg.cpp 
   )

KDE4_ADD_UI_FILES(libdialog_STAT_SRCS deletedialogbase.ui)

KDE4_AUTOMOC(${libdialog_STAT_SRCS})

KDE4_ADD_LIBRARY(libdialog STATIC ${libdialog_STAT_SRCS})


# Static library for Showfoto ####################################################

SET(libdialogshowfoto_STAT_SRCS iccprofileinfodlg.cpp)

KDE4_AUTOMOC(${libdialogshowfoto_STAT_SRCS})

KDE4_ADD_LIBRARY(libdialogshowfoto STATIC ${libdialogshowfoto_STAT_SRCS})


INSTALL( FILES ctrlpaneldlg.h imagedlgbase.h imageguidedlg.h iccprofileinfodlg.h dprogressdlg.h 
         DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )


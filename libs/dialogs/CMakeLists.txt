
include_directories( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/metadata ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


set(libdialog_STAT_SRCS ctrlpaneldlg.cpp imagedlgbase.cpp imageguidedlg.cpp iccprofileinfodlg.cpp deletedialog.cpp dprogressdlg.cpp)

kde4_add_ui3_files(libdialog_STAT_SRCS deletedialogbase.ui)

kde4_automoc(${libdialog_STAT_SRCS})

KDE4_ADD_LIBRARY(libdialog STATIC ${libdialog_STAT_SRCS})

########### next target ###############

set(libdialogshowfoto_STAT_SRCS iccprofileinfodlg.cpp)

kde4_automoc(${libdialogshowfoto_STAT_SRCS})

KDE4_ADD_LIBRARY(libdialogshowfoto STATIC ${libdialogshowfoto_STAT_SRCS})

########### next target ###############


########### install files ###############

install( FILES  ctrlpaneldlg.h imagedlgbase.h imageguidedlg.h  	                     iccprofileinfodlg.h dprogressdlg.h   DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )


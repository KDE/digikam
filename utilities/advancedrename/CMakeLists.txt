FILE(GLOB libadvancedrename_SRCS 
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/parser/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database/keys/*.cpp
)

KDE4_ADD_UI_FILES(libadvancedrename_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/fillmodifierdialogwidget.ui
        ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/replacemodifierdialogwidget.ui
        ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/rangemodifierdialogwidget.ui
        ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/dateoptiondialogwidget.ui
        ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/sequencenumberoptiondialogwidget.ui
)

KDE4_ADD_LIBRARY(advancedrename STATIC ${libadvancedrename_SRCS})

TARGET_LINK_LIBRARIES(advancedrename
        ${KEXIV2_LIBRARIES}
        ${KDCRAW_LIBRARIES}
        digikamdatabase
)

# sources
FILE(GLOB libexternaltools_SRCS 
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

# ui files
FILE(GLOB libexternaltools_UI 
        ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

KDE4_ADD_UI_FILES(libexternaltools_SRCS ${libexternaltools_UI})

KDE4_ADD_LIBRARY(externaltools STATIC ${libexternaltools_SRCS})

TARGET_LINK_LIBRARIES(externaltools
        digikamdatabase
)

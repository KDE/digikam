
include_directories( ${CMAKE_SOURCE_DIR}/digikam/utilities/setup ${CMAKE_SOURCE_DIR}/digikam/digikam   )

set(libsetup_STAT_SRCS setup.cpp setupeditor.cpp setuptooltip.cpp)

KDE4_AUTOMOC(${libsetup_STAT_SRCS})

KDE4_ADD_LIBRARY(libsetup STATIC ${libsetup_STAT_SRCS})

TARGET_LINK_LIBRARIES(libsetup ${GPHOTO2_LIBRARIES})

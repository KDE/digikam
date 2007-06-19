
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/utilities/setup 
                     ${CMAKE_SOURCE_DIR}/digikam/digikam 
		   )

SET(libsetup_STAT_SRCS setuptooltip.cpp
                       setup.cpp 
                       setupeditor.cpp
   )

KDE4_AUTOMOC(${libsetup_STAT_SRCS})

KDE4_ADD_LIBRARY(libsetup STATIC ${libsetup_STAT_SRCS})

TARGET_LINK_LIBRARIES(libsetup libshowfotosetup)
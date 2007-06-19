
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/utilities/setup 
                     ${CMAKE_SOURCE_DIR}/digikam/digikam 
		   )

SET(libshowfotosetup_STAT_SRCS setuptooltip.cpp
                               setup.cpp 
                               setupeditor.cpp
   )

KDE4_AUTOMOC(${libshowfotosetup_STAT_SRCS})

KDE4_ADD_LIBRARY(libshowfotosetup STATIC ${libshowfotosetup_STAT_SRCS})

TARGET_LINK_LIBRARIES(libshowfotosetup libsetupshowfoto)
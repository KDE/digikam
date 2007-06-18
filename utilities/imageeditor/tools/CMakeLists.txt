
include_directories(  ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/greycstoration ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor   )

set(libdimgeditortools_STAT_SRCS imageresize.cpp imageprint.cpp)
KDE4_AUTOMOC(${libdimgeditortools_STAT_SRCS})
KDE4_ADD_LIBRARY(libdimgeditortools STATIC ${libdimgeditortools_STAT_SRCS})
#


include_directories( ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/thumbbar ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/database ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )



set(libbatch_STAT_SRCS batchthumbsgenerator.cpp batchalbumssyncmetadata.cpp imageinfojob.cpp imageinfoalbumsjob.cpp batchsyncmetadata.cpp)
kde4_automoc(${libbatch_STAT_SRCS})
KDE4_ADD_LIBRARY(libbatch STATIC ${libbatch_STAT_SRCS})

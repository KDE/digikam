
INCLUDE_DIRECTORIES(
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/thumbbar 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/database 
                     ${KDE4_INCLUDE_DIR} 
                     ${QT_INCLUDES}
                   )

SET(libbatch_STAT_SRCS imageinfojob.cpp 
                       batchthumbsgenerator.cpp 
                       batchalbumssyncmetadata.cpp 
                       imageinfoalbumsjob.cpp 
                       batchsyncmetadata.cpp
   )

KDE4_AUTOMOC(${libbatch_STAT_SRCS})

KDE4_ADD_LIBRARY(libbatch STATIC ${libbatch_STAT_SRCS})

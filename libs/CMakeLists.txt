include_directories( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/loaders
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters
                     ${CMAKE_SOURCE_DIR}/digikam/libs/whitebalance
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/histogram
                     ${CMAKE_SOURCE_DIR}/digikam/libs/curves
                     ${CMAKE_SOURCE_DIR}/digikam/libs/levels
                     ${CMAKE_SOURCE_DIR}/digikam/libs/lprof/
                     ${CMAKE_SOURCE_DIR}/digikam/libs/jpegutils 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/greycstoration
                     ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio

                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/metadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/imageproperties
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs

                     ${CMAKE_SOURCE_DIR}/digikam/libs/thumbbar

                     ${JPEG_INCLUDE_DIR}
                     ${TIFF_INCLUDE_DIR}
                     ${PNG_INCLUDE_DIR}
                     ${JASPER_INCLUDE_DIR}
                   )

add_subdirectory( lprof )
add_subdirectory( curves )
add_subdirectory( levels )
add_subdirectory( histogram )
add_subdirectory( dmetadata )
add_subdirectory( dimg )
add_subdirectory( greycstoration )
add_subdirectory( dialogs )
add_subdirectory( widgets )
add_subdirectory( database )
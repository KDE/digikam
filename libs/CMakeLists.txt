include_directories(
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg
                     ${CMAKE_SOURCE_DIR}/digikam/digikam
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


include_directories( ${CMAKE_SOURCE_DIR}/digikam/digikam 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg 
                   )

add_subdirectory( scripts )
add_subdirectory( setup )
add_subdirectory( imageeditor )
add_subdirectory( hotplug )
add_subdirectory( batch )
add_subdirectory( lighttable )
add_subdirectory( cameragui )


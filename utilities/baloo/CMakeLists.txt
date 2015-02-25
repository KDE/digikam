set(baloowrap_SRCS baloowrap.cpp)

add_library(baloowrap STATIC ${baloowrap_SRCS})

target_include_directories(baloowrap PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(baloowrap digikamdatabase KF5::Baloo KF5::FileMetaData)

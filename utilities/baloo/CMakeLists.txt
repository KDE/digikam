set(baloowrap_SRCS baloowrap.cpp)

add_library(baloowrap STATIC ${baloowrap_SRCS})

target_link_libraries(baloowrap digikamdatabase KF5::Baloo KF5::FileMetaData)

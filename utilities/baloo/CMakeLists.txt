set(baloowrap_SRCS baloowrap.cpp)

add_library(baloowrap SHARED ${baloowrap_SRCS})

target_link_libraries(baloowrap KF5::I18n KF5::Baloo KF5::FileMetaData)

install(TARGETS baloowrap DESTINATION lib)

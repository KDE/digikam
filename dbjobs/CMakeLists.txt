set(libdbjobs_SRCS
    dbjobsthread.cpp
    datesjob.cpp
)

add_library(dbjobs STATIC ${libdbjobs_SRCS})

target_link_libraries(dbjobs
                      digikamdatabase
                      KF5::KDcraw
                      Qt5::Core
)

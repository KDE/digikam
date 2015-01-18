set(libdthread_SRCS
    threadmanager.cpp
    workerobject.cpp
    dynamicthread.cpp
    parallelworkers.cpp
)

add_library(dthread STATIC ${libdthread_SRCS})

message(INFO "rename dthread to something else?")
target_link_libraries(dthread
                      digikamcore
                      
                      Qt5::Core
)

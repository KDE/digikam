# Enable C++ Exceptions support, required by pgfutils
kde_source_files_enable_exceptions(pgfutils.cpp)

include_directories(${PGF_INCLUDE_DIRS})

set(libpgfutils_SRCS pgfutils.cpp)

# TODO needs exports to build a dynamic lib if wanted.
add_library(pgfutils STATIC ${libpgfutils_SRCS})

target_include_directories(pgfutils PUBLIC ${PGF_INCLUDE_DIRS})

target_link_libraries(pgfutils
                         Qt5::Gui
                         ${PGF_LIBRARIES}
)

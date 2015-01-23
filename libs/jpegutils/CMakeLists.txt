# Define libjpeg_EXPORTS: kde-win emerged libjpeg uses this define to
# decide wether to make dllimport (by default) or dllexport. So, we need to export.
add_definitions(-Dlibjpeg_EXPORTS)

set(libjpegutils_SRCS
    jpegutils.cpp
    iccjpeg.c
    ${DIGIKAM_LIBJPEG_DIR}/transupp.c
)

if(MSVC)
    set(libjpegutils_SRCS
        ${libjpegutils_SRCS}
        jpegwin.cpp
    )
endif()

add_library(jpegutils STATIC ${libjpegutils_SRCS})

target_include_directories(jpegutils
                              PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${DIGIKAM_LIBJPEG_DIR}>
)

target_link_libraries(jpegutils
                      dimg
                      dmetadata
                      
                      Qt5::Core
                      Qt5::Gui
                      ${JPEG_LIBRARIES}
)
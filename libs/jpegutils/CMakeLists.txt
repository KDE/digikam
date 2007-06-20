## "Enable Final" triggers: http://bugs.kde.org/show_bug.cgi?id=126326
## digikam: camera download: auto-rotated images lose EXIF info ...
## So make sure nofinal is always used here!
KDE4_NO_ENABLE_FINAL(libjpegutils)

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata )

SET(libjpegutils_STAT_SRCS jpegutils.cpp transupp.cpp)

KDE4_ADD_LIBRARY(libjpegutils STATIC ${libjpegutils_STAT_SRCS})

TARGET_LINK_LIBRARIES(libjpegutils ${JPEG_LIBRARY})


set(libfacemanagement_SRCS
    faceutils.cpp
    facegroup.cpp
    facepipeline.cpp
    facescandialog.cpp
    assignnamewidget.cpp
)

add_library(facemanagement STATIC ${libfacemanagement_SRCS})

target_link_libraries(facemanagement
                      KF5::KFace
                      digikamdatabase
                      digikamtags
                      fileactionmanager
)

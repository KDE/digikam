#for more info look at http://qtnode.net/wiki/Qt4_with_cmake

INCLUDE(${QT_USE_FILE})
# the variable "scriptiface_SRCS" contains all .cpp files of this project
SET(scriptiface_SRCS
    scriptiface.cpp
)

SET(scriptiface_UIS
    scriptiface.ui
)

QT4_WRAP_UI(scriptiface_UIS_H ${scriptiface_UIS})

##TARGET_LINK_LIBRARIES(${QT_QTSCRIPT_LIBRARY},${QT_QTSCRIPTTOOLS_LIBRARY})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INSTALL( FILES ui_scriptiface.h DESTINATION ${DATA_INSTALL_DIR}/digikam )
#ADD_EXECUTABLE(scripiface ${scripiface_SRCS} ${scripiface_UIS_H} )


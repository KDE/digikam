set(fileactionmanager_SRCS
    metadatahub.cpp
    fileactionprogress.cpp
    fileactionmngr.cpp
    fileactionmngr_p.cpp
    imageinfotasksplitter.cpp
    databaseworkeriface.cpp
    fileworkeriface.cpp
    fileactionimageinfolist.cpp
)

add_library(fileactionmanager STATIC ${fileactionmanager_SRCS})
target_link_libraries(fileactionmanager
    progressmanager
    dmetadata
)
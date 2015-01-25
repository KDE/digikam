#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

add_subdirectory(imgqsort)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

#------------------------------------------------------------------------

set(advancedrenametest_SRCS
    advancedrenametest.cpp
)

add_executable(advancedrenametest ${advancedrenametest_SRCS})
add_test(advancedrenametest advancedrenametest)
ecm_mark_as_test(advancedrenametest)

target_link_libraries(advancedrenametest
                      Qt5::Test

                      digikamdatabase
                      advancedrename
)

#------------------------------------------------------------------------

set(renamecustomizertest_SRCS
    renamecustomizertest.cpp
)

add_executable(renamecustomizertest ${renamecustomizertest_SRCS})
add_test(renamecustomizertest renamecustomizertest)
ecm_mark_as_test(renamecustomizertest)

# TODO clean this, requires some weird libs atm
target_link_libraries(renamecustomizertest
                      Qt5::Gui
                      Qt5::Test
                      KF5::KDELibs4Support
                      digikamdatabase
                      importui
                      advancedrename
                      #KF5::KExiv2
                      #KF5::KDcraw
                      #${OpenCV_LIBRARIES}
)

#------------------------------------------------------------------------

set(cameranamehelper_SRCS
    cameranamehelpertest.cpp
)

add_executable(cameranamehelpertest ${cameranamehelper_SRCS})
add_test(cameranamehelpertest cameranamehelpertest)
ecm_mark_as_test(cameranamehelpertest)

target_link_libraries(cameranamehelpertest
                      #Qt5::Gui
                      #Qt5::Widgets
                      Qt5::Test
                      #KF5::KDELibs4Support
                      digikamdatabase
                      #KF5::KDcraw
                      #KF5::KExiv2
                      #${OpenCV_LIBRARIES}
)

if(GPHOTO2_FOUND)
    target_link_libraries(cameranamehelpertest ${GPHOTO2_LIBRARIES})
endif()

#------------------------------------------------------------------------

set(uifilevalidator_SRCS
    uifilevalidatortest.cpp
)

add_executable(uifilevalidatortest ${uifilevalidator_SRCS})
add_test(uifilevalidatortest uifilevalidatortest)
ecm_mark_as_test(uifilevalidatortest)

target_link_libraries(uifilevalidatortest
                      Qt5::Test
                      digikamdatabase
)

if(GPHOTO2_FOUND)
    target_link_libraries(uifilevalidatortest ${GPHOTO2_LIBRARIES})
endif()

#------------------------------------------------------------------------

set(statesavingobject_SRCS
    statesavingobjecttest.cpp
)

add_executable(statesavingobjecttest ${statesavingobject_SRCS})
add_test(statesavingobjecttest statesavingobjecttest)
ecm_mark_as_test(statesavingobjecttest)

target_link_libraries(statesavingobjecttest
                      digikamwidgets
                      Qt5::Test
)

if(GPHOTO2_FOUND)
    target_link_libraries(statesavingobjecttest ${GPHOTO2_LIBRARIES})
endif()

#------------------------------------------------------------------------

set(freerotationtest_SRCS
    freerotationtest.cpp
)

add_executable(freerotationtest ${freerotationtest_SRCS})
add_test(freerotationtest freerotationtest)
ecm_mark_as_test(freerotationtest)

target_link_libraries(freerotationtest
                      dimg
                      Qt5::Test
)

#------------------------------------------------------------------------

set(searchtextbartest_SRCS
    searchtextbartest.cpp
)

add_executable(searchtextbartest ${searchtextbartest_SRCS})
add_test(searchtextbartest searchtextbartest)
ecm_mark_as_test(searchtextbartest)

target_link_libraries(searchtextbartest
                      digikamwidgets
                      Qt5::Test
)

#------------------------------------------------------------------------

set(filesaveoptionsboxtest_SRCS
    filesaveoptionsboxtest.cpp
)

add_executable(filesaveoptionsboxtest ${filesaveoptionsboxtest_SRCS})
add_test(filesaveoptionsboxtest filesaveoptionsboxtest)
ecm_mark_as_test(filesaveoptionsboxtest)

target_link_libraries(filesaveoptionsboxtest
                      digikamwidgets
                      Qt5::Test
)

#------------------------------------------------------------------------

#set(albummodeltest_SRCS
#    albummodeltest.cpp
#    modeltest/modeltest.cpp
#)

#add_executable(albummodeltest ${albummodeltest_SRCS})
#add_test(albummodeltest albummodeltest)
#ecm_mark_as_test(albummodeltest)

#target_link_libraries(albummodeltest
#                      Qt5::Test
#                      KF5::KIOCore
#                      KF5::KDELibs4Support

#                      digikamdatabase
#)

#------------------------------------------------------------------------

set(dimagehistorytest_SRCS
    abstractdimagehistorytest.cpp
    dimagehistorytest.cpp
)

add_executable(dimagehistorytest ${dimagehistorytest_SRCS})
add_test(dimagehistorytest dimagehistorytest)
ecm_mark_as_test(dimagehistorytest)

target_link_libraries(dimagehistorytest
                      dimg
                      Qt5::Test
)

#------------------------------------------------------------------------

set(dimagehistorygraphtest_SRCS
    abstractdimagehistorytest.cpp
    dimagehistorygraphtest.cpp
    modeltest/modeltest.cpp
)


add_executable(dimagehistorygraphtest ${dimagehistorygraphtest_SRCS})
add_test(dimagehistorygraphtest dimagehistorygraphtest)
ecm_mark_as_test(dimagehistorygraphtest)

target_link_libraries(dimagehistorygraphtest
                      dimg
                      Qt5::Test
                      digikamdatabase
)

#------------------------------------------------------------------------

set(dimagefilteractiontest_SRCS
    dimagefilteractiontest.cpp
)

add_executable(dimagefilteractiontest ${dimagefilteractiontest_SRCS})
add_test(dimagefilteractiontest dimagefilteractiontest)
ecm_mark_as_test(dimagefilteractiontest)

target_link_libraries(dimagefilteractiontest
                      dimg
                      Qt5::Test
)

#------------------------------------------------------------------------

# undefined reference to `Digikam::DatabaseFields::FieldMetaInfo<QFlags<Digikam::DatabaseFields::ImagesField> >::Last'
# happens even though file defining those libs/database/framework/databasefields.h is included and linking against digikamdatabase..
#set(databasefieldstest_SRCS
#    databasefieldstest.cpp
#)

#add_executable(databasefieldstest ${databasefieldstest_SRCS})
#add_test(databasefieldstest databasefieldstest)
#ecm_mark_as_test(databasefieldstest)

#target_link_libraries(databasefieldstest
#                      digikamdatabase
#                      Qt5::Gui
#                      Qt5::Test
#                      KF5::KDELibs4Support
#                      KF5::KExiv2
#)

#------------------------------------------------------------------------

set(qtpgftest_SRCS qtpgftest.cpp ${pgfutils_SRCS})
add_executable(qtpgftest ${qtpgftest_SRCS})
ecm_mark_nongui_executable(qtpgftest)
target_link_libraries(qtpgftest
                      threadimageio # for PGFUtils
)

set(pgfscaled_SRCS pgfscaled.cpp ${pgfutils_SRCS})
add_executable(pgfscaled ${pgfscaled_SRCS})
ecm_mark_nongui_executable(pgfscaled)
target_link_libraries(pgfscaled
                      threadimageio # for PGFUtils
)

set(loadpgfdata_SRCS loadpgfdata.cpp ${pgfutils_SRCS})
add_executable(loadpgfdata ${loadpgfdata_SRCS})
ecm_mark_nongui_executable(loadpgfdata)
target_link_libraries(loadpgfdata
                      threadimageio # for PGFUtils
)

#------------------------------------------------------------------------

set(testdimgloader_SRCS testdimgloader.cpp)
add_executable(testdimgloader ${testdimgloader_SRCS})
ecm_mark_nongui_executable(testdimgloader)
target_link_libraries(testdimgloader
                      dimg
)

set(loadsavethreadtest_SRCS loadsavethreadtest.cpp)
add_executable(loadsavethreadtest ${loadsavethreadtest_SRCS})
ecm_mark_nongui_executable(loadsavethreadtest)
target_link_libraries(loadsavethreadtest
                      dimg
)

if(LENSFUN_FOUND)

    set(testlensfuniface_SRCS testlensfuniface.cpp)
    add_executable(testlensfuniface ${testlensfuniface_SRCS})
    ecm_mark_nongui_executable(testlensfuniface)
    target_link_libraries(testlensfuniface
                          dimg # TODO move lensuniface to libs..
    )

    if(NOT WIN32)
        # To link under Solaris (see bug #274484)
        target_link_libraries(testlensfuniface ${MATH_LIBRARY})
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
        target_link_libraries(testlensfuniface ${KVM_LIBRARY})
    endif()

endif()

#------------------------------------------------------------------------

set (testnrestimate_SRCS testnrestimate.cpp)
add_executable (testnrestimate ${testnrestimate_SRCS})
ecm_mark_nongui_executable(testnrestimate)
target_link_libraries(testnrestimate
                      dimg
)

#------------------------------------------------------------------------

set(testautocrop_SRCS testautocrop.cpp)
add_executable(testautocrop ${testautocrop_SRCS})
ecm_mark_nongui_executable(testautocrop)
target_link_libraries(testautocrop
                      dimg
)

#------------------------------------------------------------------------

set(testdatabase_SRCS testdatabase.cpp
)

add_executable(testdatabase ${testdatabase_SRCS})
target_link_libraries(testdatabase
                      digikamdatabase
)

if(ENABLE_DIGIKAM_MODELTEST)
    message(STATUS "Modeltest enabled")
    set(libdigikamgui_SRCS ${libdigikamgui_SRCS}
        modeltest/modeltest.cpp
       )
    add_definitions(-DENABLE_DIGIKAM_MODELTEST)
endif()

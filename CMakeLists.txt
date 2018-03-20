#
# Copyright (c) 2010-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

project(digikam-software-collection)

message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "Starting CMake configuration for: ${PROJECT_NAME}")

set(CMAKE_MIN_VERSION   "3.0.0")
set(ECM_MIN_VERSION     "1.5.0")

cmake_minimum_required(VERSION ${CMAKE_MIN_VERSION})

############## ECM setup #################################################################################################################

find_package(ECM ${ECM_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)
include(ECMOptionalAddSubdirectory)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
include(FeatureSummary)

# COMPILATION OPTIONS MANAGEMENT ###########################################################################################################

# doc and translations options

option(DIGIKAMSC_CHECKOUT_PO             "Checkout application translations files from git repositories (default=OFF)"            OFF)
option(DIGIKAMSC_CHECKOUT_DOC            "Checkout documentation translations files from git repositories (default=OFF)"          OFF)
option(DIGIKAMSC_COMPILE_PO              "Build translations files (default=ON)"                                                  ON)
option(DIGIKAMSC_COMPILE_DOC             "Build handbook files (default=ON)"                                                      ON)

# shared libs options for developers only

option(DIGIKAMSC_COMPILE_LIBKSANE        "Build local libksane library instead of using distributed files (default=OFF)"          OFF)
option(DIGIKAMSC_COMPILE_LIBMEDIAWIKI    "Build local libmediawiki library instead of using distributed files (default=OFF)"      OFF)
option(DIGIKAMSC_COMPILE_LIBKVKONTAKTE   "Build local libkvkontakte library instead of using distributed files (default=OFF)"     OFF)

option(DIGIKAMSC_COMPILE_DIGIKAM         "Build digiKam core (default=ON)"                                                        ON)

###########################################################################################################################################

if(DIGIKAMSC_CHECKOUT_PO OR DIGIKAMSC_CHECKOUT_DOC)

    find_package(Ruby)
    find_package(Subversion)

    if(RUBY_EXECUTABLE AND Subversion_FOUND)

        if(DIGIKAMSC_CHECKOUT_PO)

            message(STATUS "Extract application translation files from git repositories. Please wait, it can take a while...")

            if(DIGIKAMSC_COMPILE_DIGIKAM)
                set(CHECKOUT_PO_DIGIKAM "--enable-digikam")
            endif()

            execute_process(COMMAND ${RUBY_EXECUTABLE} "${CMAKE_SOURCE_DIR}/project/release/fetch_l10n_po.rb" ${CHECKOUT_PO_DIGIKAM} ${CHECKOUT_PO_LIBKVKONTAKTE}
                            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

            message(STATUS "Application translation files extraction done.")

        endif()

        if(DIGIKAMSC_CHECKOUT_DOC)

            message(STATUS "Extract documentation translation files from git repositories. Please wait, it can take a while...")
            message(STATUS "NOTE: (u) want mean uncomplete translations files, which will be removed from compilation.")

            execute_process(COMMAND ${RUBY_EXECUTABLE} "${CMAKE_SOURCE_DIR}/project/release/fetch_l10n_doc.rb"
                            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

            message(STATUS "Documentation translation files extraction done.")

        endif()

    else()

        message(STATUS "Could not find Ruby or Subversion to extract translation files")

    endif()

endif()

if(DIGIKAMSC_COMPILE_PO AND NOT EXISTS ${CMAKE_SOURCE_DIR}/po/)

    message(STATUS "Application translations directory is missing. Compilation of application translations is disabled...")

    if(NOT DIGIKAMSC_CHECKOUT_PO)
        message(STATUS "You can use DIGIKAMSC_CHECKOUT_PO option to extract GUI i18n files from git repositories.")
    endif()

    set(DIGIKAMSC_COMPILE_PO OFF)

endif()

if(DIGIKAMSC_COMPILE_DOC AND NOT EXISTS ${CMAKE_SOURCE_DIR}/doc/)

    message(STATUS "Main one documentation directory is missing. Compilation of documentation is disabled...")
    set(DIGIKAMSC_COMPILE_DOC OFF)

endif()

if(NOT EXISTS ${CMAKE_SOURCE_DIR}/doc-translated/)

    message(STATUS "Documentation translations directory is missing.")
    message(STATUS "You can use DIGIKAMSC_CHECKOUT_DOC option to extract doc i18n files from git repositories.")

endif()

if(DIGIKAMSC_COMPILE_PO OR DIGIKAMSC_COMPILE_DOC)
    set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS 1)
endif()

if(BUILD_TESTING)
    include(CTest)
endif()

############################################################################################################################################

message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "Compilation configuration for: ${PROJECT_NAME}")

if(DIGIKAMSC_COMPILE_DIGIKAM)
    if(NOT EXISTS ${CMAKE_SOURCE_DIR}/core)
        message(STATUS "digiKam core will be compiled.................. NO (source code not available)")
        set (DIGIKAMSC_COMPILE_DIGIKAM OFF)
    else()
        message(STATUS "digiKam core will be compiled.................. YES")
    endif()
else()
    message(STATUS "digiKam core will be compiled.................. NO")
endif()

if(DIGIKAMSC_COMPILE_LIBKSANE)
    if(NOT EXISTS ${CMAKE_SOURCE_DIR}/extra/libksane)
        message(STATUS "Local shared libksane will be compiled.......... NO (library source code not available)")
        set (DIGIKAMSC_COMPILE_LIBKSANE OFF)
    else()
        message(STATUS "Local shared libksane will be compiled......... YES")
    endif()
else()
    message(STATUS "Local shared libksane will be compiled......... NO")
endif()

if(DIGIKAMSC_COMPILE_LIBMEDIAWIKI)
    if(NOT EXISTS ${CMAKE_SOURCE_DIR}/extra/libmediawiki)
        message(STATUS "Local shared libmediawiki will be compiled.......... NO (library source code not available)")
        set (DIGIKAMSC_COMPILE_LIBMEDIAWIKI OFF)
    else()
        message(STATUS "Local shared libmediawiki will be compiled..... YES")
    endif()
else()
    message(STATUS "Local shared libmediawiki will be compiled..... NO")
endif()

if(DIGIKAMSC_COMPILE_LIBKVKONTAKTE)
    if(NOT EXISTS ${CMAKE_SOURCE_DIR}/extra/libkvkontakte)
        message(STATUS "Local shared libkvkontakte will be compiled.......... NO (library source code not available)")
        set (DIGIKAMSC_COMPILE_LIBKVKONTAKTE OFF)
    else()
        message(STATUS "Local shared libkvkontakte will be compiled.... YES")
    endif()
else()
    message(STATUS "Local shared libkvkontakte will be compiled.... NO")
endif()

if(DIGIKAMSC_CHECKOUT_PO)
    message(STATUS "Extract application translations files......... YES")
else()
    message(STATUS "Extract application translations files......... NO")
endif()

if(DIGIKAMSC_CHECKOUT_DOC)
    message(STATUS "Extract documentation translations files....... YES")
else()
    message(STATUS "Extract documentation translations files....... NO")
endif()

if(DIGIKAMSC_COMPILE_PO)
    message(STATUS "Translations will be compiled.................. YES")
else()
    message(STATUS "Translations will be compiled.................. NO")
endif()

if(DIGIKAMSC_COMPILE_DOC)
    message(STATUS "Handbooks will be compiled..................... YES")
else()
    message(STATUS "Handbooks will be compiled..................... NO")
endif()

if(BUILD_TESTING)
    message(STATUS "Tests code will be compiled.................... YES")
else()
    message(STATUS "Tests code will be compiled.................... NO")
endif()

#############################################################################################

add_subdirectory(extra)

if(DIGIKAMSC_COMPILE_DIGIKAM)
    add_subdirectory(core)
endif()

if(DIGIKAMSC_COMPILE_PO OR DIGIKAMSC_COMPILE_DOC)
    find_package(Gettext REQUIRED)
endif()

if(DIGIKAMSC_COMPILE_PO)
    add_subdirectory(po)
endif()

if(DIGIKAMSC_COMPILE_DOC)
    find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS DocTools)
    ECM_OPTIONAL_ADD_SUBDIRECTORY(doc)
    ECM_OPTIONAL_ADD_SUBDIRECTORY(doc-translated)
endif()

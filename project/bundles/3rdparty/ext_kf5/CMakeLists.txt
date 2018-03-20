# Script to build KF5 Framework for digiKam bundle.
#
# Copyright (c) 2015-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

set(EXTPREFIX_frameworks "${EXTPREFIX}")

function(InstallKF5Component
         name
         version
         argoptions
         patch)

    if(MINGW)

        # Extract MD5 sums of each KF5 components directly from server.
        execute_process(COMMAND
                        wget -q -O - -o /dev/null http://download.kde.org/stable/frameworks/${version}/${name}-${version}.0.tar.xz.mirrorlist
                        COMMAND
                        grep "MD5 Hash"
                        COMMAND
                        grep -o -P "(?<=<tt>).*(?=</tt>)"
                        RESULT_VARIABLE
                        wgetresult
                        OUTPUT_VARIABLE
                        md5sm
                        OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if(NOT ${wgetresult} MATCHES 0)
            message(FATAL_ERROR "Cannot donwload MD5 sum for ${name}!")
        endif()

        message(STATUS "--------------------------------")
        message(STATUS "KF5 component    : ${name}")
        message(STATUS "Component version: ${version}")
        message(STATUS "Component MD5    : ${md5sm}")
        message(STATUS "Configure options: ${argoptions}")
        message(STATUS "Patch to apply   : ${patch}")

        if(NOT "${patch}" STREQUAL "")
            set(patch "${PATCH_COMMAND};-p1;-i;${CMAKE_CURRENT_SOURCE_DIR}/${patch}")
        endif()

        ExternalProject_Add(ext_${name}
                            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
                            URL file://${EXTERNALS_DOWNLOAD_DIR}/${name}-${version}.0.tar.xz
                            URL_MD5 ${md5sm}
                            INSTALL_DIR ${EXTPREFIX_frameworks}

                            CMAKE_ARGS -DMXE_TOOLCHAIN=${MXE_TOOLCHAIN}
                                       -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                                       -DCMAKE_COLOR_MAKEFILE=ON
                                       -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
                                       -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON
                                       -DCMAKE_TOOLCHAIN_FILE=${MXE_TOOLCHAIN}
                                       -DCMAKE_FIND_PREFIX_PATH=${CMAKE_PREFIX_PATH}
                                       -DCMAKE_SYSTEM_INCLUDE_PATH=${CMAKE_PREFIX_PATH}/include
                                       -DCMAKE_INCLUDE_PATH=${CMAKE_PREFIX_PATH}/include
                                       -DCMAKE_LIBRARY_PATH=${CMAKE_PREFIX_PATH}/lib
                                       -DZLIB_ROOT=${CMAKE_PREFIX_PATH}
                                       -DINSTALL_ROOT=${MXE_INSTALL_PREFIX}
                                       ${GLOBAL_PROFILE}
                                       "${argoptions}"
                                       -Wno-dev

                            PATCH_COMMAND ${patch}
                            UPDATE_COMMAND ""
                            ALWAYS 0
        )

        ExternalProject_Add_step(ext_${name} before_download

            # Note : under MXE, curl is not compiled with https support. We cannot use the tarball as under Linux/MacOS.
            COMMAND wget -N http://download.kde.org/stable/frameworks/${version}/${name}-${version}.0.tar.xz -O ${EXTERNALS_DOWNLOAD_DIR}/${name}-${version}.0.tar.xz
            DEPENDERS download
        )

    elseif(APPLE)

        # Extract MD5 sums of each KF5 components directly from server.
        execute_process(COMMAND
                            wget -q -O - -o /dev/null http://download.kde.org/stable/frameworks/${version}/${name}-${version}.0.tar.xz.mirrorlist
                        COMMAND
                            grep "MD5 Hash"
                        COMMAND
                            grep -o "<tt>.*</tt>"
                        COMMAND
                            cut -c 5-
                        COMMAND
                            rev
                        COMMAND
                            cut -c 6-
                        COMMAND
                            rev
                        RESULT_VARIABLE
                            wgetresult
                        OUTPUT_VARIABLE
                            md5sm
                        OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if(NOT ${wgetresult} MATCHES 0)
            message(FATAL_ERROR "Cannot donwload MD5 sum for ${name}!")
        endif()

        message(STATUS "--------------------------------")
        message(STATUS "KF5 component    : ${name}")
        message(STATUS "Component version: ${version}")
        message(STATUS "Component MD5    : ${md5sm}")
        message(STATUS "Configure options: ${argoptions}")
        message(STATUS "Patch to apply   : ${patch}")

        if(NOT "${patch}" STREQUAL "")
            set(patch "${PATCH_COMMAND};-p1;-i;${CMAKE_CURRENT_SOURCE_DIR}/${patch};&&")
        endif()

        ExternalProject_Add(ext_${name}
                            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
                            URL http://download.kde.org/stable/frameworks/${version}/${name}-${version}.0.tar.xz
                            URL_MD5 ${md5sm}

                            PATCH_COMMAND ${patch}
                                          cp ${CMAKE_CURRENT_SOURCE_DIR}/../../macports/fixbundledatapath.sh <SOURCE_DIR>/. &&
                                          <SOURCE_DIR>/fixbundledatapath.sh

                            CONFIGURE_COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bootstrap.macports <SOURCE_DIR>/. &&
                                              <SOURCE_DIR>/bootstrap.macports ${EXTPREFIX_frameworks} debug x86_64 "${argoptions}"

                            BUILD_COMMAND cd <SOURCE_DIR>/build && $(MAKE)

                            INSTALL_COMMAND cd <SOURCE_DIR>/build && make install/fast

                            BUILD_IN_SOURCE 1

                            UPDATE_COMMAND ""
                            ALWAYS 0
        )

    else() # Linux

        # Extract MD5 sums of each KF5 components directly from server.
        execute_process(COMMAND
                        wget -q -O - -o /dev/null http://download.kde.org/stable/frameworks/${version}/${name}-${version}.0.tar.xz.mirrorlist
                        COMMAND
                        grep "MD5 Hash"
                        COMMAND
                        grep -o -P "(?<=<tt>).*(?=</tt>)"
                        RESULT_VARIABLE
                        wgetresult
                        OUTPUT_VARIABLE
                        md5sm
                        OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if(NOT ${wgetresult} MATCHES 0)
            message(FATAL_ERROR "Cannot donwload MD5 sum for ${name}!")
        endif()

        message(STATUS "--------------------------------")
        message(STATUS "KF5 component    : ${name}")
        message(STATUS "Component version: ${version}")
        message(STATUS "Component MD5    : ${md5sm}")
        message(STATUS "Configure options: ${argoptions}")
        message(STATUS "Patch to apply   : ${patch}")

        if(NOT "${patch}" STREQUAL "")
            set(patch "${PATCH_COMMAND};-p1;-i;${CMAKE_CURRENT_SOURCE_DIR}/${patch}")
        endif()

        ExternalProject_Add(ext_${name}
                            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
                            URL http://download.kde.org/stable/frameworks/${version}/${name}-${version}.0.tar.xz
                            URL_MD5 ${md5sm}
                            INSTALL_DIR ${EXTPREFIX_frameworks}
                            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
                                       -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                                       ${GLOBAL_PROFILE}
                                       -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                                       -DBUILD_TESTING=OFF
                                       "${argoptions}"
                                       -Wno-dev
                            PATCH_COMMAND ${patch}
                            UPDATE_COMMAND ""
                            ALWAYS 0
        )

    endif()

endfunction()

set(KD_VERSION 5.42)

if(APPLE)
    set(MacosThreadWeaverPatch threadweaver-macports.patch)
    set(MacosSolidPatch solid-iokitsupport-macports.patch)   # See https://phabricator.kde.org/D7401
    set(MacosKauthBackend -DKAUTH_BACKEND_NAME=Apple)
endif()

InstallKF5Component(extra-cmake-modules ${KD_VERSION} ""                                                                   "")
InstallKF5Component(kconfig             ${KD_VERSION} ""                                                                   "")
InstallKF5Component(breeze-icons        ${KD_VERSION} "-DBINARY_ICONS_RESOURCE=OFF"                                        "${MacosBreezePatch}")
InstallKF5Component(solid               ${KD_VERSION} ""                                                                   "${MacosSolidPatch}")
InstallKF5Component(kcoreaddons         ${KD_VERSION} ""                                                                   "")
InstallKF5Component(threadweaver        ${KD_VERSION} ""                                                                   "${MacosThreadWeaverPatch}")
InstallKF5Component(kwindowsystem       ${KD_VERSION} ""                                                                   "")
InstallKF5Component(karchive            ${KD_VERSION} ""                                                                   "")
InstallKF5Component(kdbusaddons         ${KD_VERSION} ""                                                                   "")
InstallKF5Component(ki18n               ${KD_VERSION} ""                                                                   "")
InstallKF5Component(kcrash              ${KD_VERSION} ""                                                                   "")
InstallKF5Component(kcodecs             ${KD_VERSION} ""                                                                   "")
InstallKF5Component(kauth               ${KD_VERSION} "${MacosKauthBackend}"                                               "")
InstallKF5Component(kguiaddons          ${KD_VERSION} ""                                                                   "")
InstallKF5Component(kwidgetsaddons      ${KD_VERSION} ""                                                                   "")
InstallKF5Component(kitemviews          ${KD_VERSION} ""                                                                   "")
InstallKF5Component(kcompletion         ${KD_VERSION} ""                                                                   "")
InstallKF5Component(kconfigwidgets      ${KD_VERSION} ""                                                                   "")
InstallKF5Component(kiconthemes         ${KD_VERSION} ""                                                                   "")
InstallKF5Component(kservice            ${KD_VERSION} ""                                                                   "")
InstallKF5Component(kglobalaccel        ${KD_VERSION} ""                                                                   "")
InstallKF5Component(kxmlgui             ${KD_VERSION} ""                                                                   "kxmlgui-drop-ktextwidgets.patch")
InstallKF5Component(kbookmarks          ${KD_VERSION} ""                                                                   "")
InstallKF5Component(kimageformats       ${KD_VERSION} ""                                                                   "")
InstallKF5Component(knotifyconfig       ${KD_VERSION} ""                                                                   "")
InstallKF5Component(knotifications      ${KD_VERSION} ""                                                                   "knotifications-drop-phonon.patch")
InstallKF5Component(kjobwidgets         ${KD_VERSION} ""                                                                   "")
InstallKF5Component(sonnet              ${KD_VERSION} ""                                                                   "")
InstallKF5Component(ktextwidgets        ${KD_VERSION} ""                                                                   "")
InstallKF5Component(kio                 ${KD_VERSION} ""                                                                   "kio-drop-ktextwidgets.patch")
InstallKF5Component(kinit               ${KD_VERSION} ""                                                                   "")
InstallKF5Component(kdewebkit           ${KD_VERSION} ""                                                                   "")
InstallKF5Component(kwallet             ${KD_VERSION} ""                                                                   "kwallet-minimal.patch")

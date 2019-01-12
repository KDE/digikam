# Some useful macros for printing status information
#
# Copyright (c) 2010-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(PRINT_COMPILE_LENGTH "40")

macro(FILL_WITH_DOTS VAR)

    string(LENGTH ${${VAR}} NAME_LENGTH)

    math(EXPR DOT_LENGTH "${PRINT_COMPILE_LENGTH} - ${NAME_LENGTH}")

    if(${DOT_LENGTH} LESS 0)

        set(DOT_LENGTH 0)

    endif()

    foreach(COUNT RANGE ${DOT_LENGTH})

        set(${VAR} "${${VAR}}.")

    endforeach()

endmacro()

# -------------------------------------------------------------------------

macro(PRINT_LIBRARY_STATUS NAME WEBSITE VERSIONHINT)

    set(LIB_MESSAGE "${NAME} found")
    FILL_WITH_DOTS(LIB_MESSAGE)

    if(${ARGN})

        message(STATUS " ${LIB_MESSAGE} YES")

    else()

        message(STATUS " ${LIB_MESSAGE} NO")
        message(SEND_ERROR " ${NAME} is needed. You need to install the ${NAME} ${VERSIONHINT} development package.")
        message(STATUS " ${NAME} website is at ${WEBSITE}")
        message(STATUS "")

    endif()

endmacro()

# -------------------------------------------------------------------------

macro(PRINT_OPTIONAL_LIBRARY_STATUS NAME WEBSITE VERSIONHINT FEATUREMISSING)

    set(LIB_MESSAGE "${NAME} found")
    FILL_WITH_DOTS(LIB_MESSAGE)

    if(${ARGN})

        message(STATUS " ${LIB_MESSAGE} YES (optional)")

    else()

        message(STATUS " ${LIB_MESSAGE} NO  (optional)")
        message(STATUS " ${FEATUREMISSING}")
        message(STATUS " Please install the ${NAME} ${VERSIONHINT} development package.")
        if(${WEBSITE})
            message(STATUS " ${NAME} website is at ${WEBSITE}")
        endif()
        message(STATUS "")

    endif()

endmacro()

# -------------------------------------------------------------------------

macro(PRINT_QTMODULE_STATUS NAME)

    set(LIB_MESSAGE "${NAME} module found")
    FILL_WITH_DOTS(LIB_MESSAGE)

    if(${ARGN})

        message(STATUS " ${LIB_MESSAGE} YES")

    else()

        message(STATUS " ${LIB_MESSAGE} NO")
        message(STATUS "")
        message(SEND_ERROR " ${NAME} module is needed. You need to install a package containing the ${NAME} module.")
        message(STATUS "")

    endif()

endmacro()

# -------------------------------------------------------------------------

macro(PRINT_EXECUTABLE_STATUS NAME TECHNICAL_NAME PATH_VARIABLE)

    set(LIB_MESSAGE "${NAME} found")
    FILL_WITH_DOTS(LIB_MESSAGE)

    if(${ARGN})

        message(STATUS " ${LIB_MESSAGE} YES")

    else()

        message(STATUS " ${LIB_MESSAGE} NO")
        message(STATUS "")
        message(STATUS " ${NAME} is needed. You need to install the package containing the \"${TECHNICAL_NAME}\" executable.")
        message(STATUS " If you have this executable installed, please specify the folder containing it by ${PATH_VARIABLE}")
        message(SEND_ERROR "")

    endif()

endmacro()

# -------------------------------------------------------------------------

macro(PRINT_COMPONENT_COMPILE_STATUS NAME)

    set(COMPILE_MESSAGE "${NAME} will be compiled")
    FILL_WITH_DOTS(COMPILE_MESSAGE)

    IF(${ARGN})

        message(STATUS " ${COMPILE_MESSAGE} YES (optional)")

    else()

        message(STATUS " ${COMPILE_MESSAGE} NO  (optional)")

    endif()

endmacro()

# -------------------------------------------------------------------------

macro(PRINT_OPTIONAL_QTMODULE_STATUS NAME)

    set(LIB_MESSAGE "${NAME} module found")
    FILL_WITH_DOTS(LIB_MESSAGE)

    if(${ARGN})

        message(STATUS " ${LIB_MESSAGE} YES (optional)")

    else()

        message(STATUS " ${LIB_MESSAGE} NO  (optional)")

    endif()

endmacro()

# -------------------------------------------------------------------------

macro(HEADER_DIRECTORIES return_list)

    file(GLOB_RECURSE new_list *.h)
    set(dir_list "")

    foreach(file_path ${new_list})

        get_filename_component(dir_path ${file_path} PATH)
        set(dir_list ${dir_list} ${dir_path})

    endforeach()

    list(REMOVE_DUPLICATES dir_list)
    set(${return_list} ${dir_list})

endmacro()

# -------------------------------------------------------------------------

macro(DIGIKAM_ADD_GENERIC_PLUGIN _target_name)

    set(_src ${ARGN})

    add_library(Generic_${_target_name}_Plugin MODULE ${_src})

    target_link_libraries(Generic_${_target_name}_Plugin ${GENE${_target_name}_LIBS} digikamcore)

    install(TARGETS Generic_${_target_name}_Plugin
            DESTINATION ${PLUGIN_INSTALL_DIR}/digikam/generic)

endmacro()

# -------------------------------------------------------------------------

macro(DIGIKAM_ADD_EDITOR_PLUGIN _target_name)

    set(_src ${ARGN})

    add_library(Editor_${_target_name}_Plugin MODULE ${_src})

    target_link_libraries(Editor_${_target_name}_Plugin ${${_target_name}_LIBS} digikamcore)

    install(TARGETS Editor_${_target_name}_Plugin
            DESTINATION ${PLUGIN_INSTALL_DIR}/digikam/editor)

endmacro()

# -------------------------------------------------------------------------

macro(DIGIKAM_ADD_BQM_PLUGIN _target_name)

    set(_src ${ARGN})

    add_library(Bqm_${_target_name}_Plugin MODULE ${_src})

    target_link_libraries(Bqm_${_target_name}_Plugin ${${_target_name}_LIBS} digikamgui)

    install(TARGETS Bqm_${_target_name}_Plugin
            DESTINATION ${PLUGIN_INSTALL_DIR}/digikam/bqm)

endmacro()

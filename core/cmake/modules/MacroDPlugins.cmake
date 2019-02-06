# Some useful macros for printing status information
#
# Copyright (c) 2010-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(CMakeParseArguments)

# -------------------------------------------------------------------------

# This macro implement the rules to compile and link a Generic DPlugin with extra arguments.
#
# Usage: DIGIKAM_ADD_GENERIC_PLUGIN(NAME    _plugin_name_
#                                   SOURCE  _plugin_sources_
#                                   DEPENDS _plugin_dependencies_)
#
# With: _plugin_name_ the literal name of the plugin (mandatory).
#       _plugin_sources_ the list of source codes to compile (mandatory).
#       _plugin_dependencies_ the list of dependencies to link (optional).
#
# Note: by default a Generic plugin is linked with digikamcore shared library.
#
# This macro will generate a plugin library with this pattern as file name:
# Generic_${_pluginname_}_Plugin
#
macro(DIGIKAM_ADD_GENERIC_PLUGIN)

    set(_OPTIONS_ARGS)
    set(_ONE_VALUE_ARGS)
    set(_MULTI_VALUE_ARGS NAME SOURCES DEPENDS)

    cmake_parse_arguments(_parse_results "${_OPTIONS_ARGS}"
                                         "${_ONE_VALUE_ARGS}"
                                         "${_MULTI_VALUE_ARGS}"
                                         ${ARGN}
    )

    # Mandatory
    if(_parse_results_NAME)
#        message(STATUS "Generic plugin name=${_parse_results_NAME}")
    else()
        message(FATAL_ERROR "Generic plugin name is required.")
    endif()

    if(_parse_results_SOURCES )
#        message(STATUS "Generic plugin sources=${_parse_results_SOURCES}")
    else()
        message(FATAL_ERROR "Generic plugin sources is required.")
    endif()

    # Optional
    if(_parse_results_DEPENDS)
#        message(STATUS "Generic plugin dependencies=${_parse_results_DEPENDS}")
    endif()

    add_library(Generic_${_parse_results_NAME}_Plugin
                MODULE ${_parse_results_SOURCES})

    target_link_libraries(Generic_${_parse_results_NAME}_Plugin
                          ${${_parse_results_NAME}_LIBS}
                          digikamcore
                          ${_parse_results_DEPENDS}
    )

    install(TARGETS Generic_${_parse_results_NAME}_Plugin
            DESTINATION ${PLUGIN_INSTALL_DIR}/digikam/generic
    )

endmacro()

# -------------------------------------------------------------------------

# This macro implement the rules to compile and link an Editor DPlugin with extra arguments.
#
# Usage: DIGIKAM_ADD_EDITOR_PLUGIN(NAME    _plugin_name_
#                                  SOURCE  _plugin_sources_
#                                  DEPENDS _plugin_dependencies_)
#
# With: _plugin_name_ the literal name of the plugin (mandatory).
#       _plugin_sources_ the list of source codes to compile (mandatory).
#       _plugin_dependencies_ the list of dependencies to link (optional).
#
# Note: by default an Editor plugin is linked with digikamcore shared library.
#
# This macro will generate a plugin library with this pattern as file name:
# Bqm_${_pluginname_}_Plugin
#
macro(DIGIKAM_ADD_EDITOR_PLUGIN)

    set(_OPTIONS_ARGS)
    set(_ONE_VALUE_ARGS)
    set(_MULTI_VALUE_ARGS NAME SOURCES DEPENDS)

    cmake_parse_arguments(_parse_results "${_OPTIONS_ARGS}"
                                         "${_ONE_VALUE_ARGS}"
                                         "${_MULTI_VALUE_ARGS}"
                                         ${ARGN}
    )

    # Mandatory
    if(_parse_results_NAME)
#        message(STATUS "Editor plugin name=${_parse_results_NAME}")
    else()
        message(FATAL_ERROR "Editor plugin name is required.")
    endif()

    if(_parse_results_SOURCES )
#        message(STATUS "Editor plugin sources=${_parse_results_SOURCES}")
    else()
        message(FATAL_ERROR "Editor plugin sources is required.")
    endif()

    # Optional
    if(_parse_results_DEPENDS)
#        message(STATUS "Editor plugin dependencies=${_parse_results_DEPENDS}")
    endif()

    add_library(Editor_${_parse_results_NAME}_Plugin
                MODULE ${_parse_results_SOURCES})

    target_link_libraries(Editor_${_parse_results_NAME}_Plugin
                          ${${_parse_results_NAME}_LIBS}
                          digikamcore
                          ${_parse_results_DEPENDS}
    )

    install(TARGETS Editor_${_parse_results_NAME}_Plugin
            DESTINATION ${PLUGIN_INSTALL_DIR}/digikam/editor
    )

endmacro()

# -------------------------------------------------------------------------

# This macro implement the rules to compile and link an Bqm DPlugin with extra arguments.
#
# Usage: DIGIKAM_ADD_BQM_PLUGIN(NAME    _plugin_name_
#                               SOURCE  _plugin_sources_
#                               DEPENDS _plugin_dependencies_)
#
# With: _plugin_name_ the literal name of the plugin (mandatory).
#       _plugin_sources_ the list of source codes to compile (mandatory).
#       _plugin_dependencies_ the list of dependencies to link (optional).
#
# Note: by default a Bqm plugin is linked with digikamcore shared library.
#
# This macro will generate a plugin library with this pattern as file name:
# Editor_${_pluginname_}_Plugin
#
macro(DIGIKAM_ADD_BQM_PLUGIN)

    set(_OPTIONS_ARGS)
    set(_ONE_VALUE_ARGS)
    set(_MULTI_VALUE_ARGS NAME SOURCES DEPENDS)

    cmake_parse_arguments(_parse_results "${_OPTIONS_ARGS}"
                                         "${_ONE_VALUE_ARGS}"
                                         "${_MULTI_VALUE_ARGS}"
                                         ${ARGN}
    )

    # Mandatory
    if(_parse_results_NAME)
#        message(STATUS "Bqm plugin name=${_parse_results_NAME}")
    else()
        message(FATAL_ERROR "Bqm plugin name is required.")
    endif()

    if(_parse_results_SOURCES )
#        message(STATUS "Bqm plugin sources=${_parse_results_SOURCES}")
    else()
        message(FATAL_ERROR "Bqm plugin sources is required.")
    endif()

    # Optional
    if(_parse_results_DEPENDS)
#        message(STATUS "Bqm plugin dependencies=${_parse_results_DEPENDS}")
    endif()

    add_library(Bqm_${_parse_results_NAME}_Plugin
                MODULE ${_parse_results_SOURCES})

    target_link_libraries(Bqm_${_parse_results_NAME}_Plugin
                          ${${_parse_results_NAME}_LIBS}
                          digikamgui
                          ${_parse_results_DEPENDS}
    )

    install(TARGETS Bqm_${_parse_results_NAME}_Plugin
            DESTINATION ${PLUGIN_INSTALL_DIR}/digikam/bqm
    )

endmacro()

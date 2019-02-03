# Some useful macros for printing status information
#
# Copyright (c) 2010-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(CMakeParseArguments)

# -------------------------------------------------------------------------

# This macro implement the rules to compile and link a Generic DPlugin with extar arguments.
#
# Usage: DIGIKAM_ADD_GENERIC_PLUGIN_ADVANCED(NAME    _plugin_name_
#                                            SOURCE  _plugin_sources_
#                                            DEPENDS _plugin_dependencies_)
#
# With: _plugin_name_ the literal name of the plugin (mandatory).
#       _plugin_sources_ the list of source codes to compile (mandatory).
#       _plugin_dependencies_ the list of dependencies to link (optional).
#
macro(DIGIKAM_ADD_GENERIC_PLUGIN_ADVANCED)

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
#        message(STATUS "Generic plugin sources= ${_parse_results_SOURCES}")
    else()
        message(FATAL_ERROR "Generic plugin sources is required.")
    endif()

    # Optional
    if(_parse_results_DEPENDS)
#        message(STATUS "Generic plugin dependencies= ${_parse_results_DEPENDS}")
    endif()

    add_library(Generic_${_parse_results_NAME}_Plugin
                MODULE ${_parse_results_SOURCES})

    target_link_libraries(Generic_${_parse_results_NAME}_Plugin
                          ${GENE${_parse_results_NAME}_LIBS}
                          digikamcore
                          ${_parse_results_DEPENDS}
    )

    install(TARGETS Generic_${_parse_results_NAME}_Plugin
            DESTINATION ${PLUGIN_INSTALL_DIR}/digikam/generic
    )

endmacro()

# -------------------------------------------------------------------------


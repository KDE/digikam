# Some useful macros to manage compiler rules
#
# Copyright (c) 2010-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# -------------------------------------------------------------------------

# Macro to remove GCC compilation option.

macro(REMOVE_GCC_COMPILER_WARNINGS COMPILATION_OPTION)

    if(CMAKE_COMPILER_IS_GNUCXX)

        message(STATUS "Remove GCC compiler option ${COMPILATION_OPTION} from ${CMAKE_CURRENT_SOURCE_DIR}")

        while(CMAKE_CXX_FLAGS MATCHES ${COMPILATION_OPTION})

            string(REPLACE ${COMPILATION_OPTION} "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

        endwhile()

    endif()

endmacro()

# -------------------------------------------------------------------------

# Macro to remove CLANG compilation option.

macro(REMOVE_CLANG_COMPILER_WARNINGS COMPILATION_OPTION)

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")

        message(STATUS "Remove CLANG compiler option ${COMPILATION_OPTION} from ${CMAKE_CURRENT_SOURCE_DIR}")

        while(CMAKE_CXX_FLAGS MATCHES ${COMPILATION_OPTION})

            string(REPLACE ${COMPILATION_OPTION} "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

        endwhile()

    endif()

endmacro()

# -------------------------------------------------------------------------

# Macro to remove GCC or CLANG compilation option which use the same syntax.

macro(REMOVE_COMPILER_WARNINGS COMPILATION_OPTION)

    REMOVE_GCC_COMPILER_WARNINGS(${COMPILATION_OPTION})
    REMOVE_CLANG_COMPILER_WARNINGS(${COMPILATION_OPTION})

endmacro()

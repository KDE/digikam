# Some useful macros to manage compiler rules
#
# For a complete list of GCC and Clang compiler warnings options available
# depending of compiler version, check this url:
# https://github.com/Barro/compiler-warnings
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

# -------------------------------------------------------------------------

# Macro to disable GCC compilation option using -Wno* syntax.

macro(DISABLE_GCC_COMPILER_WARNINGS COMPILER_VERSION NO_COMPILATION_OPTION)

    if(CMAKE_COMPILER_IS_GNUCXX)

        exec_program(${CMAKE_CXX_COMPILER} ARGS ${CMAKE_CXX_COMPILER_ARG1} -dumpversion OUTPUT_VARIABLE GCC_VERSION)

        if (${GCC_VERSION} VERSION_GREATER ${COMPILER_VERSION})

            message(STATUS "Disable GCC compiler option ${NO_COMPILATION_OPTION} from ${CMAKE_CURRENT_SOURCE_DIR}")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NO_COMPILATION_OPTION}")

        endif()

    endif()

endmacro()

# -------------------------------------------------------------------------

# Macro to disable CLANG compilation option using -Wno* syntax.

macro(DISABLE_CLANG_COMPILER_WARNINGS COMPILER_VERSION NO_COMPILATION_OPTION)

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")

        exec_program(${CMAKE_CXX_COMPILER} ARGS ${CMAKE_CXX_COMPILER_ARG1} -dumpversion OUTPUT_VARIABLE CLANG_VERSION)

        if (${CLANG_VERSION} VERSION_GREATER ${COMPILER_VERSION})

            message(STATUS "Disable CLANG compiler option ${NO_COMPILATION_OPTION} from ${CMAKE_CURRENT_SOURCE_DIR}")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NO_COMPILATION_OPTION}")

        endif()

    endif()

endmacro()

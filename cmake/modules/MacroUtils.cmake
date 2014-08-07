# Some useful macros for printing status information
#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier.gilles@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

SET(PRINT_COMPILE_LENGTH "40")

MACRO(FILL_WITH_DOTS VAR)

    STRING(LENGTH ${${VAR}} NAME_LENGTH)

    MATH(EXPR DOT_LENGTH "${PRINT_COMPILE_LENGTH} - ${NAME_LENGTH}")

    IF(${DOT_LENGTH} LESS 0)
    
        SET(DOT_LENGTH 0)
    
    ENDIF()

    FOREACH(COUNT RANGE ${DOT_LENGTH})

        SET(${VAR} "${${VAR}}.")

    ENDFOREACH(COUNT)

ENDMACRO(FILL_WITH_DOTS)

# -------------------------------------------------------------------------

MACRO(PRINT_LIBRARY_STATUS NAME WEBSITE VERSIONHINT)

    SET(LIB_MESSAGE "${NAME} library found")
    FILL_WITH_DOTS(LIB_MESSAGE)

    IF(${ARGN})

        MESSAGE(STATUS " ${LIB_MESSAGE} YES")

    ELSE()

        MESSAGE(STATUS " ${LIB_MESSAGE} NO")
        MESSAGE(STATUS "")
        MESSAGE(SEND_ERROR " ${NAME} is needs. You need to install the ${NAME}${VERSIONHINT} library development package.")
        MESSAGE(STATUS " ${NAME} website is at ${WEBSITE}")
        MESSAGE(STATUS "")

    ENDIF()

ENDMACRO(PRINT_LIBRARY_STATUS)

# -------------------------------------------------------------------------

MACRO(PRINT_QTMODULE_STATUS NAME)

    SET(LIB_MESSAGE "${NAME} module found")
    FILL_WITH_DOTS(LIB_MESSAGE)

    IF(${ARGN})
    
        MESSAGE(STATUS " ${LIB_MESSAGE} YES")
    
    ELSE()
    
        MESSAGE(STATUS " ${LIB_MESSAGE} NO")
        MESSAGE(STATUS "")
        MESSAGE(SEND_ERROR " ${NAME} module is needs. You need to install a package containing the ${NAME} module.")
        MESSAGE(STATUS "")
    
    ENDIF()

ENDMACRO(PRINT_QTMODULE_STATUS)

# -------------------------------------------------------------------------

MACRO(PRINT_EXECUTABLE_STATUS NAME TECHNICAL_NAME PATH_VARIABLE)

    SET(LIB_MESSAGE "${NAME} found")
    FILL_WITH_DOTS(LIB_MESSAGE)

    IF(${ARGN})
    
        MESSAGE(STATUS " ${LIB_MESSAGE} YES")
    
    ELSE()
    
        MESSAGE(STATUS " ${LIB_MESSAGE} NO")
        MESSAGE(STATUS "")
        MESSAGE(STATUS " ${NAME} is needs. You need to install the package containing the \"${TECHNICAL_NAME}\" executable.")
        MESSAGE(STATUS " If you have this executable installed, please specify the folder containing it by ${PATH_VARIABLE}")
        MESSAGE(SEND_ERROR "")
    
    ENDIF()

ENDMACRO(PRINT_EXECUTABLE_STATUS)

# -------------------------------------------------------------------------

MACRO(PRINT_COMPONENT_COMPILE_STATUS NAME)

    SET(COMPILE_MESSAGE "${NAME} will be compiled")
    FILL_WITH_DOTS(COMPILE_MESSAGE)

    IF(${ARGN})
    
        MESSAGE(STATUS " ${COMPILE_MESSAGE} YES (optional)")
    
    ELSE()
    
        MESSAGE(STATUS " ${COMPILE_MESSAGE} NO  (optional - Look README file for more details about dependencies)")
    
    ENDIF()

ENDMACRO(PRINT_PLUGIN_COMPILE_STATUS)

# -------------------------------------------------------------------------

MACRO(PRINT_OPTIONAL_LIBRARY_STATUS NAME)

    SET(LIB_MESSAGE "${NAME} library found")
    FILL_WITH_DOTS(LIB_MESSAGE)

    IF(${ARGN})

        MESSAGE(STATUS " ${LIB_MESSAGE} YES (optional)")

    ELSE()
    
        MESSAGE(STATUS " ${LIB_MESSAGE} NO  (optional)")
    
    ENDIF()

ENDMACRO(PRINT_OPTIONAL_LIBRARY_STATUS)

# -------------------------------------------------------------------------

MACRO(PRINT_OPTIONAL_QTMODULE_STATUS NAME)

    SET(LIB_MESSAGE "${NAME} module found")
    FILL_WITH_DOTS(LIB_MESSAGE)

    IF(${ARGN})

        MESSAGE(STATUS " ${LIB_MESSAGE} YES (optional)")
    
    ELSE()
    
        MESSAGE(STATUS " ${LIB_MESSAGE} NO  (optional)")
    
    ENDIF()

ENDMACRO(PRINT_OPTIONAL_QTMODULE_STATUS)

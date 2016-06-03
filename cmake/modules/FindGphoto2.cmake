# Cmake macro to detect gphoto2 libraries
#
#  GPHOTO2_FOUND - system has the GPHOTO2 library
#  GPHOTO2_INCLUDE_DIR - the GPHOTO2 include directory
#  GPHOTO2_LIBRARIES - The libraries needed to use GPHOTO2
#
# Copyright (c) 2006, 2007 Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(GPHOTO2_LIBRARIES AND GPHOTO2_INCLUDE_DIR)

    # in cache already
    set(GPHOTO2_FOUND TRUE)

else()

    find_program(GPHOTO2CONFIG_EXECUTABLE     NAMES gphoto2-config)
    find_program(GPHOTO2PORTCONFIG_EXECUTABLE NAMES gphoto2-port-config)

    set(GPHOTO2_LIBRARIES)
    set(GPHOTO2_INCLUDE_DIRS)

    # if gphoto2-port-config and gphoto2-config have been found

    if(GPHOTO2PORTCONFIG_EXECUTABLE AND GPHOTO2CONFIG_EXECUTABLE)

        message(STATUS "libgphoto2 execurables check...")
        exec_program(${GPHOTO2PORTCONFIG_EXECUTABLE} ARGS --libs   RETURN_VALUE _return_VALUE OUTPUT_VARIABLE GPHOTO2PORT_LIBRARY)
        exec_program(${GPHOTO2CONFIG_EXECUTABLE}     ARGS --libs   RETURN_VALUE _return_VALUE OUTPUT_VARIABLE GPHOTO2_LIBRARY)
        exec_program(${GPHOTO2PORTCONFIG_EXECUTABLE} ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _GPHOTO2PORT_RESULT_INCLUDE_DIR)
        exec_program(${GPHOTO2CONFIG_EXECUTABLE}     ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _GPHOTO2_RESULT_INCLUDE_DIR)

        set(GPHOTO2_LIBRARIES ${GPHOTO2PORT_LIBRARY} ${GPHOTO2_LIBRARY})

        # the cflags can contain more than one include path
        separate_arguments(_GPHOTO2_RESULT_INCLUDE_DIR)

        foreach(_includedir ${_GPHOTO2_RESULT_INCLUDE_DIR})

            string(REGEX REPLACE "-I(.+)" "\\1" _includedir "${_includedir}")
            set(GPHOTO2_INCLUDE_DIR ${GPHOTO2_INCLUDE_DIR} ${_includedir})

        endforeach()

        separate_arguments(_GPHOTO2PORT_RESULT_INCLUDE_DIR)

        foreach(_includedir ${_GPHOTO2PORT_RESULT_INCLUDE_DIR})

            string(REGEX REPLACE "-I(.+)" "\\1" _includedir "${_includedir}")
            set(GPHOTO2PORT_INCLUDE_DIR ${GPHOTO2PORT_INCLUDE_DIR} ${_includedir})

        endforeach()

        set(GPHOTO2_INCLUDE_DIRS ${GPHOTO2PORT_INCLUDE_DIR} ${GPHOTO2_INCLUDE_DIR} )

    else()

        message(WARNING "Could not find gphoto2-config and/or \
                         gphoto2-port-config executables.")

        set(GPHOTO2_FOUND FALSE)

    endif()

    if(GPHOTO2_LIBRARIES AND GPHOTO2_INCLUDE_DIRS)

        set(GPHOTO2_FOUND TRUE)
        message(STATUS "Found gphoto2: ${GPHOTO2_LIBRARIES}")

    else()

        message(WARNING "Could not find gphoto2 libraries.")
        set(GPHOTO2_FOUND FALSE)

    endif()

endif()

mark_as_advanced(GPHOTO2_LIBRARIES GPHOTO2_INCLUDE_DIRS)

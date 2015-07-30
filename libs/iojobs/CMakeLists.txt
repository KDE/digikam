#
# Copyright (c) 2015 by Mohamed Anwer, <m dot anwer at gmx dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libiojobs_SRCS
    iojob.cpp
    iojobsthread.cpp
    iojobsmanager.cpp
    dtrash.cpp
)

add_library(iojobs_src OBJECT ${libiojobs_SRCS})

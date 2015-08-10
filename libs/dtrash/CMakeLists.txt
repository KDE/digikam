#
# Copyright (c) 2015 by Mohamed Anwer, <m dot anwer at gmx dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libdtrash_SRCS
    dtrash.cpp
    dtrashiteminfo.cpp
    dtrashitemmodel.cpp
)

add_library(dtrash_src OBJECT ${libdtrash_SRCS})

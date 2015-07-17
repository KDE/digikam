#
# Copyright (c) 2015 by Mohamed Anwer, <m dot anwer at gmx dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libkiowrapper_SRCS
    kiowrapper.cpp
)

add_library(kiowrapper_src OBJECT ${libkiowrapper_SRCS})

#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015 by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libfilters_SRCS
    filterstatusbar.cpp
    geolocationfilter.cpp
    textfilter.cpp
    mimefilter.cpp
    colorlabelfilter.cpp
    picklabelfilter.cpp
    ratingfilter.cpp
)

#TODO rename to digikamfilters?

add_library(filters_src OBJECT ${libfilters_SRCS})

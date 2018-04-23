#!/bin/sh

# Copyright (c) 2013-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

cppcheck --enable=warning \
         --quiet \
         --force \
         --template=edit \
         --suppress=*:*CImg.h* \
         -i../../core/libs/dimg/filters/greycstoration/cimg \
         -i../../core/libs/rawengine/libraw \
         -i../../core/libs/dngwriter/extra \
         -i../../core/libs/facesengine/dnnface \
         -i../../core/libs/kmemoryinfo \
         -i../../core/utilities/mediaserver/upnpsdk \
         ../../core \

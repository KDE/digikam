#!/bin/sh

# Copyright (c) 2013-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Run CppCheck static analyzer on whole digiKam source code.
# http://cppcheck.sourceforge.net/
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

cppcheck -j4 \
         --verbose \
         --platform=unix64 \
         --enable=warning \
         --report-progress \
         --template=edit \
         --suppress=*:*CImg.h* \
         -i../../core/libs/dimg/filters/greycstoration/cimg \
         -i../../core/libs/rawengine/libraw \
         -i../../core/libs/dngwriter/extra \
         -i../../core/libs/facesengine/dnnface \
         -i../../core/libs/facesengine/opencv3-face \
         -i../../core/libs/kmemoryinfo \
         -i../../core/libs/pgfutils/libpgf \
         -i../../core/libs/jpegutils/libjpeg \
         -i../../core/utilities/mediaserver/upnpsdk \
         -i../../core/utilities/assistants/webservices/common/o2 \
         ../../core \
         2> report.cppcheck

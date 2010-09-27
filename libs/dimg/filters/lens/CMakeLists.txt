# ===========================================================
#
# This file is a part of digiKam project
# <a href="http://www.digikam.org">http://www.digikam.org</a>
#
# @date   2010-09-15
# @brief  test lensfun interface
#
# @author Copyright (C) 2010 by Gilles Caulier
#         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
#
# This program is free software; you can redistribute it
# and/or modify it under the terms of the GNU General
# Public License as published by the Free Software Foundation;
# either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# ============================================================

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/widgets/common
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/widgets/metadata
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../libs/widgets/iccprofiles
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/jpegutils
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/imageproperties
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg/loaders
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg/filters
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg/filters/auto
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg/filters/bcg
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg/filters/bw
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg/filters/decorate
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg/filters/fx
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg/filters/greycstoration
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg/filters/hsl
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg/filters/icc
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg/filters/lc
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg/filters/nr
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg/filters/sharp
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg/filters/levels
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg/filters/curves
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg/filters/wb
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg/filters/transform
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dimg/filters/lens
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dmetadata
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/themeengine
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/histogram
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/threadimageio
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/threads
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dialogs
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../utilities/setup
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../utilities/imageeditor/canvas
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../utilities/imageeditor/widgets
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../utilities/imageeditor/editor
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../utilities/imageeditor/printiface
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../utilities/imageeditor/rawimport
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../utilities/slideshow
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../digikam
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../

                     # version.h
                     ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CURRENT_SOURCE_DIR}/../../../
                     # external library paths
                     ${LCMS_INCLUDE_DIR}
                   )

SET(testlensfuniface_SRCS testlensfuniface.cpp)
KDE4_ADD_EXECUTABLE(testlensfuniface NOGUI ${testlensfuniface_SRCS})
TARGET_LINK_LIBRARIES(testlensfuniface digikamcore ${QT_QTCORE_LIBRARY} ${KDE4_KDECORE_LIBS})

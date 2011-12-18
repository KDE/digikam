# ===========================================================
#
# This file is a part of digiKam project
# <a href="http://www.digikam.org">http://www.digikam.org</a>
#
# @date   2010-09-15
# @brief  test lensfun interface
#
# @author Copyright (C) 2010-2011 by Gilles Caulier
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

SET(testlensfuniface_SRCS testlensfuniface.cpp)
KDE4_ADD_EXECUTABLE(testlensfuniface NOGUI ${testlensfuniface_SRCS})
TARGET_LINK_LIBRARIES(testlensfuniface
                      digikamcore
                      ${QT_QTCORE_LIBRARY}
                      ${KDE4_KDECORE_LIBS}
                      ${KEXIV2_LIBRARIES}

                      # To link under Solaris (see B.K.O #274484)
                      ${MATH_LIBRARY}
                     )

/*  This file is part of the KDE project
    Copyright (C) 2014 Laurent Montel <montel at kde dot org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "dimg_debug.h"

// note, per default only warnings and more severe messages are logged for other than general category
Q_LOGGING_CATEGORY(LOG_DIMG,        "digikam.dimg")
Q_LOGGING_CATEGORY(LOG_DIMG_JPEG,   "digikam.dimg.jpeg",   QtWarningMsg)
Q_LOGGING_CATEGORY(LOG_DIMG_JP2K,   "digikam.dimg.jp2k",   QtWarningMsg)
Q_LOGGING_CATEGORY(LOG_DIMG_PGF,    "digikam.dimg.pgf",    QtWarningMsg)
Q_LOGGING_CATEGORY(LOG_DIMG_PNG,    "digikam.dimg.png",    QtWarningMsg)
Q_LOGGING_CATEGORY(LOG_DIMG_PPM,    "digikam.dimg.ppm",    QtWarningMsg)
Q_LOGGING_CATEGORY(LOG_DIMG_TIFF,   "digikam.dimg.tiff",   QtWarningMsg)
Q_LOGGING_CATEGORY(LOG_DIMG_RAW,    "digikam.dimg.raw",    QtWarningMsg)
Q_LOGGING_CATEGORY(LOG_DIMG_QIMAGE, "digikam.dimg.qimage", QtWarningMsg)


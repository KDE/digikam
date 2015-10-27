/*  This file is part of the KDE project
    Copyright (C) 2014 Laurent Montel <montel at kde dot org>
    Copyright (C) 2015 Mohamed Anwer <m dot anwer at gmx dot com>

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

#include "digikam_debug.h"

Q_LOGGING_CATEGORY(DIGIKAM_GENERAL_LOG,        "digikam.general")
Q_LOGGING_CATEGORY(DIGIKAM_WIDGETS_LOG,        "digikam.widgets")
Q_LOGGING_CATEGORY(DIGIKAM_DBJOB_LOG,          "digikam.dbjob")
Q_LOGGING_CATEGORY(DIGIKAM_IOJOB_LOG,          "digikam.iojob")
Q_LOGGING_CATEGORY(DIGIKAM_SHOWFOTO_LOG,       "digikam.showfoto")
Q_LOGGING_CATEGORY(DIGIKAM_IMAGEPLUGINS_LOG,   "digikam.imageplugins")
Q_LOGGING_CATEGORY(DIGIKAM_DATABASESERVER_LOG, "digikam.databaseserver")
Q_LOGGING_CATEGORY(DIGIKAM_IMPORTUI_LOG,       "digikam.import")
Q_LOGGING_CATEGORY(DIGIKAM_METAENGINE_LOG,     "digikam.metaengine")
Q_LOGGING_CATEGORY(DIGIKAM_RAWENGINE_LOG,      "digikam.rawengine")
Q_LOGGING_CATEGORY(DIGIKAM_GEOIFACE_LOG,       "digikam.geoiface")

// NOTE: per default only warnings and more severe messages are logged for other than general category
Q_LOGGING_CATEGORY(DIGIKAM_DIMG_LOG,           "digikam.dimg")
Q_LOGGING_CATEGORY(DIGIKAM_DIMG_LOG_JPEG,      "digikam.dimg.jpeg",   QtWarningMsg)
Q_LOGGING_CATEGORY(DIGIKAM_DIMG_LOG_JP2K,      "digikam.dimg.jp2k",   QtWarningMsg)
Q_LOGGING_CATEGORY(DIGIKAM_DIMG_LOG_PGF,       "digikam.dimg.pgf",    QtWarningMsg)
Q_LOGGING_CATEGORY(DIGIKAM_DIMG_LOG_PNG,       "digikam.dimg.png",    QtWarningMsg)
Q_LOGGING_CATEGORY(DIGIKAM_DIMG_LOG_PPM,       "digikam.dimg.ppm",    QtWarningMsg)
Q_LOGGING_CATEGORY(DIGIKAM_DIMG_LOG_TIFF,      "digikam.dimg.tiff",   QtWarningMsg)
Q_LOGGING_CATEGORY(DIGIKAM_DIMG_LOG_RAW,       "digikam.dimg.raw",    QtWarningMsg)
Q_LOGGING_CATEGORY(DIGIKAM_DIMG_LOG_QIMAGE,    "digikam.dimg.qimage", QtWarningMsg)

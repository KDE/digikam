/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-08
 * Description : digiKam debug spaces
 *
 * Copyright (C) 2014      by Laurent Montel <montel at kde dot org>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "digikam_debug.h"

Q_LOGGING_CATEGORY(DIGIKAM_GENERAL_LOG,        "digikam.general")
Q_LOGGING_CATEGORY(DIGIKAM_WIDGETS_LOG,        "digikam.widgets")
Q_LOGGING_CATEGORY(DIGIKAM_IOJOB_LOG,          "digikam.iojob")
Q_LOGGING_CATEGORY(DIGIKAM_SHOWFOTO_LOG,       "digikam.showfoto")
Q_LOGGING_CATEGORY(DIGIKAM_WEBSERVICES_LOG,    "digikam.webservices")
Q_LOGGING_CATEGORY(DIGIKAM_DATABASESERVER_LOG, "digikam.databaseserver")
Q_LOGGING_CATEGORY(DIGIKAM_IMPORTUI_LOG,       "digikam.import")
Q_LOGGING_CATEGORY(DIGIKAM_METAENGINE_LOG,     "digikam.metaengine")
Q_LOGGING_CATEGORY(DIGIKAM_RAWENGINE_LOG,      "digikam.rawengine")
Q_LOGGING_CATEGORY(DIGIKAM_FACESENGINE_LOG,    "digikam.facesengine")
Q_LOGGING_CATEGORY(DIGIKAM_GEOIFACE_LOG,       "digikam.geoiface")

Q_LOGGING_CATEGORY(DIGIKAM_DATABASE_LOG,       "digikam.database")
Q_LOGGING_CATEGORY(DIGIKAM_DBENGINE_LOG,       "digikam.dbengine")
Q_LOGGING_CATEGORY(DIGIKAM_DBJOB_LOG,          "digikam.dbjob")
Q_LOGGING_CATEGORY(DIGIKAM_COREDB_LOG,         "digikam.coredb")
Q_LOGGING_CATEGORY(DIGIKAM_FACEDB_LOG,         "digikam.facedb")
Q_LOGGING_CATEGORY(DIGIKAM_THUMBSDB_LOG,       "digikam.thumbsdb")
Q_LOGGING_CATEGORY(DIGIKAM_SIMILARITYDB_LOG,   "digikam.similaritydb")

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

Q_LOGGING_CATEGORY(DIGIKAM_MEDIASRV_LOG,        "digikam.mediaserver")
Q_LOGGING_CATEGORY(DIGIKAM_MEDIASRV_LOG_INFO,   "digikam.mediaserver.info",   QtInfoMsg)
Q_LOGGING_CATEGORY(DIGIKAM_MEDIASRV_LOG_DEBUG,  "digikam.mediaserver.debug",  QtDebugMsg)
Q_LOGGING_CATEGORY(DIGIKAM_MEDIASRV_LOG_WARN,   "digikam.mediaserver.warn",   QtWarningMsg)
Q_LOGGING_CATEGORY(DIGIKAM_MEDIASRV_LOG_SEVERE, "digikam.mediaserver.severe", QtWarningMsg)
Q_LOGGING_CATEGORY(DIGIKAM_MEDIASRV_LOG_FATAL,  "digikam.mediaserver.fatal",  QtCriticalMsg)

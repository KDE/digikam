/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-18
 * Description : camera item info container
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef GPITEMINFO_H
#define GPITEMINFO_H

// Qt includes

#include <QList>
#include <QByteArray>
#include <QDateTime>

// KDE includes

#include <kurl.h>

// Local includes

#include "photoinfocontainer.h"

class QDataStream;

namespace Digikam
{

class GPItemInfo
{

public:

    enum DownloadStatus
    {
        DownloadUnknown  = -1,
        DownloadedNo     = 0,
        DownloadedYes    = 1,
        DownloadFailed   = 2,
        DownloadStarted  = 3,
        NewPicture       = 4
    };

public:

    GPItemInfo();
    ~GPItemInfo();

    bool isNull() const;
    KUrl url() const;

public:

    qint64             size;                 // Camera file size in bytes.

    int                width;                // Image width in pixels
    int                height;               // Image height in pixels
    int                downloaded;           // Download status of camera file. See DownloadStatus enum for details
    int                readPermissions;      // Read permission of camera file
    int                writePermissions;     // Write permission of camera file

    QString            name;                 // File name in camera file-system
    QString            folder;               // Folder path to acces to file in camera
    QString            mime;                 // Type mime of camera file
    QString            downloadName;         // New file-name to use during download from camera

    QDateTime          mtime;                // Modified time stamp of camera file

    PhotoInfoContainer photoInfo;            // Photo Info from camera file (get from file metadata)
};

QDataStream& operator<<(QDataStream&, const GPItemInfo&);
QDataStream& operator>>(QDataStream&, GPItemInfo&);

typedef QList<GPItemInfo> GPItemInfoList;

}  // namespace Digikam

#endif /* GPITEMINFO_H */

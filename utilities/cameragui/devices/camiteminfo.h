/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-18
 * Description : camera item info container
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CamItemInfo_H
#define CamItemInfo_H

// Qt includes

#include <QList>
#include <QByteArray>
#include <QDateTime>
#include <QDebug>

// KDE includes

#include <kurl.h>

// Local includes

#include "photoinfocontainer.h"

class QDataStream;

namespace Digikam
{

class CamItemInfo
{

public:

    enum DownloadStatus
    {
        DownloadUnknown  = -1,               // Donwload state is unknow
        DownloadedNo     = 0,                // Is not yet downloaded on computer
        DownloadedYes    = 1,                // Is already downloaded on computer
        DownloadFailed   = 2,                // Download is failed or have been aborted by user
        DownloadStarted  = 3,                // Download is under progress
        NewPicture       = 4                 // This is a new item from camera
    };

public:

    CamItemInfo();
    ~CamItemInfo();

    bool isNull() const;                     // Return true if all member in this container are null
    KUrl url() const;                        // Return the local file system (mounted on computer) url to the camera file

    /** Compare for camera inforation equality, not including variable values.
     */
    bool operator==(const CamItemInfo& t) const;

public:

    /// Static values taken from camera.
    qint64             size;                 // Camera file size in bytes.

    int                width;                // Image width in pixels
    int                height;               // Image height in pixels
    int                readPermissions;      // Read permission of camera file
    int                writePermissions;     // Write permission of camera file

    QString            name;                 // File name in camera file-system
    QString            folder;               // Folder path to access to file in camera
    QString            mime;                 // Type mime of camera file

    QDateTime          mtime;                // Modified time stamp of camera file

    PhotoInfoContainer photoInfo;            // Photo Info from camera file (get from file metadata)

    /// Variable values depending of user actions.
    int                downloaded;           // Download status of camera file. See DownloadStatus enum for details
    QString            downloadName;         // New file-name to use during download from camera
};

QDataStream& operator<<(QDataStream&, const CamItemInfo&);
QDataStream& operator>>(QDataStream&, CamItemInfo&);

typedef QList<CamItemInfo> CamItemInfoList;

//! kDebug() stream operator. Writes property @a info to the debug output in a nicely formatted way.
QDebug operator<<(QDebug dbg, const CamItemInfo& info);

}  // namespace Digikam

#endif /* CamItemInfo_H */

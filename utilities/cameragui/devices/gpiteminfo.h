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

    GPItemInfo()
    {
        size             = -1;
        width            = -1;
        height           = -1;
        downloaded       = DownloadUnknown;
        readPermissions  = -1;
        writePermissions = -1;
    };

    ~GPItemInfo(){};

    KUrl url() const
    {
        KUrl url;
        url.addPath(folder);
        url.setFileName(name);
        return url;
    };

public:

    qint64             size;

    int                width;
    int                height;
    int                downloaded;           // See DownloadStatus enum.
    int                readPermissions;
    int                writePermissions;

    QString            name;
    QString            folder;
    QString            mime;

    QDateTime          mtime;

    PhotoInfoContainer photoInfo;
};

QDataStream& operator<<(QDataStream&, const GPItemInfo&);
QDataStream& operator>>(QDataStream&, GPItemInfo&);

typedef QList<GPItemInfo> GPItemInfoList;

}  // namespace Digikam

#endif /* GPITEMINFO_H */

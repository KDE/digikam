/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-02-24
 * Description : Container for IOJob data.
 *
 * Copyright (C) 2018 by Maik Qualmann <metzpinguin at gmail dot com>
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

#ifndef DIGIKAM_IO_JOB_DATA_H
#define DIGIKAM_IO_JOB_DATA_H

// Qt includes

#include <QUrl>
#include <QList>
#include <QDateTime>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class PAlbum;
class ItemInfo;

class DIGIKAM_EXPORT IOJobData
{

public:

    enum Operation
    {
        Unknown = 0,
        CopyAlbum,
        CopyImage,
        CopyFiles,
        MoveAlbum,
        MoveImage,
        MoveFiles,
        Rename,
        Delete,
        Trash
    };

public:

    explicit IOJobData(int operation,
                       const QList<ItemInfo>& infos,
                       PAlbum* const dest = 0);

    explicit IOJobData(int operation,
                       const QList<QUrl>& urls,
                       PAlbum* const dest = 0);

    explicit IOJobData(int operation,
                       PAlbum* const src,
                       PAlbum* const dest = 0);

    explicit IOJobData(int operation,
                       const QList<QUrl>& urls,
                       const QUrl& dest);

    explicit IOJobData(int operation,
                       const ItemInfo& info,
                       const QString& newName,
                       bool overwrite = false);

    ~IOJobData();

    void             setItemInfos(const QList<ItemInfo>& infos);
    void             setSourceUrls(const QList<QUrl>& urls);

    void             setDestUrl(const QUrl& srcUrl,
                                const QUrl& destUrl);

    void             setProgressId(const QString& id);

    int              operation()                          const;

    bool             overwrite()                          const;

    PAlbum*          srcAlbum()                           const;
    PAlbum*          destAlbum()                          const;

    QUrl             destUrl(const QUrl& srcUrl = QUrl()) const;
    QUrl             getNextUrl()                         const;

    QString          getProgressId()                      const;
    QDateTime        jobTime()                            const;

    ItemInfo         findItemInfo(const QUrl& url)        const;

    QList<QUrl>      sourceUrls()                         const;
    QList<ItemInfo>  itemInfos()                          const;

private:

    IOJobData(const IOJobData&); // Disable

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_IO_JOB_DATA_H

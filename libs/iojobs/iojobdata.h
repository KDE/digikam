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

#ifndef IOJOB_DATA_H
#define IOJOB_DATA_H

// Qt includes

#include <QUrl>
#include <QList>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class PAlbum;
class ImageInfo;

class DIGIKAM_EXPORT IOJobData
{

public:

enum Operation
{
    CopyAlbum = 1 << 0,
    CopyImage = 1 << 1,
    CopyFiles = 1 << 2,
    MoveAlbum = 1 << 3,
    MoveImage = 1 << 4,
    MoveFiles = 1 << 5,
    Rename    = 1 << 6,
    Delete    = 1 << 7,
    DFiles    = 1 << 8, // not mark as obsolete in the database
    Trash     = 1 << 9
};

    explicit IOJobData(int operation,
                       const QList<ImageInfo>& infos,
                       PAlbum* const dest = 0);

    explicit IOJobData(int operation,
                       const QList<QUrl>& urls,
                       PAlbum* const dest = 0);

    explicit IOJobData(int operation,
                       PAlbum* const src,
                       PAlbum* const dest);

    explicit IOJobData(int operation,
                       const QList<QUrl>& urls,
                       const QUrl& dest);

    explicit IOJobData(int operation,
                       const ImageInfo& info,
                       const QString& newName);

    ~IOJobData();

    void             setImageInfos(const QList<ImageInfo>& infos);
    void             setSourceUrls(const QList<QUrl>& urls);
    void             setDestUrl(const QUrl& url);

    void             addProcessedUrl(const QUrl& url);

    int              operation()     const;

    PAlbum*          srcAlbum()      const;
    PAlbum*          destAlbum()     const;

    QUrl             srcUrl()        const;
    QUrl             destUrl()       const;
    QUrl             getNextUrl()    const;

    ImageInfo        imageInfo()     const;

    QList<QUrl>      sourceUrls()    const;
    QList<ImageInfo> imageInfos()    const;

    QList<QUrl>      processedUrls() const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IOJOB_DATA_H

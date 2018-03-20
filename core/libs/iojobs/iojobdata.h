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
    Unknown=0,
    CopyAlbum,
    CopyImage,
    CopyFiles,
    MoveAlbum,
    MoveImage,
    MoveFiles,
    Rename,
    Delete,
    DFiles, // not mark as obsolete in the database
    Trash
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

    void             setDestUrl(const QUrl& srcUrl,
                                const QUrl& destUrl);

    void             addProcessedUrl(const QUrl& url);

    int              operation()                          const;

    PAlbum*          srcAlbum()                           const;
    PAlbum*          destAlbum()                          const;

    QUrl             srcUrl()                             const;
    QUrl             destUrl(const QUrl& srcUrl = QUrl()) const;
    QUrl             getNextUrl()                         const;

    ImageInfo        imageInfo()                          const;

    QList<QUrl>      sourceUrls()                         const;
    QList<ImageInfo> imageInfos()                         const;

    QList<QUrl>      processedUrls()                      const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IOJOB_DATA_H

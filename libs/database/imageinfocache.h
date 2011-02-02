/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-05-01
 * Description : ImageInfo common data
 *
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C)      2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEINFOCACHE_H
#define IMAGEINFOCACHE_H

// Qt includes

#include <QObject>
#include <QHash>

// Local includes

#include "databasewatch.h"

namespace Digikam
{

class ImageInfoData;
class DatabaseAccess;

// No EXPORT class
class ImageInfoCache : public QObject
{
    Q_OBJECT

public:

    ImageInfoCache();
    ~ImageInfoCache();

    /**
     * Return an ImageInfoData object for the given image id.
     * A new object is created, or an existing object is returned.
     * If a new object is created, the id field will be initialized.
     */
    ImageInfoData* infoForId(qlonglong id);
    /**
     * Returns whether an ImageInfoObject for the given image id
     * is contained in the cache.
     */
    bool hasInfoForId(qlonglong id) const;

    /**
     * Call this when the reference count is dropped to 1.
     * This method is called under mutex lock, and will check
     * again the reference count. A count of 1 means the info is only
     * left here in the cache, all ImageInfo containers are gone.
     * The cache will delete this object when it wants.
     */
    void dropInfo(ImageInfoData* infodata);

    QString albumName(DatabaseAccess& access, int albumId);

    void invalidate();

private Q_SLOTS:

    void slotImageChanged(const ImageChangeset& changeset);
    void slotImageTagChanged(const ImageTagChangeset& changeset);
    void slotAlbumChange(const AlbumChangeset&);

private:

    QHash<qlonglong, ImageInfoData*> m_infos;
    QHash<int, QString>              m_albums;
};

}  // namespace Digikam

#endif // IMAGEINFOCACHE_H

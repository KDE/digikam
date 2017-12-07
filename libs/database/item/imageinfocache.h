/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-05-01
 * Description : ImageInfo common data
 *
 * Copyright (C) 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C)      2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QMultiHash>
#include <QHash>
#include <QObject>

// Local includes

#include "coredbwatch.h"
#include "dshareddata.h"

namespace Digikam
{

class AlbumShortInfo;
class ImageInfoData;

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
    DSharedDataPointer<ImageInfoData> infoForId(qlonglong id);

    /**
     * Call this when the data has been dereferenced,
     * before deletion.
     */
    void dropInfo(ImageInfoData* infodata);

    /**
     * Call this to put data in the hash by file name if you have newly created data
     * and the name is filled.
     * Call under write lock.
     */
    void cacheByName(ImageInfoData* data);

    /**
     * Return an ImageInfoData object for the given album root, relativePath and file name triple.
     * Works if previously cached with cacheByName.
     * Returns 0 if not found.
     */
    DSharedDataPointer<ImageInfoData> infoForPath(int albumRootId, const QString& relativePath, const QString& name);

    /**
     * Returns the cached relativePath for the given album id.
     */
    QString albumRelativePath(int albumId);

    /**
     * Invalidate the cache and all its cached data
     */
    void invalidate();

private Q_SLOTS:

    void slotImageChanged(const ImageChangeset& changeset);
    void slotImageTagChanged(const ImageTagChangeset& changeset);
    void slotAlbumChange(const AlbumChangeset&);

private:

    QList<AlbumShortInfo>::const_iterator findAlbum(int id);
    void                                  checkAlbums();

private:

    QHash<qlonglong, ImageInfoData*>    m_infos;
    QHash<ImageInfoData*, QString>      m_dataHash;
    QMultiHash<QString, ImageInfoData*> m_nameHash;
    volatile bool                       m_needUpdateAlbums;
    QList<AlbumShortInfo>               m_albums;
};

}  // namespace Digikam

#endif // IMAGEINFOCACHE_H

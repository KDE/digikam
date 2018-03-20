/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-08
 * Description : Qt item model for database entries, listing done with ioslave
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#ifndef IMAGEALBUMMODEL_H
#define IMAGEALBUMMODEL_H

// Local includes

#include "imagethumbnailmodel.h"
#include "album.h"

namespace Digikam
{

class ImageChangeset;
class CollectionImageChangeset;
class SearchChangeset;
class Album;

class ImageAlbumModel : public ImageThumbnailModel
{
    Q_OBJECT

public:

    explicit ImageAlbumModel(QObject* const parent = 0);
    ~ImageAlbumModel();

    QList<Album*> currentAlbums() const;

    bool hasScheduledRefresh() const;
    bool isRecursingAlbums() const;
    bool isRecursingTags() const;
    bool isListingOnlyAvailableImages() const;

public Q_SLOTS:

    /**
     * Call this method to populate the model with data from the given album.
     * If called with 0, the model will be empty.
     * Opening the same album again is a no-op.
     */
    void openAlbum(QList<Album*> album);
    /** Reloads the current album */
    void refresh();

    void setRecurseAlbums(bool recursiveListing);
    void setRecurseTags(bool recursiveListing);
    void setListOnlyAvailableImages(bool onlyAvailable);

    void setSpecialTagListing(const QString& specialListing);

//Q_SIGNALS:

    //void listedAlbumChanged(QList<Album*> album);

protected Q_SLOTS:

    void scheduleRefresh();
    void scheduleIncrementalRefresh();

    void slotResult();
    void slotData(const QList<ImageListerRecord> &records);

    void slotNextRefresh();
    void slotNextIncrementalRefresh();

    void slotCollectionImageChange(const CollectionImageChangeset& changeset);
    void slotSearchChange(const SearchChangeset& changeset);

    void slotAlbumAdded(Album* album);
    void slotAlbumDeleted(Album* album);
    void slotAlbumRenamed(Album* album);
    void slotAlbumsCleared();

    void incrementalRefresh();

    virtual void slotImageChange(const ImageChangeset& changeset);
    virtual void slotImageTagChange(const ImageTagChangeset& changeset);

protected:

    void startListJob(QList<Album*> album);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IMAGEALBUMMODEL_H

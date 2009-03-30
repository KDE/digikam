/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-08
 * Description : Qt item model for database entries, listing done with ioslave
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes.

// Local includes.

#include "imagemodel.h"
#include "album.h"
#include "digikam_export.h"

namespace KIO { class Job; }
class KJob;

namespace Digikam
{

class CollectionImageChangeset;
class SearchChangeset;
class Album;
class ImageAlbumModelPriv;

class DIGIKAM_MODEL_EXPORT ImageAlbumModel : public ImageModel
{
    Q_OBJECT

public:

    ImageAlbumModel(QObject *parent = 0);
    ~ImageAlbumModel();

    Album *currentAlbum() const;

public Q_SLOTS:

    /**
     * Call this method to populate the model with data from the given album.
     * If called with 0, the model will be empty.
     * Opening the same album again is a no-op.
     */
    void openAlbum(Album *album);
    /** Reloads the current album */
    void refresh();

Q_SIGNALS:

    void listedAlbumChanged(Album *album);

protected Q_SLOTS:

    void slotResult(KJob* job);
    void slotData(KIO::Job* job, const QByteArray& data);

    void slotNextRefresh();
    void slotCollectionImageChange(const CollectionImageChangeset &changeset);
    void slotSearchChange(const SearchChangeset &changeset);

    void slotRecurseSettingsChanged();

protected:

    void startLoadingAlbum(Album *album);
    void startListJob(const KUrl &url);

private:

    ImageAlbumModelPriv *const d;
};

} // namespace Digikam

#endif // IOSLAVEIMAGEMODEL_H

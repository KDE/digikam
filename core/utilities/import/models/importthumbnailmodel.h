/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-06-13
 * Description : Qt item model for camera thumbnails entries
 *
 * Copyright (C) 2009-2012 by Islam Wazery <wazery at ubuntu dot com>
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

#ifndef IMPORTTHUMBNAILMODEL_H
#define IMPORTTHUMBNAILMODEL_H

// Local includes

#include "importimagemodel.h"
#include "thumbnailsize.h"
#include "camerathumbsctrl.h"
#include "digikam_export.h"

namespace Digikam
{

typedef QPair<CamItemInfo, QPixmap> CachedItem;

class DIGIKAM_EXPORT ImportThumbnailModel : public ImportImageModel
{
    Q_OBJECT

public:

    /**
     *  This model provides thumbnail loading, it uses the Camera Controller
     *  to retrieve thumbnails for CamItemInfos. It also provides preloading of thumbnails,
     *  and caching facility. Thumbnails size can be adjusted.
     */
    explicit ImportThumbnailModel(QObject* const parent);
    ~ImportThumbnailModel();

    /// Sets the camera thumbs controller which is used to get the thumbnails for item infos.
    void setCameraThumbsController(CameraThumbsCtrl* const thumbsCtrl);

    /// Get the thumbnail size
    ThumbnailSize thumbnailSize() const;

    /**
     *  Enable emitting dataChanged() when a thumbnail becomes available.
     *  The thumbnailAvailable() signal will be emitted in any case.
     *  Default is true.
     */
    void setEmitDataChanged(bool emitSignal);

    /**
     *  Handles the ThumbnailRole.
     *  If the pixmap is available, returns it in the QVariant.
     *  If it still needs to be loaded, returns a null QVariant and emits
     *  thumbnailAvailable() as soon as it is available.
     */
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    /**
     * You can override the current thumbnail size by giving an integer value for ThumbnailRole.
     * Set a null QVariant to use the thumbnail size set by setThumbnailSize() again.
     * The index given here is ignored for this purpose.
     */
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole);

Q_SIGNALS:

    void thumbnailAvailable(const QModelIndex& index, int requestedSize);
    void thumbnailFailed(const QModelIndex& index, int requestedSize);

private Q_SLOTS:

    void slotThumbInfoReady(const CamItemInfo&);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* IMPORTTHUMBNAILMODEL_H */

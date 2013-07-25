/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-07-22
 * Description : Qt item model for Showfoto thumbnails entries
 *
 * Copyright (C) 2013 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#ifndef SHOWFOTOTHUMBNAILMODEL_H
#define SHOWFOTOTHUMBNAILMODEL_H

// Local Include

#include "showfotoimagemodel.h"
#include "thumbnailsize.h"
#include "loadingdescription.h"

namespace ShowFoto {

typedef QPair<ShowfotoItemInfo, QPixmap> CachedItem;

class ShowfotoThumbnailModel : public ShowfotoImageModel
{
    Q_OBJECT

public:
    /**
     *  An ThumbnailModel that supports thumbnail loading.
     *  You need to set a ThumbnailLoadThread to enable thumbnail loading.
     *  Adjust the thumbnail size to your needs.
     *  Note that setKeepsFilePathCache is enabled per default.
     */
    explicit ShowfotoThumbnailModel(QObject* parent);
    ~ShowfotoThumbnailModel();

    void setLoader(ShowfotoItemLoader* Loader);

    /// Set the thumbnail size to use
    void setThumbnailSize(const ThumbnailSize& thumbSize);

    /// Get the thumbnail size
    ThumbnailSize thumbnailSize() const;

    void setExifRotate(bool rotate);

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

public Q_SLOTS:
    void slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb);

private:
    class Private;
    Private* const d;
};

}

#endif // SHOWFOTOTHUMBNAILMODEL_H

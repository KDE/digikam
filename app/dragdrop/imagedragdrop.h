/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-16
 * Description : Qt Model for Albums - drag and drop handling
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGEDRAGDROP_H
#define IMAGEDRAGDROP_H

// KDE includes

#include <kurl.h>

// Local includes

#include "imageinfo.h"
#include "abstractitemdragdrophandler.h"
#include "imagealbummodel.h"

class KJob;

namespace Digikam
{

class ImageDragDropHandler : public AbstractItemDragDropHandler
{
    Q_OBJECT

public:

    explicit ImageDragDropHandler(ImageModel* const model);

    ImageModel* model() const
    {
        return static_cast<ImageModel*>(m_model);
    }

    ImageAlbumModel* albumModel() const;

    /**
     * Enables a mode in which dropping will never start an operation
     * which copies or moves files on disk.
     * Only the signals are emitted.
     */
    void setReadOnlyDrop(bool readOnly);

    virtual bool dropEvent(QAbstractItemView* view, const QDropEvent* e, const QModelIndex& droppedOn);
    virtual Qt::DropAction accepts(const QDropEvent* e, const QModelIndex& dropIndex);
    virtual QStringList mimeTypes() const;
    virtual QMimeData* createMimeData(const QList<QModelIndex> &);

Q_SIGNALS:

    void imageInfosDropped(const QList<ImageInfo>& infos);
    void urlsDropped(const KUrl::List& urls);
    void assignTags(const QList<ImageInfo>& list, const QList<int>& tagIDs);
    void addToGroup(const ImageInfo& pick, const QList<ImageInfo>& infos);
    void dioResult(KJob*);

protected:

    bool m_readOnly;
};

} // namespace Digikam

#endif /* IMAGEDRAGDROP_H */

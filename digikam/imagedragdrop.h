/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-16
 * Description : Qt Model for Albums - drag and drop handling
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

#ifndef IMAGEDRAGDROP_H
#define IMAGEDRAGDROP_H

// Local includes

#include "imagemodeldragdrophandler.h"
#include "imagealbummodel.h"

class KJob;

namespace Digikam
{

class ImageDragDropHandler : public ImageModelDragDropHandler
{
    Q_OBJECT

public:

    ImageDragDropHandler(ImageAlbumModel *model);

    ImageAlbumModel *model() const { return static_cast<ImageAlbumModel*>(m_model); }

    virtual bool dropEvent(QAbstractItemView *view, const QDropEvent *e, const QModelIndex& droppedOn);
    virtual Qt::DropAction accepts(const QDropEvent *e, const QModelIndex& dropIndex);
    virtual QStringList mimeTypes() const;
    virtual QMimeData *createMimeData(const QList<ImageInfo> &);

Q_SIGNALS:

    void assignTags(const QList<ImageInfo>& list, const QList<int>& tagIDs);
    void dioResult(KJob *);
};

} // namespace Digikam

#endif /* IMAGEDRAGDROP_H */

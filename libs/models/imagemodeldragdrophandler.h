/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-26
 * Description : Qt Model for Images - drag and drop handling
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

#ifndef IMAGEMODELDRAGDROPHANDLER_H
#define IMAGEMODELDRAGDROPHANDLER_H

// Qt includes

#include <QAbstractItemModel>
#include <QAbstractItemView>

// Local includes

#include "imageinfo.h"
#include "digikam_export.h"

namespace Digikam
{

class Album;
class ImageModel;

class DIGIKAM_DATABASE_EXPORT ImageModelDragDropHandler : public QObject
{
public:

    ImageModelDragDropHandler(ImageModel *model);
    virtual ~ImageModelDragDropHandler() {}

    ImageModel *model() const;

    /** Gives the view and the occurring drop event.
     *  The index is the index where the drop was dropped on.
     *  It may be invalid (dropped on decoration, viewport)
     *  Returns true if the event is to be accepted.
     */
    virtual bool dropEvent(QAbstractItemView *view, const QDropEvent *e, const QModelIndex &droppedOn);

    /** Returns if the given mime data is accepted for drop on dropIndex.
     *  Returns the proposed action, or Qt::IgnoreAction if not accepted. */
    virtual Qt::DropAction accepts(const QDropEvent *e, const QModelIndex &dropIndex);

    /** Returns the supported mime types.
     *  Called by the default implementation of model's mimeTypes(). */
    virtual QStringList mimeTypes() const;

    /** Create a mime data object for starting a drag from the given Albums */
    virtual QMimeData *createMimeData(const QList<ImageInfo> &);

protected:

    ImageModel *m_model;
};

} // namespace Digikam

#endif // IMAGEMODELDRAGDROPHANDLER_H

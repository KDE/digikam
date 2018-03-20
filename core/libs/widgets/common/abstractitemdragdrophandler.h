/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-26
 * Description : Qt Model for Images - drag and drop handling
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

#ifndef ABSTRACTITEMDRAGDROPHANDLER_H
#define ABSTRACTITEMDRAGDROPHANDLER_H

// Qt includes

#include <QAbstractItemModel>

// Local includes

#include "digikam_export.h"

class QAbstractItemView;
class QDropEvent;

namespace Digikam
{

class DIGIKAM_EXPORT AbstractItemDragDropHandler : public QObject
{
public:

    explicit AbstractItemDragDropHandler(QAbstractItemModel* const model);
    virtual ~AbstractItemDragDropHandler() {}

    QAbstractItemModel* model() const;

    /** Gives the view and the occurring drop event.
     *  The index is the index where the drop was dropped on.
     *  It may be invalid (dropped on decoration, viewport)
     *  Returns true if the event is to be accepted.
     */
    virtual bool dropEvent(QAbstractItemView* view, const QDropEvent* e, const QModelIndex& droppedOn);

    /** Returns if the given mime data is accepted for drop on dropIndex.
     *  Returns the proposed action, or Qt::IgnoreAction if not accepted. */
    virtual Qt::DropAction accepts(const QDropEvent* e, const QModelIndex& dropIndex);

    /** Returns the supported mime types.
     *  Called by the default implementation of model's mimeTypes(). */
    virtual QStringList mimeTypes() const;

    /** Create a mime data object for starting a drag from the given Albums */
    virtual QMimeData* createMimeData(const QList<QModelIndex>&);

    /** Returns if the given mime data can be handled. acceptsMimeData shall return true
     *  if a drop of the given mime data will be accepted on any index or place at all.
     *  If this returns false, the more specific method accepts() will not be called for this drag.
     *  The default implementation uses mimeTypes() to check for supported mime types.
     *  There is usually no need to reimplement this. */
    virtual bool acceptsMimeData(const QMimeData* data);

protected:

    QAbstractItemModel* m_model;
};

} // namespace Digikam

#endif // ABSTRACTITEMDRAGDROPHANDLER_H

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Central object for managing bookmarks
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef GPS_BOOKMARK_MODEL_HELPER_H
#define GPS_BOOKMARK_MODEL_HELPER_H

// Qt includes

#include <QObject>

// Local includes

#include "geomodelhelper.h"
#include "gpsdatacontainer.h"
#include "bookmarksmngr.h"

namespace Digikam
{

class GPSImageModel;
class GPSUndoCommand;

class GPSBookmarkModelHelper : public GeoModelHelper
{
    Q_OBJECT

public:

    enum Constants
    {
        CoordinatesRole = Qt::UserRole + 1
    };

public:

    GPSBookmarkModelHelper(BookmarksManager* const bookmarkManager,
                           GPSImageModel* const imageModel,
                           QObject* const parent = 0);
    virtual ~GPSBookmarkModelHelper();

    void setVisible(const bool state);

    virtual QAbstractItemModel* model() const;
    virtual QItemSelectionModel* selectionModel() const;
    virtual bool itemCoordinates(const QModelIndex& index,
                                 GeoCoordinates* const coordinates) const;
    virtual bool itemIcon(const QModelIndex& index, QPoint* const offset,
                          QSize* const size, QPixmap* const pixmap,
                          QUrl* const url) const;
    virtual PropertyFlags modelFlags() const;
    virtual PropertyFlags itemFlags(const QModelIndex& index) const;
    virtual void snapItemsTo(const QModelIndex& targetIndex,
                             const QList<QModelIndex>& snappedIndices);

private Q_SLOTS:

    void slotUpdateBookmarksModel();

Q_SIGNALS:

    void signalUndoCommand(GPSUndoCommand* undoCommand);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // GPS_BOOKMARK_MODEL_HELPER_H

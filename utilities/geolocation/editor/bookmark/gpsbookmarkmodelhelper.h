/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Central object for managing bookmarks
 *
 * Copyright (C) 2010-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef GPSBOOKMARKMODELHELPER_H
#define GPSBOOKMARKMODELHELPER_H

// Local includes

#include "modelhelper.h"

// Local includes

#include "gpsdatacontainer.h"

class KBookmarkManager;

namespace Digikam
{

class GPSImageModel;
class GPSUndoCommand;

class GPSBookmarkModelHelper : public GeoIface::ModelHelper
{
    Q_OBJECT

public:

    enum Constants
    {
        CoordinatesRole = Qt::UserRole + 1
    };

public:

    GPSBookmarkModelHelper(KBookmarkManager* const bookmarkManager, GPSImageModel* const imageModel, QObject* const parent = 0);
    virtual ~GPSBookmarkModelHelper();

    void setVisible(const bool state);

    virtual QAbstractItemModel* model() const;
    virtual QItemSelectionModel* selectionModel() const;
    virtual bool itemCoordinates(const QModelIndex& index, GeoIface::GeoCoordinates* const coordinates) const;
    virtual bool itemIcon(const QModelIndex& index, QPoint* const offset, QSize* const size, QPixmap* const pixmap, QUrl* const url) const;
    virtual Flags modelFlags() const;
    virtual Flags itemFlags(const QModelIndex& index) const;
    virtual void snapItemsTo(const QModelIndex& targetIndex, const QList<QModelIndex>& snappedIndices);

private Q_SLOTS:

    void slotUpdateBookmarksModel();

Q_SIGNALS:

    void signalUndoCommand(GPSUndoCommand* undoCommand);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // GPSBOOKMARKMODELHELPER_H

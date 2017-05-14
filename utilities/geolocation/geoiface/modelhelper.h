/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : Helper class to access models
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

#ifndef MODELHELPER_H
#define MODELHELPER_H

// Qt includes

#include <QItemSelectionModel>
#include <QPixmap>
#include <QAbstractItemModel>
#include <QPersistentModelIndex>
#include <QPoint>
#include <QString>

// Local includes

#include "geoiface_types.h"
#include "geocoordinates.h"
#include "digikam_export.h"

namespace GeoIface
{

class DIGIKAM_EXPORT ModelHelper : public QObject
{
    Q_OBJECT

public:

    enum Flag
    {
        FlagNull    = 0,
        FlagVisible = 1,
        FlagMovable = 2,
        FlagSnaps   = 4
    };

    Q_DECLARE_FLAGS(Flags, Flag)

public:

    explicit ModelHelper(QObject* const parent = 0);
    virtual ~ModelHelper();

    void snapItemsTo(const QModelIndex& targetIndex, const QList<QPersistentModelIndex>& snappedIndices);

    // these are necessary for grouped and ungrouped models
    virtual QAbstractItemModel* model() const = 0;
    virtual QItemSelectionModel* selectionModel() const = 0;
    virtual bool itemCoordinates(const QModelIndex& index, GeoCoordinates* const coordinates) const = 0;
    virtual Flags modelFlags() const;

    // these are necessary for ungrouped models
    virtual bool itemIcon(const QModelIndex& index, QPoint* const offset, QSize* const size, QPixmap* const pixmap, QUrl* const url) const;
    virtual Flags itemFlags(const QModelIndex& index) const;
    virtual void snapItemsTo(const QModelIndex& targetIndex, const QList<QModelIndex>& snappedIndices);

    // these are used by MarkerModel for grouped models
    virtual QPixmap pixmapFromRepresentativeIndex(const QPersistentModelIndex& index, const QSize& size);
    virtual QPersistentModelIndex bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list, const int sortKey);

    virtual void onIndicesClicked(const QList<QPersistentModelIndex>& clickedIndices);
    virtual void onIndicesMoved(const QList<QPersistentModelIndex>& movedIndices, const GeoCoordinates& targetCoordinates, const QPersistentModelIndex& targetSnapIndex);

Q_SIGNALS:

    void signalVisibilityChanged();
    void signalThumbnailAvailableForIndex(const QPersistentModelIndex& index, const QPixmap& pixmap);
    void signalModelChangedDrastically();
};

} // namespace GeoIface

Q_DECLARE_OPERATORS_FOR_FLAGS(GeoIface::ModelHelper::Flags)

#endif // MODELHELPER_H

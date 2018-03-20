/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : Helper class to access models
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

#ifndef GEO_MODEL_HELPER_H
#define GEO_MODEL_HELPER_H

// Qt includes

#include <QItemSelectionModel>
#include <QPixmap>
#include <QAbstractItemModel>
#include <QPersistentModelIndex>
#include <QPoint>
#include <QString>

// Local includes

#include "geoifacetypes.h"
#include "geocoordinates.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT GeoModelHelper : public QObject
{
    Q_OBJECT

public:

    enum PropertyFlag
    {
        FlagNull    = 0,
        FlagVisible = 1,
        FlagMovable = 2,
        FlagSnaps   = 4
    };

    Q_DECLARE_FLAGS(PropertyFlags, PropertyFlag)

public:

    explicit GeoModelHelper(QObject* const parent = 0);
    virtual ~GeoModelHelper();

    void snapItemsTo(const QModelIndex& targetIndex,
                     const QList<QPersistentModelIndex>& snappedIndices);

    // these are necessary for grouped and ungrouped models
    virtual QAbstractItemModel* model() const = 0;
    virtual QItemSelectionModel* selectionModel() const = 0;
    virtual bool itemCoordinates(const QModelIndex& index,
                                 GeoCoordinates* const coordinates) const = 0;
    virtual PropertyFlags modelFlags() const;

    // these are necessary for ungrouped models
    virtual bool itemIcon(const QModelIndex& index,
                          QPoint* const offset,
                          QSize* const size,
                          QPixmap* const pixmap,
                          QUrl* const url) const;
    virtual PropertyFlags itemFlags(const QModelIndex& index) const;
    virtual void snapItemsTo(const QModelIndex& targetIndex,
                             const QList<QModelIndex>& snappedIndices);

    // these are used by MarkerModel for grouped models
    virtual QPixmap pixmapFromRepresentativeIndex(const QPersistentModelIndex& index,
                                                  const QSize& size);
    virtual QPersistentModelIndex bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list,
                                                                  const int sortKey);

    virtual void onIndicesClicked(const QList<QPersistentModelIndex>& clickedIndices);
    virtual void onIndicesMoved(const QList<QPersistentModelIndex>& movedIndices,
                                const GeoCoordinates& targetCoordinates,
                                const QPersistentModelIndex& targetSnapIndex);

Q_SIGNALS:

    void signalVisibilityChanged();
    void signalThumbnailAvailableForIndex(const QPersistentModelIndex& index,
                                          const QPixmap& pixmap);
    void signalModelChangedDrastically();
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::GeoModelHelper::PropertyFlags)

#endif // GEO_MODEL_HELPER_H

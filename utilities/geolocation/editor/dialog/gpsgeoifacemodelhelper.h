/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-05-16
 * Description : A tool to edit geolocation
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2010      by Gabriel Voicu <ping dot gabi at gmail dot com>
 * Copyright (C) 2014      by Justus Schwartz <justus at gmx dot li>
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

#ifndef GPSGEOIFACEMODELHELPER_H
#define GPSGEOIFACEMODELHELPER_H

// Qt includes

#include <QModelIndex>

// Local includes

#include "geomodelhelper.h"

namespace Digikam
{

class GPSImageModel;
class GPSUndoCommand;
class MapWidget;

class GPSGeoIfaceModelHelper : public GeoModelHelper
{
    Q_OBJECT

public:

    GPSGeoIfaceModelHelper(GPSImageModel* const model,
                           QItemSelectionModel* const selectionModel,
                           QObject* const parent = 0);
    virtual ~GPSGeoIfaceModelHelper();

    virtual QAbstractItemModel*  model()          const;
    virtual QItemSelectionModel* selectionModel() const;
    virtual bool itemCoordinates(const QModelIndex& index,
                                 GeoCoordinates* const coordinates) const;
    virtual PropertyFlags modelFlags() const;

    virtual QPixmap pixmapFromRepresentativeIndex(const QPersistentModelIndex& index,
                                                  const QSize& size);
    virtual QPersistentModelIndex bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list,
                                                                  const int sortKey);

    virtual void onIndicesMoved(const QList<QPersistentModelIndex>& movedMarkers,
                                const GeoCoordinates& targetCoordinates,
                                const QPersistentModelIndex& targetSnapIndex);

    void addUngroupedModelHelper(GeoModelHelper* const newModelHelper);

private Q_SLOTS:

    void slotThumbnailFromModel(const QPersistentModelIndex& index, const QPixmap& pixmap);

Q_SIGNALS:

    void signalUndoCommand(GPSUndoCommand* undoCommand);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // GPSGEOIFACEMODELHELPER_H

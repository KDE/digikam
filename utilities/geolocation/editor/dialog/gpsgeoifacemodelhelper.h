/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2006-05-16
 * @brief  A tool to edit geolocation
 *
 * @author Copyright (C) 2006-2017 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2010, 2014 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010 by Gabriel Voicu
 *         <a href="mailto:ping dot gabi at gmail dot com">ping dot gabi at gmail dot com</a>
 * @author Copyright (C) 2014 by Justus Schwartz
 *         <a href="mailto:justus at gmx dot li">justus at gmx dot li</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef GPSGEOIFACEMODELHELPER_H
#define GPSGEOIFACEMODELHELPER_H

// Qt includes

#include <QModelIndex>

// Local includes

#include "modelhelper.h"

namespace GeoIface
{
    class MapWidget;
}

using namespace GeoIface;

namespace Digikam
{

class GPSImageModel;
class GPSUndoCommand;

class GPSGeoIfaceModelHelper : public ModelHelper
{
    Q_OBJECT

public:

    GPSGeoIfaceModelHelper(GPSImageModel* const model, QItemSelectionModel* const selectionModel, QObject* const parent = 0);
    virtual ~GPSGeoIfaceModelHelper();

    virtual QAbstractItemModel*  model()          const;
    virtual QItemSelectionModel* selectionModel() const;
    virtual bool itemCoordinates(const QModelIndex& index, GeoCoordinates* const coordinates) const;
    virtual Flags modelFlags() const;

    virtual QPixmap pixmapFromRepresentativeIndex(const QPersistentModelIndex& index, const QSize& size);
    virtual QPersistentModelIndex bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list, const int sortKey);

    virtual void onIndicesMoved(const QList<QPersistentModelIndex>& movedMarkers,
                                const GeoCoordinates& targetCoordinates, const QPersistentModelIndex& targetSnapIndex);

    void addUngroupedModelHelper(ModelHelper* const newModelHelper);

private Q_SLOTS:

    void slotThumbnailFromModel(const QPersistentModelIndex& index, const QPixmap& pixmap);

Q_SIGNALS:

    void signalUndoCommand(GPSUndoCommand* undoCommand);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* GPSGEOIFACEMODELHELPER_H */

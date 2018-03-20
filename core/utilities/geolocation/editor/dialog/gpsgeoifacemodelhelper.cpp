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

#include "gpsgeoifacemodelhelper.h"

// Qt includes

#include <QtConcurrentMap>
#include <QCloseEvent>
#include <QFuture>
#include <QFutureWatcher>
#include <QPointer>
#include <QTimer>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "gpscommon.h"
#include "gpsimagemodel.h"
#include "gpsimageitem.h"
#include "gpsundocommand.h"
#include "mapdragdrophandler.h"
#include "backend-rg.h"
#include "digikam_debug.h"

namespace Digikam
{

class GPSGeoIfaceModelHelper::Private
{
public:

    Private()
    {
        model          = 0;
        selectionModel = 0;
    }

    GPSImageModel*         model;
    QItemSelectionModel*   selectionModel;
    QList<GeoModelHelper*> ungroupedModelHelpers;
};

GPSGeoIfaceModelHelper::GPSGeoIfaceModelHelper(GPSImageModel* const model,
                                               QItemSelectionModel* const selectionModel,
                                               QObject* const parent)
    : GeoModelHelper(parent),
      d(new Private())
{
    d->model          = model;
    d->selectionModel = selectionModel;

    connect(d->model, SIGNAL(signalThumbnailForIndexAvailable(QPersistentModelIndex,QPixmap)),
            this, SLOT(slotThumbnailFromModel(QPersistentModelIndex,QPixmap)));

    connect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SIGNAL(signalModelChangedDrastically()));
}

QAbstractItemModel* GPSGeoIfaceModelHelper::model() const
{
    return d->model;
}

QItemSelectionModel* GPSGeoIfaceModelHelper::selectionModel() const
{
    return d->selectionModel;
}

bool GPSGeoIfaceModelHelper::itemCoordinates(const QModelIndex& index,
                                             GeoCoordinates* const coordinates) const
{
    GPSImageItem* const item = d->model->itemFromIndex(index);

    if (!item)
        return false;

    if (!item->gpsData().hasCoordinates())
        return false;

    if (coordinates)
        *coordinates = item->gpsData().getCoordinates();

    return true;
}

GPSGeoIfaceModelHelper::~GPSGeoIfaceModelHelper()
{
    delete d;
}

QPixmap GPSGeoIfaceModelHelper::pixmapFromRepresentativeIndex(const QPersistentModelIndex& index,
                                                              const QSize& size)
{
    return d->model->getPixmapForIndex(index, qMax(size.width(), size.height()));
}

QPersistentModelIndex GPSGeoIfaceModelHelper::bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list,
                                                                              const int sortKey)
{
    const bool oldestFirst = sortKey & 1;
    QPersistentModelIndex bestIndex;
    QDateTime             bestTime;

    for (int i = 0 ; i < list.count() ; ++i)
    {
        const QPersistentModelIndex currentIndex = list.at(i);
        const GPSImageItem* const currentItem   = static_cast<GPSImageItem*>(d->model->itemFromIndex(currentIndex));
        const QDateTime currentTime              = currentItem->dateTime();
        bool takeThisIndex                       = bestTime.isNull();

        if (!takeThisIndex)
        {
            if (oldestFirst)
            {
                takeThisIndex = currentTime < bestTime;
            }
            else
            {
                takeThisIndex = bestTime < currentTime;
            }
        }

        if (takeThisIndex)
        {
            bestIndex = currentIndex;
            bestTime  = currentTime;
        }
    }

    return bestIndex;
}

void GPSGeoIfaceModelHelper::slotThumbnailFromModel(const QPersistentModelIndex& index,
                                                    const QPixmap& pixmap)
{
    emit(signalThumbnailAvailableForIndex(index, pixmap));
}

void GPSGeoIfaceModelHelper::onIndicesMoved(const QList<QPersistentModelIndex>& movedMarkers,
                                            const GeoCoordinates& targetCoordinates,
                                            const QPersistentModelIndex& targetSnapIndex)
{
    if (targetSnapIndex.isValid())
    {
        const QAbstractItemModel* const targetModel = targetSnapIndex.model();

        for (int i = 0; i < d->ungroupedModelHelpers.count(); ++i)
        {
            GeoModelHelper* const ungroupedHelper = d->ungroupedModelHelpers.at(i);

            if (ungroupedHelper->model() == targetModel)
            {
                QList<QModelIndex> iMovedMarkers;

                for (int i = 0; i < movedMarkers.count(); ++i)
                {
                    iMovedMarkers << movedMarkers.at(i);
                }

                ungroupedHelper->snapItemsTo(targetSnapIndex, iMovedMarkers);
                return;
            }
        }
    }

    GPSUndoCommand* const undoCommand = new GPSUndoCommand();

    for (int i = 0; i < movedMarkers.count(); ++i)
    {
        const QPersistentModelIndex itemIndex = movedMarkers.at(i);
        GPSImageItem* const item              = static_cast<GPSImageItem*>(d->model->itemFromIndex(itemIndex));

        GPSUndoCommand::UndoInfo undoInfo(itemIndex);
        undoInfo.readOldDataFromItem(item);

        GPSDataContainer newData;
        newData.setCoordinates(targetCoordinates);
        item->setGPSData(newData);

        undoInfo.readNewDataFromItem(item);
        undoCommand->addUndoInfo(undoInfo);
    }

    undoCommand->setText(i18np("1 image moved", "%1 images moved", movedMarkers.count()));

    emit(signalUndoCommand(undoCommand));
}

void GPSGeoIfaceModelHelper::addUngroupedModelHelper(GeoModelHelper* const newModelHelper)
{
    d->ungroupedModelHelpers << newModelHelper;
}

GeoModelHelper::PropertyFlags GPSGeoIfaceModelHelper::modelFlags() const
{
    return FlagMovable;
}

} // namespace Digikam

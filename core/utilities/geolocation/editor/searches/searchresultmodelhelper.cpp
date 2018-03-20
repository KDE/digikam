/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-01
 * Description : A widget to search for places.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
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

#include "searchresultmodelhelper.h"

// Qt includes

#include <QContextMenuEvent>
#include <QListView>
#include <QPainter>
#include <QPushButton>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QComboBox>
#include <QStandardPaths>
#include <QLineEdit>
#include <QMessageBox>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "searchresultmodel.h"
#include "gpscommon.h"
#include "gpsundocommand.h"
#include "gpsimagemodel.h"

namespace Digikam
{

class SearchResultModelHelper::Private
{
public:

    Private()
      : model(0),
        selectionModel(0),
        imageModel(0),
        visible(true)
    {
    }

    SearchResultModel*   model;
    QItemSelectionModel* selectionModel;
    GPSImageModel*       imageModel;
    bool                 visible;
};

SearchResultModelHelper::SearchResultModelHelper(SearchResultModel* const resultModel,
                                                 QItemSelectionModel* const selectionModel,
                                                 GPSImageModel* const imageModel,
                                                 QObject* const parent)
    : GeoModelHelper(parent),
      d(new Private())
{
    d->model          = resultModel;
    d->selectionModel = selectionModel;
    d->imageModel     = imageModel;
}

SearchResultModelHelper::~SearchResultModelHelper()
{
    delete d;
}

QAbstractItemModel* SearchResultModelHelper::model() const
{
    return d->model;
}

QItemSelectionModel* SearchResultModelHelper::selectionModel() const
{
    return d->selectionModel;
}

bool SearchResultModelHelper::itemCoordinates(const QModelIndex& index,
                                              GeoCoordinates* const coordinates) const
{
    const SearchResultModel::SearchResultItem item = d->model->resultItem(index);
    *coordinates                                   = item.result.coordinates;

    return true;
}

void SearchResultModelHelper::setVisibility(const bool state)
{
    d->visible = state;
    emit(signalVisibilityChanged());
}

bool SearchResultModelHelper::itemIcon(const QModelIndex& index,
                                       QPoint* const offset,
                                       QSize* const size,
                                       QPixmap* const pixmap,
                                       QUrl* const url) const
{
    return d->model->getMarkerIcon(index, offset, size, pixmap, url);
}

GeoModelHelper::PropertyFlags SearchResultModelHelper::modelFlags() const
{
    return FlagSnaps | (d->visible ? FlagVisible:FlagNull);
}

GeoModelHelper::PropertyFlags SearchResultModelHelper::itemFlags(const QModelIndex& /*index*/) const
{
    return FlagVisible | FlagSnaps;
}

void SearchResultModelHelper::snapItemsTo(const QModelIndex& targetIndex,
                                          const QList<QModelIndex>& snappedIndices)
{
    GPSUndoCommand* const undoCommand              = new GPSUndoCommand();
    SearchResultModel::SearchResultItem targetItem = d->model->resultItem(targetIndex);
    const GeoCoordinates& targetCoordinates        = targetItem.result.coordinates;

    for (int i = 0; i < snappedIndices.count(); ++i)
    {
        const QPersistentModelIndex itemIndex = snappedIndices.at(i);
        GPSImageItem* const item             = d->imageModel->itemFromIndex(itemIndex);

        GPSUndoCommand::UndoInfo undoInfo(itemIndex);
        undoInfo.readOldDataFromItem(item);

        GPSDataContainer newData;
        newData.setCoordinates(targetCoordinates);
        item->setGPSData(newData);

        undoInfo.readNewDataFromItem(item);

        undoCommand->addUndoInfo(undoInfo);
    }

    undoCommand->setText(i18np("1 image snapped to '%2'",
                               "%1 images snapped to '%2'", snappedIndices.count(), targetItem.result.name));

    emit(signalUndoCommand(undoCommand));
}

} // namespace Digikam

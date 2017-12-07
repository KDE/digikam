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

#include "gpsbookmarkmodelhelper.h"

// Qt includes

#include <QStandardItemModel>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "bookmarksmngr.h"
#include "bookmarknode.h"
#include "gpsbookmarkowner.h"
#include "gpsundocommand.h"
#include "gpsimagemodel.h"

namespace Digikam
{

class GPSBookmarkModelHelper::Private
{
public:

    Private()
      : model(0),
        bookmarkManager(0),
        imageModel(0),
        visible(false)
    {
    }

    void addBookmarkGroupToModel(BookmarkNode* const);

public:

    QStandardItemModel* model;
    BookmarksManager*   bookmarkManager;
    GPSImageModel*      imageModel;
    QPixmap             pixmap;
    QUrl                bookmarkIconUrl;
    bool                visible;
};

void GPSBookmarkModelHelper::Private::addBookmarkGroupToModel(BookmarkNode* const node)
{
    if (node->type() != BookmarkNode::Folder)
        return;

    QList<BookmarkNode*> list = node->children();

    if (list.isEmpty())
        return;

    foreach(BookmarkNode* const currentBookmark, list)
    {
        if (currentBookmark)
        {
            if (currentBookmark->type() == BookmarkNode::Folder)
            {
                addBookmarkGroupToModel(currentBookmark);
            }
            else
            {
                bool okay                        = false;
                const GeoCoordinates coordinates =
                    GeoCoordinates::fromGeoUrl(currentBookmark->url, &okay);

                if (okay)
                {
                    QStandardItem* const item = new QStandardItem();
                    item->setData(currentBookmark->title, Qt::DisplayRole);
                    item->setData(QVariant::fromValue(coordinates), GPSBookmarkModelHelper::CoordinatesRole);
                    model->appendRow(item);
                }
            }
        }
    }
}

GPSBookmarkModelHelper::GPSBookmarkModelHelper(BookmarksManager* const bookmarkManager,
                                               GPSImageModel* const imageModel,
                                               QObject* const parent)
    : GeoModelHelper(parent),
      d(new Private())
{
    d->model           = new QStandardItemModel(this);
    d->bookmarkManager = bookmarkManager;
    d->imageModel      = imageModel;
    d->bookmarkIconUrl = QUrl::fromLocalFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                             QString::fromLatin1("digikam/geolocationedit/bookmarks-marker.png")));
    d->pixmap          = QPixmap(d->bookmarkIconUrl.toLocalFile());

    connect(d->bookmarkManager, SIGNAL(entryChanged(BookmarkNode*)),
            this, SLOT(slotUpdateBookmarksModel()));

    connect(d->bookmarkManager, SIGNAL(entryAdded(BookmarkNode*)),
            this, SLOT(slotUpdateBookmarksModel()));

    connect(d->bookmarkManager, SIGNAL(entryRemoved(BookmarkNode*, int, BookmarkNode*)),
            this, SLOT(slotUpdateBookmarksModel()));

    slotUpdateBookmarksModel();
}

GPSBookmarkModelHelper::~GPSBookmarkModelHelper()
{
    delete d;
}

QAbstractItemModel* GPSBookmarkModelHelper::model() const
{
    return d->model;
}

QItemSelectionModel* GPSBookmarkModelHelper::selectionModel() const
{
    return 0;
}

bool GPSBookmarkModelHelper::itemCoordinates(const QModelIndex& index,
                                             GeoCoordinates* const coordinates) const
{
    const GeoCoordinates itemCoordinates = index.data(CoordinatesRole).value<GeoCoordinates>();

    if (coordinates)
    {
        *coordinates = itemCoordinates;
    }

    return itemCoordinates.hasCoordinates();
}

bool GPSBookmarkModelHelper::itemIcon(const QModelIndex& index,
                                      QPoint* const offset,
                                      QSize* const size,
                                      QPixmap* const pixmap,
                                      QUrl* const url) const
{
    Q_UNUSED(index)

    if (offset)
    {
        *offset = QPoint(d->pixmap.width()/2, d->pixmap.height()-1);
    }

    if (url)
    {
        *url = d->bookmarkIconUrl;

        if (size)
        {
            *size = d->pixmap.size();
        }
    }
    else
    {
        *pixmap = d->pixmap;
    }

    return true;
}

void GPSBookmarkModelHelper::slotUpdateBookmarksModel()
{
    d->model->clear();

    // iterate trough all bookmarks
    d->addBookmarkGroupToModel(d->bookmarkManager->bookmarks());
}

void GPSBookmarkModelHelper::setVisible(const bool state)
{
    d->visible = state;
    emit(signalVisibilityChanged());
}

GeoModelHelper::PropertyFlags GPSBookmarkModelHelper::modelFlags() const
{
    return FlagSnaps | (d->visible ? FlagVisible : FlagNull);
}

GeoModelHelper::PropertyFlags GPSBookmarkModelHelper::itemFlags(const QModelIndex&) const
{
    return FlagVisible | FlagSnaps;
}

void GPSBookmarkModelHelper::snapItemsTo(const QModelIndex& targetIndex,
                                         const QList<QModelIndex>& snappedIndices)
{
    GPSUndoCommand* const undoCommand = new GPSUndoCommand();
    GeoCoordinates targetCoordinates;

    if (!itemCoordinates(targetIndex, &targetCoordinates))
        return;

    for (int i = 0; i < snappedIndices.count(); ++i)
    {
        const QPersistentModelIndex itemIndex = snappedIndices.at(i);
        GPSImageItem* const item             = d->imageModel->itemFromIndex(itemIndex);

        GPSDataContainer newData;
        newData.setCoordinates(targetCoordinates);

        GPSUndoCommand::UndoInfo undoInfo(itemIndex);
        undoInfo.readOldDataFromItem(item);

        item->setGPSData(newData);
        undoInfo.readNewDataFromItem(item);

        //undoCommand->addUndoInfo(GPSUndoCommand::UndoInfo(itemIndex, oldData, newData, oldTagList, newTagList));
        undoCommand->addUndoInfo(undoInfo);
    }

    qCDebug(DIGIKAM_GENERAL_LOG)<<targetIndex.data(Qt::DisplayRole).toString();
    undoCommand->setText(i18np("1 image snapped to '%2'",
                               "%1 images snapped to '%2'", snappedIndices.count(), targetIndex.data(Qt::DisplayRole).toString()));

    emit(signalUndoCommand(undoCommand));
}

}  // namespace Digikam

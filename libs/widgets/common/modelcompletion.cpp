/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-13
 * Description : A KCompletion for AbstractAlbumModels
 *
 * Copyright (C) 2007-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "modelcompletion.moc"

// Qt includes

#include <QPointer>

// KDE includes

#include <kglobal.h>
#include <kdebug.h>

namespace Digikam
{

class ModelCompletion::Private
{
public:

    Private() :
        displayRole(Qt::DisplayRole),
        uniqueIdRole(Qt::DisplayRole),
        model(0)
    {
    }

    int                          displayRole;
    int                          uniqueIdRole;

    QPointer<QAbstractItemModel> model;

    /**
     * This map maps model indexes to their current text representation in the
     * completion object. This is needed because if data changes in one index,
     * the old text value is not known anymore, so that it cannot be removed
     * from the completion object.
     */
    //TODO: if we want to use models that return unique strings but not integer, add support
    QMap<int, QString>           idToTextMap;
};

ModelCompletion::ModelCompletion()
    : KCompletion(),
      d(new Private)
{
    setOrder(KCompletion::Sorted);
    setIgnoreCase(true);
}

ModelCompletion::~ModelCompletion()
{
    delete d;
}

void ModelCompletion::setModel(QAbstractItemModel* const model, int uniqueIdRole, int displayRole)
{
    // first release old model
    if (d->model)
    {
        disconnectFromModel(d->model);
        d->idToTextMap.clear();
        clear();
    }

    d->model        = model;
    d->displayRole  = displayRole;
    d->uniqueIdRole = uniqueIdRole;

    // connect to the new model
    if (d->model)
    {
        connectToModel(d->model);

        // do an initial sync wit the new model
        sync(d->model);
    }
}

void ModelCompletion::connectToModel(QAbstractItemModel* const model)
{
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotRowsInserted(QModelIndex,int,int)));

    connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(slotRowsAboutToBeRemoved(QModelIndex,int,int)));

    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotDataChanged(QModelIndex,QModelIndex)));

    connect(model, SIGNAL(modelReset()),
            this, SLOT(slotModelReset()));
}

QAbstractItemModel* ModelCompletion::model() const
{
    return d->model;
}

void ModelCompletion::slotRowsInserted(const QModelIndex& parent, int start, int end)
{
    //kDebug() << "rowInserted in parent " << parent << ", start = " << start
    //         << ", end = " << end;

    for (int i = start; i <= end; ++i)
    {
        // this cannot work if this is called from rowsAboutToBeInserted
        // because then the model doesn't know the index yet. So never do this
        // ;)
        const QModelIndex child = d->model->index(i, 0, parent);

        if (child.isValid())
        {
            sync(d->model, child);
        }
        else
        {
            kError() << "inserted rows are not valid for parent " << parent
                     << parent.data(d->displayRole).toString() << "and child"
                     << child;
        }
    }
}

void ModelCompletion::slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    //kDebug() << "rows of parent " << parent << " removed, start = " << start
    //         << ", end = " << end;
    for (int i = start; i <= end; ++i)
    {
        QModelIndex index = d->model->index(i, 0, parent);

        if (!index.isValid())
        {
            kError() << "Received an invalid index to be removed";
            continue;
        }

        int id = index.data(d->uniqueIdRole).toInt();

        if (d->idToTextMap.contains(id))
        {
            QString itemName = d->idToTextMap[id];
            d->idToTextMap.remove(id);

            // only delete an item in the completion object if there is no other
            // item with the same display name
            if (d->idToTextMap.keys(itemName).empty())
            {
                removeItem(itemName);
            }
        }
        else
        {
            kWarning() << "idToTextMap seems to be out of sync with the model. "
                       << "There is no entry for model index " << index;
        }
    }
}

void ModelCompletion::slotModelReset()
{
    sync(d->model);
}

void ModelCompletion::slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    for (int row = topLeft.row(); row <= bottomRight.row(); ++row)
    {
        if (!d->model->hasIndex(row, topLeft.column(), topLeft.parent()))
        {
            kError() << "Got wrong change event for index with row " << row
                     << ", column " << topLeft.column()
                     << " and parent " << topLeft.parent()
                     << " in model " << d->model << ". Ignoring it.";
            continue;
        }

        QModelIndex index = d->model->index(row, topLeft.column(), topLeft.parent());

        if (!index.isValid())
        {
            kError() << "illegal index in changed data";
            continue;
        }

        int id           = index.data(d->uniqueIdRole).toInt();
        QString itemName = index.data(d->displayRole).toString();

        if (d->idToTextMap.contains(id))
        {
            removeItem(d->idToTextMap.value(id));
        }
        else
        {
            // FIXME normally this should be a bug. Fortunately we can handle
            // it and it is a constant case that this happens because of some
            // kind of race condition between the tree vies and this class.
            // If the model emits the signal, that a new index was added, it may
            // be first processed by the tree view. This updates the item
            // counting based on the expansion state. Unfortunately, this
            // operations needs a data change which is emitted as a dataChanged
            // signal which then will arrive at this class before the original
            // inserted signal arrived at this class.
            //kError() << "idToTextMap did not contain an entry for index "
            //         << index << itemName;
        }

        d->idToTextMap[id] = itemName;
        addItem(itemName);
    }
}

void ModelCompletion::disconnectFromModel(QAbstractItemModel* const model)
{
    disconnect(model);
}

void ModelCompletion::sync(QAbstractItemModel* const model)
{
    //kDebug() << "Starting sync with model " << model
    //         << ", rowCount for parent: " << model->rowCount();

    clear();
    d->idToTextMap.clear();

    for (int i = 0; i < model->rowCount(); ++i)
    {
        const QModelIndex index = model->index(i, 0);
        sync(model, index);
    }
}

void ModelCompletion::sync(QAbstractItemModel* const model, const QModelIndex& index)
{
    QString itemName = index.data(d->displayRole).toString();
    //kDebug() << "sync adding item '" << itemName << "' for index " << index;
    addItem(itemName);
    d->idToTextMap.insert(index.data(d->uniqueIdRole).toInt(), itemName);

    for (int i = 0; i < model->rowCount(index); ++i)
    {
        const QModelIndex child = model->index(i, 0, index);
        sync(model, child);
    }
}

}  // namespace Digikam

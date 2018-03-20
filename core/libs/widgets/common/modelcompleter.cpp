/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-13
 * Description : A QCompleter for AbstractAlbumModels
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "modelcompleter.h"

// Qt includes

#include <QTimer>
#include <QPointer>
#include <QStringListModel>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class ModelCompleter::Private
{
public:

    Private() :
        displayRole(Qt::DisplayRole),
        uniqueIdRole(Qt::DisplayRole),
        delayedModelTimer(0),
        stringModel(0),
        model(0)
    {
    }

    int                          displayRole;
    int                          uniqueIdRole;

    QTimer*                      delayedModelTimer;
    QStringListModel*            stringModel;
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

ModelCompleter::ModelCompleter(QObject* parent)
    : QCompleter(parent),
      d(new Private)
{
    d->stringModel = new QStringListModel(this);
    setModel(d->stringModel);

    setModelSorting(CaseSensitivelySortedModel);
    setCaseSensitivity(Qt::CaseInsensitive);
    setCompletionMode(PopupCompletion);
    setCompletionRole(Qt::DisplayRole);
    setFilterMode(Qt::MatchContains);
    setMaxVisibleItems(10);
    setCompletionColumn(0);

    d->delayedModelTimer = new QTimer(this);
    d->delayedModelTimer->setInterval(250);
    d->delayedModelTimer->setSingleShot(true);

    connect(d->delayedModelTimer, SIGNAL(timeout()),
            this, SLOT(slotDelayedModelTimer()));
}

ModelCompleter::~ModelCompleter()
{
    delete d;
}

void ModelCompleter::setItemModel(QAbstractItemModel* const model, int uniqueIdRole, int displayRole)
{
    // first release old model
    if (d->model)
    {
        disconnect(d->model);
        d->idToTextMap.clear();
        d->stringModel->setStringList(QStringList());
    }

    d->model        = model;
    d->displayRole  = displayRole;
    d->uniqueIdRole = uniqueIdRole;

    // connect to the new model
    if (d->model)
    {
        connect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(slotRowsInserted(QModelIndex,int,int)));

        connect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(slotRowsAboutToBeRemoved(QModelIndex,int,int)));

        connect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(slotDataChanged(QModelIndex,QModelIndex)));

        connect(d->model, SIGNAL(modelReset()),
                this, SLOT(slotModelReset()));

        // do an initial sync wit the new model
        sync(d->model);
    }
}

QAbstractItemModel* ModelCompleter::itemModel() const
{
    return d->model;
}

void ModelCompleter::addItem(const QString& item)
{
    QStringList stringList = d->stringModel->stringList();
    d->stringModel->setStringList(stringList << item);
    d->stringModel->sort(0);
}

QStringList ModelCompleter::items() const
{
    return d->stringModel->stringList();
}

void ModelCompleter::slotDelayedModelTimer()
{
    QStringList stringList = d->idToTextMap.values();
    stringList.removeDuplicates();
    stringList.sort();

    d->stringModel->setStringList(stringList);
}

void ModelCompleter::slotRowsInserted(const QModelIndex& parent, int start, int end)
{
    //qCDebug(DIGIKAM_WIDGETS_LOG) << "rowInserted in parent " << parent << ", start = " << start
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
            qCDebug(DIGIKAM_WIDGETS_LOG) << "inserted rows are not valid for parent " << parent
                     << parent.data(d->displayRole).toString() << "and child"
                     << child;
        }
    }

    d->delayedModelTimer->start();
}

void ModelCompleter::slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    //qCDebug(DIGIKAM_WIDGETS_LOG) << "rows of parent " << parent << " removed, start = " << start
    //         << ", end = " << end;
    for (int i = start; i <= end; ++i)
    {
        QModelIndex index = d->model->index(i, 0, parent);

        if (!index.isValid())
        {
            qCDebug(DIGIKAM_WIDGETS_LOG) << "Received an invalid index to be removed";
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
                d->delayedModelTimer->start();
            }
        }
        else
        {
            qCWarning(DIGIKAM_WIDGETS_LOG) << "idToTextMap seems to be out of sync with the model. "
                       << "There is no entry for model index " << index;
        }
    }
}

void ModelCompleter::slotModelReset()
{
    sync(d->model);
}

void ModelCompleter::slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    for (int row = topLeft.row(); row <= bottomRight.row(); ++row)
    {
        if (!d->model->hasIndex(row, topLeft.column(), topLeft.parent()))
        {
            qCDebug(DIGIKAM_WIDGETS_LOG) << "Got wrong change event for index with row " << row
                     << ", column " << topLeft.column()
                     << " and parent " << topLeft.parent()
                     << " in model " << d->model << ". Ignoring it.";
            continue;
        }

        QModelIndex index  = d->model->index(row, topLeft.column(), topLeft.parent());

        if (!index.isValid())
        {
            qCDebug(DIGIKAM_WIDGETS_LOG) << "illegal index in changed data";
            continue;
        }

        int id             = index.data(d->uniqueIdRole).toInt();
        QString itemName   = index.data(d->displayRole).toString();
        d->idToTextMap[id] = itemName;

        d->delayedModelTimer->start();
    }
}

void ModelCompleter::sync(QAbstractItemModel* const model)
{
    //qCDebug(DIGIKAM_WIDGETS_LOG) << "Starting sync with model " << model
    //         << ", rowCount for parent: " << model->rowCount();

    d->idToTextMap.clear();

    for (int i = 0; i < model->rowCount(); ++i)
    {
        const QModelIndex index = model->index(i, 0);
        sync(model, index);
    }

    d->delayedModelTimer->start();
}

void ModelCompleter::sync(QAbstractItemModel* const model, const QModelIndex& index)
{
    QString itemName = index.data(d->displayRole).toString();
    //qCDebug(DIGIKAM_WIDGETS_LOG) << "sync adding item '" << itemName << "' for index " << index;
    d->idToTextMap.insert(index.data(d->uniqueIdRole).toInt(), itemName);

    for (int i = 0; i < model->rowCount(index); ++i)
    {
        const QModelIndex child = model->index(i, 0, index);
        sync(model, child);
    }
}

}  // namespace Digikam

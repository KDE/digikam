/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-13
 * Description : A QCompleter for AbstractAlbumModels
 *
 * Copyright (C) 2007-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
        stringModel(0),
        model(0)
    {
    }

    int                          displayRole;
    int                          uniqueIdRole;

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
    QCompleter::setModel(d->stringModel);

    setModelSorting(CaseSensitivelySortedModel);
    setCaseSensitivity(Qt::CaseInsensitive);
    setCompletionMode(PopupCompletion);
    setCompletionRole(d->displayRole);
    setMaxVisibleItems(10);
    setCompletionColumn(0);
}

ModelCompleter::~ModelCompleter()
{
    delete d;
}

void ModelCompleter::setModel(QAbstractItemModel* const model, int uniqueIdRole, int displayRole)
{
    // first release old model
    if (d->model)
    {
        disconnectFromModel(d->model);
        d->idToTextMap.clear();
        d->stringModel->setStringList(QStringList());
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

void ModelCompleter::connectToModel(QAbstractItemModel* const model)
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

QAbstractItemModel* ModelCompleter::model() const
{
    return d->model;
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
                QStringList stringList = d->idToTextMap.values();
                stringList.removeDuplicates();

                d->stringModel->setStringList(stringList);
                d->stringModel->sort(0);
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

        QModelIndex index = d->model->index(row, topLeft.column(), topLeft.parent());

        if (!index.isValid())
        {
            qCDebug(DIGIKAM_WIDGETS_LOG) << "illegal index in changed data";
            continue;
        }

        int id           = index.data(d->uniqueIdRole).toInt();
        QString itemName = index.data(d->displayRole).toString();

        d->idToTextMap[id] = itemName;

        QStringList stringList = d->idToTextMap.values();
        stringList.removeDuplicates();

        d->stringModel->setStringList(stringList);
        d->stringModel->sort(0);
    }
}

void ModelCompleter::disconnectFromModel(QAbstractItemModel* const model)
{
    disconnect(model);
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

    QStringList stringList = d->idToTextMap.values();
    stringList.removeDuplicates();

    d->stringModel->setStringList(stringList);
    d->stringModel->sort(0);
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

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-18
 * Description : Sync QItemSelectionModel of ImageFilterModel and TableViewModel
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "tableview_selection_model_syncer.moc"

// KDE includes

#include <kdebug.h>

// local includes

#include "imagefiltermodel.h"
#include "tableview_model.h"
#include "tableview_shared.h"

namespace Digikam
{

class TableViewSelectionModelSyncer::Private
{
public:
    Private()
      : syncing(false)
    {
    }

    bool syncing;
};

TableViewSelectionModelSyncer::TableViewSelectionModelSyncer(TableViewShared* const sharedObject, QObject* const parent)
  : QObject(parent),
    d(new Private()),
    s(sharedObject)
{
    connect(s->imageFilterSelectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotSourceCurrentChanged(QModelIndex,QModelIndex)));
    connect(s->imageFilterSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotSourceSelectionChanged(QItemSelection,QItemSelection)));
    connect(s->tableViewSelectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotTargetCurrentChanged(QModelIndex,QModelIndex)));
    connect(s->tableViewSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotTargetSelectionChanged(QItemSelection,QItemSelection)));
    connect(s->tableViewModel, SIGNAL(columnsInserted(QModelIndex,int,int)),
            this, SLOT(slotTargetColumnsInserted(QModelIndex,int,int)));

    /// @todo This is necessary to re-sync the selection when tags are added to images.
    ///       Check whether both are necessary or whether we need more.
    connect(s->imageFilterModel, SIGNAL(layoutChanged()),
            this, SLOT(slotSourceModelReset()));
    connect(s->imageFilterModel, SIGNAL(modelReset()),
            this, SLOT(slotSourceModelReset()));

    doInitialSync();
}

TableViewSelectionModelSyncer::~TableViewSelectionModelSyncer()
{

}

QModelIndex TableViewSelectionModelSyncer::toSource(const QModelIndex& tableViewIndex) const
{
    return s->tableViewModel->toImageFilterModelIndex(tableViewIndex);
}

QModelIndex TableViewSelectionModelSyncer::toTarget(const QModelIndex& sourceIndex) const
{
    return s->tableViewModel->fromImageFilterModelIndex(sourceIndex);
}

int TableViewSelectionModelSyncer::targetModelColumnCount() const
{
    return s->tableViewModel->columnCount(QModelIndex());
}

QItemSelection TableViewSelectionModelSyncer::targetIndexToRowItemSelection(const QModelIndex targetIndex) const
{
    const int row = targetIndex.row();

    const QModelIndex topLeft = s->tableViewModel->index(row, 0, targetIndex.parent());
    const QModelIndex bottomRight = s->tableViewModel->index(row, targetModelColumnCount()-1, targetIndex.parent());
    const QItemSelection mySelection(topLeft, bottomRight);

    return mySelection;
}

void TableViewSelectionModelSyncer::doInitialSync()
{
    d->syncing = true;

    s->tableViewSelectionModel->clearSelection();

    const QItemSelection sourceSelection = s->imageFilterSelectionModel->selection();
    const QItemSelection targetSelection = itemSelectionToTarget(sourceSelection);
    s->tableViewSelectionModel->select(targetSelection, QItemSelectionModel::Select);

    const QModelIndex targetIndexCurrent = toTarget(s->imageFilterSelectionModel->currentIndex());
    s->tableViewSelectionModel->setCurrentIndex(targetIndexCurrent, QItemSelectionModel::NoUpdate);

    d->syncing = false;
}

void TableViewSelectionModelSyncer::slotSourceCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous)

    if (d->syncing)
    {
        return;
    }
    d->syncing = true;

    // we have to select the whole row of the target index
    const QModelIndex targetIndexCurrent = toTarget(current);

    s->tableViewSelectionModel->setCurrentIndex(targetIndexCurrent, QItemSelectionModel::Select);

    d->syncing = false;
}

QItemSelection TableViewSelectionModelSyncer::itemSelectionToSource(const QItemSelection& selection) const
{
    QItemSelection sourceSelection;
    Q_FOREACH(const QItemSelectionRange& range, selection)
    {
        const int firstRow = range.top();
        const int lastRow = range.bottom();
        
        for (int row = firstRow; row<=lastRow; ++row)
        {
            const QModelIndex tableViewIndex = s->tableViewModel->index(row, 0, range.parent());
            const QModelIndex sourceIndex = s->tableViewModel->toImageFilterModelIndex(tableViewIndex);
            
            if (sourceIndex.isValid())
            {
                sourceSelection.select(sourceIndex, sourceIndex);
            }
        }
    }

    return sourceSelection;
}

QItemSelection TableViewSelectionModelSyncer::itemSelectionToTarget(const QItemSelection& selection) const
{
    const int targetColumnCount = targetModelColumnCount();

    QItemSelection targetSelection;
    Q_FOREACH(const QItemSelectionRange& range, selection)
    {
        const int firstRow = range.top();
        const int lastRow = range.bottom();

        for (int row = firstRow; row<=lastRow; ++row)
        {
            
            const QModelIndex sourceIndex = s->imageFilterModel->index(row, 0, range.parent());
            const QModelIndex tableViewIndexTopLeft = s->tableViewModel->fromImageFilterModelIndex(sourceIndex);
            const QModelIndex tableViewIndexBottomRight = s->tableViewModel->index(
                    tableViewIndexTopLeft.row(),
                    targetColumnCount-1,
                    tableViewIndexTopLeft.parent()
                );

            targetSelection.select(tableViewIndexTopLeft, tableViewIndexBottomRight);
        }
    }

    return targetSelection;
}


void TableViewSelectionModelSyncer::slotSourceSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (d->syncing)
    {
        return;
    }
    d->syncing = true;

    const QItemSelection targetSelection = itemSelectionToTarget(selected);
    s->tableViewSelectionModel->select(targetSelection, QItemSelectionModel::Select);

    const QItemSelection targetDeselection = itemSelectionToTarget(deselected);
    s->tableViewSelectionModel->select(targetDeselection, QItemSelectionModel::Deselect);

    d->syncing = false;
}

void TableViewSelectionModelSyncer::slotTargetCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous)

    if (d->syncing)
    {
        return;
    }
    d->syncing = true;

    const QModelIndex sourceIndexCurrent = toSource(current);
    s->imageFilterSelectionModel->setCurrentIndex(sourceIndexCurrent, QItemSelectionModel::Select);

    d->syncing = false;
}

void TableViewSelectionModelSyncer::slotTargetSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (d->syncing)
    {
        return;
    }
    d->syncing = true;

    const QItemSelection sourceSelection = itemSelectionToSource(selected);
    s->imageFilterSelectionModel->select(sourceSelection, QItemSelectionModel::Select);

    const QItemSelection sourceDeselection = itemSelectionToSource(deselected);
    s->imageFilterSelectionModel->select(sourceDeselection, QItemSelectionModel::Deselect);

    d->syncing = false;
}

void TableViewSelectionModelSyncer::slotSourceModelReset()
{
    doInitialSync();
}

void TableViewSelectionModelSyncer::slotTargetColumnsInserted(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)

    if (d->syncing)
    {
        return;
    }

    // New columns were inserted. We have to make sure that all selected rows include the new columns.
    // We just re-perform the initial synchronization.
    /// @todo There may be more efficient ways.
    doInitialSync();
}

} /* namespace Digikam */


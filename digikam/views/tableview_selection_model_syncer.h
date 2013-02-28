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

#ifndef TABLEVIEW_SELECTION_MODEL_SYNCER_H
#define TABLEVIEW_SELECTION_MODEL_SYNCER_H

// Qt includes

#include <QObject>
#include <QItemSelectionModel>

namespace Digikam
{

class TableViewShared;

class TableViewSelectionModelSyncer : public QObject
{
    Q_OBJECT

public:

    TableViewSelectionModelSyncer(TableViewShared* const sharedObject, QObject* const parent = 0);
    virtual ~TableViewSelectionModelSyncer();

    void doInitialSync();
    QModelIndex toSource(const QModelIndex& targetIndex) const;
    QModelIndex toTarget(const QModelIndex& sourceIndex) const;
    QItemSelection itemSelectionToSource(const QItemSelection& selection) const;
    QItemSelection itemSelectionToTarget(const QItemSelection& selection) const;
    int targetModelColumnCount() const;
    QItemSelection targetIndexToRowItemSelection(const QModelIndex targetIndex) const;

private Q_SLOTS:

    void slotSourceModelReset();
    void slotSourceCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
//     void slotSourceCurrentColumnChanged(const QModelIndex& current, const QModelIndex& previous);
//     void slotSourceCurrentRowChanged(const QModelIndex& current, const QModelIndex& previous);
    void slotSourceSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void slotTargetCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
//     void slotTargetCurrentColumnChanged(const QModelIndex& current, const QModelIndex& previous);
//     void slotTargetCurrentRowChanged(const QModelIndex& current, const QModelIndex& previous);
    void slotTargetSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:

    class Private;
    const QScopedPointer<Private> d;
    TableViewShared* const s;
};

} /* namespace Digikam */

#endif // TABLEVIEW_SELECTION_MODEL_SYNCER_H


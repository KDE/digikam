/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-13
 * Description : tabbed queue items list.
 *
 * Copyright (C) 2009-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef QUEUEPOOL_H
#define QUEUEPOOL_H

// Qt includes

#include <QMap>
#include <QWidget>
#include <QDragMoveEvent>
#include <QDropEvent>

// KDE includes

#include <ktabwidget.h>

// Local includes

#include "iteminfoset.h"

namespace Digikam
{

class AssignedBatchTools;
class QueueSettings;
class QueueListView;

class QueuePool : public KTabWidget
{
    Q_OBJECT

public:

    QueuePool(QWidget* const parent);
    ~QueuePool();

    QueueListView*     currentQueue()                    const;
    QueueListView*     findQueueById(int index)          const;
    QueueListView*     findQueueByItemId(qlonglong id)   const;
    QueuePoolItemsList totalPendingItemsList()           const;
    int                totalPendingItems()               const;
    int                totalPendingTasks()               const;
    QMap<int, QString> queuesMap()                       const;
    QString            queueTitle(int index)             const;
    bool               customRenamingRulesAreValid()     const;
    bool               assignedBatchToolsListsAreValid() const;

    void setEnableToolTips(bool b);

    void setBusy(bool b);

    void animProgress(qlonglong id);

Q_SIGNALS:

    void signalItemSelectionChanged();
    void signalQueuePoolChanged();
    void signalQueueSelected(int id, const QueueSettings&, const AssignedBatchTools&);
    void signalQueueContentsChanged();

public Q_SLOTS:

    void removeTab(int index);

    void slotAddQueue();
    void slotRemoveCurrentQueue();
    void slotClearList();
    void slotRemoveSelectedItems();
    void slotRemoveItemsDone();
    void slotAddItems(const ImageInfoList&, int queueId);
    void slotAssignedToolsChanged(const AssignedBatchTools&);
    void slotSettingsChanged(const QueueSettings&);

protected:

    virtual void keyPressEvent(QKeyEvent* event);

private Q_SLOTS:

    void slotFileChanged(const QString&);
    void slotQueueSelected(int);
    void slotCloseQueueRequest(QWidget*);
    void slotTestCanDecode(const QDragMoveEvent*, bool&);
};

}  // namespace Digikam

#endif // QUEUEPOOL_H

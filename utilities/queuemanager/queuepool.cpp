/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-13
 * Description : tabbed queue items list.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "queuepool.h"
#include "queuepool.moc"

// KDE includes.

#include <kmessagebox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes.

#include "queuelist.h"

namespace Digikam
{

class QueuePoolPriv
{

public:

    QueuePoolPriv()
    {
    }

};

QueuePool::QueuePool(QWidget *parent)
        : KTabWidget(parent), d(new QueuePoolPriv)
{
    setTabBarHidden(false);
    setCloseButtonEnabled(true);
    slotAddQueue();

    connect(this, SIGNAL(currentChanged(int)),
            this, SLOT(slotQueuePoolChanged(int)));

    connect(this, SIGNAL(closeRequest(QWidget*)),
            this, SLOT(slotCloseQueueRequest(QWidget*)));
}

QueuePool::~QueuePool()
{
    delete d;
}

QueueListView* QueuePool::currentQueue() const
{
    return (dynamic_cast<QueueListView*>(currentWidget()));
}

void QueuePool::slotAddQueue()
{
    QueueListView* queue = new QueueListView(this);
    int index = addTab(queue, SmallIcon("vcs_diff"), QString("#%1").arg(count()+1));

    connect(queue, SIGNAL(signalImageListChanged()),
            this, SIGNAL(signalImageListChanged()));

    connect(queue, SIGNAL(signalItemSelected(const AssignedBatchTools&)),
            this, SIGNAL(signalItemSelected(const AssignedBatchTools&)));

    emit signalQueuePoolChanged();

    setCurrentIndex(index);
}

QueuePoolItemsList QueuePool::totalPendingItemsList()
{
    QueuePoolItemsList qpool;
    for (int i = 0; i < count(); i++)
    {
        QueueListView* queue = dynamic_cast<QueueListView*>(widget(i));
        ImageInfoList list   = queue->pendingItemsList();
        for (ImageInfoList::iterator it = list.begin() ; it != list.end() ; ++it)
        {
            ImageInfo info = *it;
            ItemInfoSet set(i, info);
            qpool.append(set);
        }
    }
    return qpool;
}

int QueuePool::totalPendingItems()
{
    int items = 0;
    for (int i = 0; i < count(); i++)
    {
        QueueListView* queue = dynamic_cast<QueueListView*>(widget(i));
        items += queue->pendingItemsCount();
    }
    return items;
}

int QueuePool::totalPendingTasks()
{
    int tasks = 0;
    for (int i = 0; i < count(); i++)
    {
        QueueListView* queue = dynamic_cast<QueueListView*>(widget(i));
        tasks += queue->pendingTasksCount();
    }
    return tasks;
}

void QueuePool::slotRemoveCurrentQueue()
{
    QueueListView* queue = currentQueue();
    if (!queue) return;

    removeTab(indexOf(queue));
    if (count() == 0)
        slotAddQueue();

    emit signalQueuePoolChanged();
}

void QueuePool::slotClearList()
{
    QueueListView* queue = currentQueue();
    if (queue) queue->slotClearList();
}

void QueuePool::slotRemoveSelectedItems()
{
    QueueListView* queue = currentQueue();
    if (queue) queue->slotRemoveSelectedItems();
}

void QueuePool::slotRemoveItemsDone()
{
    QueueListView* queue = currentQueue();
    if (queue) queue->slotRemoveItemsDone();
}

void QueuePool::slotAddItems(const ImageInfoList& list, const ImageInfo &current)
{
    QueueListView* queue = currentQueue();
    if (queue) queue->slotAddItems(list, current);
}

void QueuePool::slotAssignedToolsChanged(const AssignedBatchTools& tools4Item)
{
    QueueListView* queue = currentQueue();
    if (queue) queue->slotAssignedToolsChanged(tools4Item);
}

void QueuePool::slotQueuePoolChanged(int)
{
    QueueListView* queue = currentQueue();
    if (queue) queue->slotItemSelectionChanged();
}

void QueuePool::slotCloseQueueRequest(QWidget* w)
{
    removeTab(indexOf(w));
    if (count() == 0)
        slotAddQueue();

    emit signalQueuePoolChanged();
}

void QueuePool::removeTab(int index)
{
    QueueListView* queue = dynamic_cast<QueueListView*>(widget(index));
    int count = queue->pendingItemsCount();
    if (count > 0)
    {
        int ret = KMessageBox::questionYesNo(this, 
                  i18np("It still 1 unprocessed item in \"%2\". Do you want to close this queue?", 
                        "There still %1 unprocessed items in \"%2\". Do you want to close this queue?", 
                  count, tabText(index)));
        if (ret == KMessageBox::No)
            return;
    }

    KTabWidget::removeTab(index);
}

}  // namespace Digikam

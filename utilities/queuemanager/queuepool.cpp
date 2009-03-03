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

#include "ddragobjects.h"
#include "queuelist.h"

namespace Digikam
{

QueuePool::QueuePool(QWidget *parent)
         : KTabWidget(parent)
{
    setTabBarHidden(false);
    setCloseButtonEnabled(true);
    slotAddQueue();

    connect(this, SIGNAL(currentChanged(int)),
            this, SLOT(slotQueueSelected(int)));

    connect(this, SIGNAL(closeRequest(QWidget*)),
            this, SLOT(slotCloseQueueRequest(QWidget*)));

    connect(this, SIGNAL(testCanDecode(const QDragMoveEvent*, bool&)),
            this, SLOT(slotTestCanDecode(const QDragMoveEvent*, bool&)));
}

QueuePool::~QueuePool()
{
}

QueueListView* QueuePool::currentQueue() const
{
    return (dynamic_cast<QueueListView*>(currentWidget()));
}

QueueListView* QueuePool::findQueueById(int index) const
{
    return (dynamic_cast<QueueListView*>(widget(index)));
}

QMap<int, QString> QueuePool::queuesMap() const
{
    QMap<int, QString> map;
    for (int i = 0; i < count(); i++)
        map.insert(i, tabText(i));
    return map;
}

void QueuePool::slotAddQueue()
{
    QueueListView* queue = new QueueListView(this);
    int index            = addTab(queue, SmallIcon("vcs_diff"), QString("#%1").arg(count()+1));

    connect(queue, SIGNAL(signalQueueContentsChanged()),
            this, SIGNAL(signalQueueContentsChanged()));

    connect(queue, SIGNAL(itemSelectionChanged()),
            this, SIGNAL(signalItemSelectionChanged()));

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
        items                += queue->pendingItemsCount();
    }
    return items;
}

int QueuePool::totalPendingTasks()
{
    int tasks = 0;
    for (int i = 0; i < count(); i++)
    {
        QueueListView* queue = dynamic_cast<QueueListView*>(widget(i));
        tasks                += queue->pendingTasksCount();
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

void QueuePool::slotAddItems(const ImageInfoList& list, int queueId)
{
    QueueListView* queue = findQueueById(queueId);
    if (queue) queue->slotAddItems(list);
}

void QueuePool::slotAssignedToolsChanged(const AssignedBatchTools& tools4Item)
{
    QueueListView* queue = currentQueue();
    if (queue) queue->slotAssignedToolsChanged(tools4Item);
}

void QueuePool::slotQueueSelected(int index)
{
    QueueListView* queue = dynamic_cast<QueueListView*>(widget(index));
    if (queue)
    {
        emit signalItemSelectionChanged();
        emit signalQueueSelected(index, queue->settings(), queue->assignedTools());
    }
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
    int count            = queue->pendingItemsCount();
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

void QueuePool::slotTestCanDecode(const QDragMoveEvent* e, bool& accept)
{
    int        albumID;
    QList<int> albumIDs;
    QList<int> imageIDs;
    KUrl::List urls;
    KUrl::List kioURLs;

    if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs) ||
        DAlbumDrag::decode(e->mimeData(), urls, albumID) ||
        DTagDrag::canDecode(e->mimeData()))
    {
        accept = true;
        return;
    }
    accept = false;
}

void QueuePool::slotSettingsChanged(const QueueSettings& settings)
{
    QueueListView* queue = currentQueue();
    if (queue) queue->setSettings(settings);
}

void QueuePool::setEnableToolTips(bool b)
{
    for (int i = 0; i < count(); i++)
    {
        QueueListView* queue = dynamic_cast<QueueListView*>(widget(i));
        if (queue) queue->setEnableToolTips(b);
    }
}

}  // namespace Digikam

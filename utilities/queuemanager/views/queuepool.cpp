/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-13
 * Description : tabbed queue items list.
 *
 * Copyright (C) 2009-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "queuepool.moc"

// Qt includes

#include <QTabBar>

// KDE includes

#include <kapplication.h>
#include <kdeversion.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

// Local includes

#include "applicationsettings.h"
#include "iccsettings.h"
#include "metadatasettings.h"
#include "ddragobjects.h"
#include "queuelist.h"
#include "workflowmanager.h"
#include "workflowdlg.h"
#include "queuesettings.h"
#include "loadingcacheinterface.h"

namespace Digikam
{

QueuePool::QueuePool(QWidget* const parent)
    : KTabWidget(parent)
{
    setTabBarHidden(false);
#if KDE_IS_VERSION(4,3,0)
    setTabsClosable(false);
#else
    setCloseButtonEnabled(false);
#endif
    slotAddQueue();

    connect(this, SIGNAL(currentChanged(int)),
            this, SLOT(slotQueueSelected(int)));

    connect(this, SIGNAL(closeRequest(QWidget*)),
            this, SLOT(slotCloseQueueRequest(QWidget*)));

    connect(this, SIGNAL(testCanDecode(const QDragMoveEvent*,bool&)),
            this, SLOT(slotTestCanDecode(const QDragMoveEvent*,bool&)));

    // -- FileWatch connections ------------------------------

    LoadingCacheInterface::connectToSignalFileChanged(this, SLOT(slotFileChanged(QString)));
}

QueuePool::~QueuePool()
{
}

void QueuePool::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete)
    {
        slotRemoveSelectedItems();
    }
    else
    {
        KTabWidget::keyPressEvent(event);
    }
}

void QueuePool::setBusy(bool b)
{
    tabBar()->setEnabled(!b);

    for (int i = 0; i < count(); ++i)
    {
        QueueListView* const queue = dynamic_cast<QueueListView*>(widget(i));

        if (queue)
            queue->viewport()->setEnabled(!b);
    }
}

QueueListView* QueuePool::currentQueue() const
{
    return (dynamic_cast<QueueListView*>(currentWidget()));
}

QueueListView* QueuePool::findQueueByItemId(qlonglong id) const
{
    for (int i = 0; i < count(); ++i)
    {
        QueueListView* const queue = dynamic_cast<QueueListView*>(widget(i));

        if (queue && queue->findItemById(id))
        {
            return queue;
        }
    }

    return 0;
}

void QueuePool::setItemBusy(qlonglong id)
{
    QueueListView* const queue = findQueueByItemId(id);

    if (queue)
        queue->setItemBusy(id);
}

QueueListView* QueuePool::findQueueByIndex(int index) const
{
    return (dynamic_cast<QueueListView*>(widget(index)));
}

QMap<int, QString> QueuePool::queuesMap() const
{
    QMap<int, QString> map;

    for (int i = 0; i < count(); ++i)
    {
        map.insert(i, queueTitle(i));
    }

    return map;
}

QString QueuePool::queueTitle(int index) const
{
    // NOTE: clean up tab title. With KTabWidget, it sound like mistake is added, as '&' and space.
    // NOTE update, & is an usability helper to allow keyboard access -teemu
    return (tabText(index).remove('&').remove(' '));
}

void QueuePool::slotAddQueue()
{
    QueueListView* const queue = new QueueListView(this);

    if (!queue)
        return;

    int index = addTab(queue, SmallIcon("bqm-diff"), QString("#%1").arg(count() + 1));

    connect(queue, SIGNAL(signalQueueContentsChanged()),
            this, SIGNAL(signalQueueContentsChanged()));

    connect(queue, SIGNAL(itemSelectionChanged()),
            this, SIGNAL(signalItemSelectionChanged()));

    emit signalQueuePoolChanged();

    setCurrentIndex(index);
}

QueuePoolItemsList QueuePool::queueItemsList(int index) const
{
    QueuePoolItemsList qpool;

    QueueListView* const queue = dynamic_cast<QueueListView*>(widget(index));

    if (queue)
    {
        ImageInfoList list = queue->pendingItemsList();

        for (ImageInfoList::const_iterator it = list.constBegin() ; it != list.constEnd() ; ++it)
        {
            ImageInfo info = *it;
            ItemInfoSet set(index, info);
            qpool.append(set);
        }
    }

    return qpool;
}

int QueuePool::totalPendingItems() const
{
    int items = 0;

    for (int i = 0; i < count(); ++i)
    {
        QueueListView* const queue = dynamic_cast<QueueListView*>(widget(i));

        if (queue)
            items += queue->pendingItemsCount();
    }

    return items;
}

int QueuePool::totalPendingTasks() const
{
    int tasks = 0;

    for (int i = 0; i < count(); ++i)
    {
        QueueListView* const queue = dynamic_cast<QueueListView*>(widget(i));

        if (queue)
            tasks += queue->pendingTasksCount();
    }

    return tasks;
}

void QueuePool::slotRemoveCurrentQueue()
{
    QueueListView* const queue = currentQueue();

    if (!queue)
    {
        return;
    }

    removeTab(indexOf(queue));

    if (count() == 0)
    {
        slotAddQueue();
    }
    else
    {
        for (int i = 0; i < count(); ++i)
        {
            setTabText(i, QString("#%1").arg(i + 1));
        }
    }

    emit signalQueuePoolChanged();
}

bool QueuePool::saveWorkflow() const
{
    QueueListView* const queue = currentQueue();

    if (queue)
    {
        WorkflowManager* const mngr = WorkflowManager::instance();
        Workflow wf;
        wf.qSettings = queue->settings();
        wf.aTools    = queue->assignedTools().m_toolsList;

        if (WorkflowDlg::createNew(wf))
        {
            mngr->insert(wf);
            mngr->save();
            return true;
        }
    }

    return false;
}

void QueuePool::slotClearList()
{
    QueueListView* const queue = currentQueue();

    if (queue)
    {
        queue->slotClearList();
    }
}

void QueuePool::slotRemoveSelectedItems()
{
    QueueListView* const queue = currentQueue();

    if (queue)
    {
        queue->slotRemoveSelectedItems();
    }
}

void QueuePool::slotRemoveItemsDone()
{
    QueueListView* const queue = currentQueue();

    if (queue)
    {
        queue->slotRemoveItemsDone();
    }
}

void QueuePool::slotAddItems(const ImageInfoList& list, int queueId)
{
    QueueListView* const queue = findQueueByIndex(queueId);

    if (queue)
    {
        queue->slotAddItems(list);
    }
}

void QueuePool::slotAssignedToolsChanged(const AssignedBatchTools& tools4Item)
{
    QueueListView* const queue = currentQueue();

    if (queue)
    {
        queue->slotAssignedToolsChanged(tools4Item);
    }
}

void QueuePool::slotQueueSelected(int index)
{
    QueueListView* const queue = dynamic_cast<QueueListView*>(widget(index));

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
    {
        slotAddQueue();
    }

    emit signalQueuePoolChanged();
}

void QueuePool::removeTab(int index)
{
    QueueListView* const queue = dynamic_cast<QueueListView*>(widget(index));

    if (!queue)
        return;

    int count = queue->pendingItemsCount();

    if (count > 0)
    {
        int ret = KMessageBox::questionYesNo(this,
                                             i18np("There is still 1 unprocessed item in \"%2\". Do you want to close this queue?",
                                                   "There are still %1 unprocessed items in \"%2\". Do you want to close this queue?",
                                                   count, queueTitle(index)));

        if (ret == KMessageBox::No)
        {
            return;
        }
    }

    KTabWidget::removeTab(index);
}

void QueuePool::slotTestCanDecode(const QDragMoveEvent* e, bool& accept)
{
    int              albumID;
    QList<int>       albumIDs;
    QList<qlonglong> imageIDs;
    KUrl::List       urls;
    KUrl::List       kioURLs;

    if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs) ||
        DAlbumDrag::decode(e->mimeData(), urls, albumID)                    ||
        DTagListDrag::canDecode(e->mimeData()))
    {
        accept = true;
        return;
    }

    accept = false;
}

void QueuePool::slotSettingsChanged(const QueueSettings& settings)
{
    QueueListView* const queue = currentQueue();

    if (queue)
    {
        queue->setSettings(settings);
    }
}

bool QueuePool::customRenamingRulesAreValid() const
{
    QStringList list;

    for (int i = 0; i < count(); ++i)
    {
        QueueListView* const queue = dynamic_cast<QueueListView*>(widget(i));

        if (queue)
        {
            if (queue->settings().renamingRule == QueueSettings::CUSTOMIZE &&
                queue->settings().renamingParser.isEmpty())
            {
                list.append(queueTitle(i));
            }
        }
    }

    if (!list.isEmpty())
    {
        KMessageBox::errorList(kapp->activeWindow(),
                               i18n("Custom renaming rules are invalid for Queues listed below. "
                                    "Please fix them."), list);
        return false;
    }

    return true;
}

bool QueuePool::assignedBatchToolsListsAreValid() const
{
    QStringList list;

    for (int i = 0; i < count(); ++i)
    {
        QueueListView* const queue = dynamic_cast<QueueListView*>(widget(i));

        if (queue)
        {
            if (queue->assignedTools().m_toolsList.isEmpty())
            {
                list.append(queueTitle(i));
            }
        }
    }

    if (!list.isEmpty())
    {
        KMessageBox::errorList(kapp->activeWindow(),
                               i18n("Assigned batch tools list is empty for Queues listed below. "
                                    "Please assign tools."), list);
        return false;
    }

    return true;
}

void QueuePool::slotFileChanged(const QString& filePath)
{
    for (int i = 0; i < count(); ++i)
    {
        QueueListView* const queue = dynamic_cast<QueueListView*>(widget(i));

        if (queue)
        {
            queue->reloadThumbs(KUrl::fromPath(filePath));
        }
    }
}

void QueuePool::applySettings()
{
    for (int i = 0; i < count(); ++i)
    {
        QueueListView* const queue = dynamic_cast<QueueListView*>(widget(i));

        if (queue)
        {
            // Show/hide tool-tips settings.
            queue->setEnableToolTips(ApplicationSettings::instance()->getShowToolTips());

            // Reset Exif Orientation settings.
            QueueSettings prm = queue->settings();
            prm.exifSetOrientation = MetadataSettings::instance()->settings().exifRotate;

            // Apply Color Management rules to RAW images decoding settings

            // If digiKam Color Management is enable, no need to correct color of decoded RAW image,
            // else, sRGB color workspace will be used.

            ICCSettingsContainer ICCSettings = IccSettings::instance()->settings();

            if (ICCSettings.enableCM)
            {
                if (ICCSettings.defaultUncalibratedBehavior & ICCSettingsContainer::AutomaticColors)
                {
                    prm.rawDecodingSettings.outputColorSpace = RawDecodingSettings::CUSTOMOUTPUTCS;
                    prm.rawDecodingSettings.outputProfile    = ICCSettings.workspaceProfile;
                }
                else
                {
                    prm.rawDecodingSettings.outputColorSpace = RawDecodingSettings::RAWCOLOR;
                }
            }
            else
            {
                prm.rawDecodingSettings.outputColorSpace = RawDecodingSettings::SRGB;
            }

            queue->setSettings(prm);
        }
    }
}

}  // namespace Digikam

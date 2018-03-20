/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-10-16
 * Description : history updater thread for importui
 *
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "camerahistoryupdater.h"

// Qt includes

#include <QList>
#include <QMutex>
#include <QVariant>
#include <QWaitCondition>
#include <QWidget>

// Local includes

#include "digikam_debug.h"
#include "coredbdownloadhistory.h"

namespace Digikam
{

class CameraHistoryUpdater::Private
{

public:

    typedef QList<CHUpdateItem> CHUpdateItemsList;

public:

    Private() :
        canceled(false),
        running(false)
    {
    }

    bool              canceled;
    bool              running;

    QMutex            mutex;
    QWaitCondition    condVar;
    CHUpdateItemsList updateItems;
};

// --------------------------------------------------------

CameraHistoryUpdater::CameraHistoryUpdater(QWidget* const parent)
    : QThread(parent),
      d(new Private)
{
    qRegisterMetaType<CHUpdateItemMap>("CHUpdateItemMap");
}

CameraHistoryUpdater::~CameraHistoryUpdater()
{
    // clear updateItems, stop processing
    slotCancel();

    // stop thread
    {
        QMutexLocker lock(&d->mutex);
        d->running = false;
        d->condVar.wakeAll();
    }

    wait();

    delete d;
}

void CameraHistoryUpdater::slotCancel()
{
    d->canceled = true;
    QMutexLocker lock(&d->mutex);
    d->updateItems.clear();
}

void CameraHistoryUpdater::run()
{
    while (d->running)
    {
        CHUpdateItem item;

        QMutexLocker lock(&d->mutex);

        if (!d->updateItems.isEmpty())
        {
            item = d->updateItems.takeFirst();
            sendBusy(true);
            proccessMap(item.first, item.second);
        }
        else
        {
            sendBusy(false);
            d->condVar.wait(&d->mutex);
            continue;
        }
    }

    sendBusy(false);
}

void CameraHistoryUpdater::sendBusy(bool val)
{
    emit signalBusy(val);
}

void CameraHistoryUpdater::addItems(const QByteArray& id, CHUpdateItemMap& map)
{
    if (map.empty())
    {
        return;
    }

    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Check download state from DB for " << map.count() << " items";

    QMutexLocker lock(&d->mutex);
    d->running  = true;
    d->canceled = false;
    d->updateItems << CHUpdateItem(id, map);

    if (!isRunning())
    {
        start(LowPriority);
    }

    d->condVar.wakeAll();
}

void CameraHistoryUpdater::proccessMap(const QByteArray& id, CHUpdateItemMap& map)
{
    CHUpdateItemMap::iterator it = map.begin();

    do
    {
        // We query database to check if (*it).have been already downloaded from camera.
        switch (CoreDbDownloadHistory::status(QString::fromUtf8(id), (*it).name, (*it).size, (*it).ctime))
        {
            case CoreDbDownloadHistory::NotDownloaded:
                (*it).downloaded = CamItemInfo::NewPicture;
                break;

            case CoreDbDownloadHistory::Downloaded:
                (*it).downloaded = CamItemInfo::DownloadedYes;
                break;

            default: // CoreDbDownloadHistory::StatusUnknown
                (*it).downloaded = CamItemInfo::DownloadUnknown;
                break;
        }

        ++it;
    }
    while (it != map.end() && !d->canceled);

    emit signalHistoryMap(map);
}

}  // namespace Digikam

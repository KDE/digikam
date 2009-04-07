/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-06-05
 * Description : Multithreaded loader for thumbnails
 *
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "thumbnailtask.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QApplication>
#include <QImage>
#include <QVariant>
#include <QMatrix>

// KDE includes

#include <kdebug.h>

// LibKDcraw includes

#include <libkdcraw/kdcraw.h>

// Local includes

#include "dmetadata.h"
#include "jpegutils.h"
#include "thumbnailloadthread.h"
#include "thumbnailcreator.h"

namespace Digikam
{

ThumbnailLoadingTask::ThumbnailLoadingTask(LoadSaveThread* thread, LoadingDescription description)
                    : SharedLoadingTask(thread, description, LoadSaveThread::AccessModeRead,
                                        LoadingTaskStatusLoading)
{
    ThumbnailLoadThread *thumbThread = dynamic_cast<ThumbnailLoadThread *>(thread);
    m_creator = thumbThread->thumbnailCreator();
}

void ThumbnailLoadingTask::execute()
{
    if (m_loadingTaskStatus == LoadingTaskStatusStopping)
        return;

    // initialize creator
    m_creator->setThumbnailSize(m_loadingDescription.previewParameters.size);
    m_creator->setExifRotate(m_loadingDescription.previewParameters.exifRotate);
    m_creator->setLoadingProperties(this, m_loadingDescription.rawDecodingSettings);

    QImage qimage;

    LoadingCache *cache = LoadingCache::cache();
    {
        LoadingCache::CacheLock lock(cache);

        // find possible cached images
        const QImage *cachedImage = cache->retrieveThumbnail(m_loadingDescription.cacheKey());
        if (cachedImage)
            qimage = QImage(*cachedImage);

        if (qimage.isNull())
        {
            // find possible running loading process
            m_usedProcess = cache->retrieveLoadingProcess(m_loadingDescription.cacheKey());
            // do not wait on other loading processes?
            //m_usedProcess = cache->retrieveLoadingProcess(m_loadingDescription.cacheKey());

            if (m_usedProcess)
            {
                // Other process is right now loading this image.
                // Add this task to the list of listeners and
                // attach this thread to the other thread, wait until loading
                // has finished.
                m_usedProcess->addListener(this);
                // break loop when either the loading has completed, or this task is being stopped
                while ( !m_usedProcess->completed() && m_loadingTaskStatus != LoadingTaskStatusStopping )
                    lock.timedWait();
                // remove listener from process
                m_usedProcess->removeListener(this);
                // wake up the process which is waiting until all listeners have removed themselves
                lock.wakeAll();
                // set to 0, as checked in setStatus
                m_usedProcess = 0;
                return;
            }
            else
            {
                // Neither in cache, nor currently loading in different thread.
                // Load it here and now, add this LoadingProcess to cache list.
                cache->addLoadingProcess(this);
                // Add this to the list of listeners
                addListener(this);
                // for use in setStatus
                m_usedProcess = this;
                // Notify other processes that we are now loading this image.
                // They might be interested - see notifyNewLoadingProcess below
                cache->notifyNewLoadingProcess(this, m_loadingDescription);
            }
        }
    }

    if (!qimage.isNull())
    {
        // following the golden rule to avoid deadlocks, do this when CacheLock is not held
        m_thread->taskHasFinished();
        m_thread->thumbnailLoaded(m_loadingDescription, qimage);
        return;
    }

    // Load or create thumbnail
    qimage = m_creator->load(m_loadingDescription.filePath);

    {
        LoadingCache::CacheLock lock(cache);
        // put (valid) image into cache of loaded images
        if (!qimage.isNull())
            cache->putThumbnail(m_loadingDescription.cacheKey(), qimage, m_loadingDescription.filePath);
        // remove this from the list of loading processes in cache
        cache->removeLoadingProcess(this);
    }

    // again: following the golden rule to avoid deadlocks, do this when CacheLock is not held
    m_thread->taskHasFinished();

    {
        LoadingCache::CacheLock lock(cache);
        // indicate that loading has finished so that listeners can stop waiting
        m_completed = true;

        // dispatch image to all listeners, including this
        for (int i=0; i<m_listeners.count(); ++i)
        {
            m_listeners[i]->loadSaveNotifier()->thumbnailLoaded(m_loadingDescription, qimage);
        }

        // remove myself from list of listeners
        removeListener(this);
        // wake all listeners waiting on cache condVar, so that they remove themselves
        lock.wakeAll();
        // wait until all listeners have removed themselves
        while (m_listeners.count() != 0)
            lock.timedWait();
        // set to 0, as checked in setStatus
        m_usedProcess = 0;
    }
}

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-06-05
 * Description : Multithreaded loader for thumbnails
 *
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes

#include "drawdecoder.h"
#include "digikam_debug.h"
#include "dmetadata.h"
#include "iccmanager.h"
#include "jpegutils.h"
#include "metadatasettings.h"
#include "thumbnailloadthread.h"
#include "thumbnailcreator.h"

namespace Digikam
{

ThumbnailLoadingTask::ThumbnailLoadingTask(LoadSaveThread* const thread, const LoadingDescription& description)
    : SharedLoadingTask(thread, description, LoadSaveThread::AccessModeRead,
                        LoadingTaskStatusLoading)
{
    // Thread must be a ThumbnailLoadThread, crashes otherwise.
    // Not a clean but pragmatic solution.
    ThumbnailLoadThread* const thumbThread = static_cast<ThumbnailLoadThread*>(thread);
    m_creator                              = thumbThread->thumbnailCreator();
}

void ThumbnailLoadingTask::execute()
{
    if (m_loadingTaskStatus == LoadingTaskStatusStopping)
    {
        return;
    }

    if (m_loadingDescription.previewParameters.onlyPregenerate())
    {
        setupCreator();

        switch (m_loadingDescription.previewParameters.type)
        {
            case LoadingDescription::PreviewParameters::Thumbnail:
                m_creator->pregenerate(m_loadingDescription.thumbnailIdentifier());
                break;
            case LoadingDescription::PreviewParameters::DetailThumbnail:
                m_creator->pregenerateDetail(m_loadingDescription.thumbnailIdentifier(),
                                             m_loadingDescription.previewParameters.extraParameter.toRect());
                break;
            default:
                break;
        }

        m_thread->taskHasFinished();
        // do not emit any signal
        return;
    }

    LoadingCache* const cache = LoadingCache::cache();
    {
        LoadingCache::CacheLock lock(cache);

        // find possible cached images
        const QImage* const cachedImage = cache->retrieveThumbnail(m_loadingDescription.cacheKey());

        if (cachedImage)
        {
            m_qimage = QImage(*cachedImage);
        }

        if (m_qimage.isNull())
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
                {
                    lock.timedWait();
                }

                // remove listener from process
                if (m_usedProcess)
                {
                    m_usedProcess->removeListener(this);
                }

                // wake up the process which is waiting until all listeners have removed themselves
                lock.wakeAll();
                // set to 0, as checked in setStatus
                m_usedProcess = 0;
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

    if (!m_qimage.isNull())
    {
        // following the golden rule to avoid deadlocks, do this when CacheLock is not held
        postProcess();
        m_thread->taskHasFinished();
        m_thread->thumbnailLoaded(m_loadingDescription, m_qimage);
        return;
    }

    // Load or create thumbnail
    setupCreator();

    switch (m_loadingDescription.previewParameters.type)
    {
        case LoadingDescription::PreviewParameters::Thumbnail:
            m_qimage = m_creator->load(m_loadingDescription.thumbnailIdentifier());
            break;
        case LoadingDescription::PreviewParameters::DetailThumbnail:
            m_qimage = m_creator->loadDetail(m_loadingDescription.thumbnailIdentifier(),
                                             m_loadingDescription.previewParameters.extraParameter.toRect());
            break;
        default:
            break;
    }

    {
        LoadingCache::CacheLock lock(cache);

        // put (valid) image into cache of loaded images
        if (!m_qimage.isNull())
        {
            cache->putThumbnail(m_loadingDescription.cacheKey(), m_qimage, m_loadingDescription.filePath);
        }

        // remove this from the list of loading processes in cache
        cache->removeLoadingProcess(this);
    }

    {
        LoadingCache::CacheLock lock(cache);
        // indicate that loading has finished so that listeners can stop waiting
        m_completed = true;

        // dispatch image to all listeners, including this
        for (int i=0; i<m_listeners.count(); ++i)
        {
            ThumbnailLoadingTask* const task = dynamic_cast<ThumbnailLoadingTask*>(m_listeners.at(i));

            if (task)
            {
                task->setResult(m_loadingDescription, m_qimage);
            }
        }

        // remove myself from list of listeners
        removeListener(this);
        // wake all listeners waiting on cache condVar, so that they remove themselves
        lock.wakeAll();

        // wait until all listeners have removed themselves
        while (m_listeners.count() != 0)
        {
            lock.timedWait();
        }

        // set to 0, as checked in setStatus
        m_usedProcess = 0;
    }

    // again: following the golden rule to avoid deadlocks, do this when CacheLock is not held
    postProcess();
    m_thread->taskHasFinished();
    m_thread->thumbnailLoaded(m_loadingDescription, m_qimage);
}

void ThumbnailLoadingTask::setupCreator()
{
    m_creator->setThumbnailSize(m_loadingDescription.previewParameters.size);
    m_creator->setExifRotate(MetadataSettings::instance()->settings().exifRotate);
    m_creator->setLoadingProperties(this, m_loadingDescription.rawDecodingSettings);
}

void ThumbnailLoadingTask::setResult(const LoadingDescription& loadingDescription, const QImage& qimage)
{
    // this is called from another process's execute while this task is waiting on m_usedProcess.
    // Note that loadingDescription need not equal m_loadingDescription (may be superior)
    m_resultLoadingDescription = loadingDescription;
    // these are taken from our own description
    m_resultLoadingDescription.postProcessingParameters = m_loadingDescription.postProcessingParameters;
    m_qimage = qimage;
}

void ThumbnailLoadingTask::postProcess()
{
    m_loadingDescription.postProcessingParameters.profile().description();

    switch (m_loadingDescription.postProcessingParameters.colorManagement)
    {
        case LoadingDescription::NoColorConversion:
        {
            break;
        }
        case LoadingDescription::ConvertToSRGB:
        {
            // Thumbnails are stored in sRGB
            break;
        }
        case LoadingDescription::ConvertForDisplay:
        {
            IccManager::transformForDisplay(m_qimage, m_loadingDescription.postProcessingParameters.profile());
            break;
        }
        default:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Unsupported postprocessing parameter for thumbnail loading:"
                     << m_loadingDescription.postProcessingParameters.colorManagement;
            break;
        }
    }
}

} // namespace Digikam

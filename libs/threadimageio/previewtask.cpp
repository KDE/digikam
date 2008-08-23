/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-12-26
 * Description : Multithreaded loader for previews
 *
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

// C++ includes.

#include <cmath>

// Qt includes.

#include <qapplication.h>
#include <qimage.h>
#include <qvariant.h>
#include <qwmatrix.h>

// LibKDcraw includes.

#include <libkdcraw/kdcraw.h>

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
#include "jpegutils.h"
#include "previewloadthread.h"
#include "previewtask.h"

namespace Digikam
{

void PreviewLoadingTask::execute()
{
    if (m_loadingTaskStatus == LoadingTaskStatusStopping)
        return;

    LoadingCache *cache = LoadingCache::cache();
    {
        LoadingCache::CacheLock lock(cache);

        // find possible cached images
        DImg *cachedImg = 0;
        QStringList lookupKeys = m_loadingDescription.lookupCacheKeys();
        // lookupCacheKeys returns "best first". Prepend the cache key to make the list "fastest first":
        // Scaling a full version takes longer!
        lookupKeys.push_front(m_loadingDescription.cacheKey());
        for ( QStringList::Iterator it = lookupKeys.begin(); it != lookupKeys.end(); ++it ) {
            if ( (cachedImg = cache->retrieveImage(*it)) )
                break;
        }

        if (cachedImg)
        {
            // image is found in image cache, loading is successful

            DImg img(*cachedImg);

            // rotate if needed - images are unrotated in the cache,
            // except for RAW images, which are already rotated by dcraw.
            if (m_loadingDescription.previewParameters.exifRotate)
            {
                img = img.copy();
                LoadSaveThread::exifRotate(img, m_loadingDescription.filePath);
            }

            QApplication::postEvent(m_thread, new LoadedEvent(m_loadingDescription.filePath, img));
            return;
        }
        else
        {
            // find possible running loading process
            m_usedProcess = 0;
            for ( QStringList::Iterator it = lookupKeys.begin(); it != lookupKeys.end(); ++it ) {
                if ( (m_usedProcess = cache->retrieveLoadingProcess(*it)) )
                {
                    break;
                }
            }
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

    // load image
    int  size = m_loadingDescription.previewParameters.size;

    DImg img;
    QImage qimage;
    bool fromEmbeddedPreview = false;

    // -- Get the image preview --------------------------------

    // First the QImage-dependent loading methods
    // Trying to load with dcraw: RAW files.
    if (KDcrawIface::KDcraw::loadEmbeddedPreview(qimage, m_loadingDescription.filePath))
        fromEmbeddedPreview = true;

    if (qimage.isNull())
    {
        //TODO: Use DImg based loader instead?
        KDcrawIface::KDcraw::loadHalfPreview(qimage, m_loadingDescription.filePath);
    }

    // Try to extract Exif/Iptc preview.
    if (qimage.isNull())
    {
        loadImagePreview(qimage, m_loadingDescription.filePath);
    }

    if (!qimage.isNull())
    {
        // convert from QImage
        img = DImg(qimage);
        // mark as embedded preview (for exif rotation)
        if (fromEmbeddedPreview)
            img.setAttribute("fromRawEmbeddedPreview", true);
        // free memory
        qimage = QImage();
    }

    // DImg-dependent loading methods
    if (img.isNull())
    {
        // Set a hint to try to load a JPEG with the fast scale-before-decoding method
        img.setAttribute("jpegScaledLoadingSize", size);
        img.load(m_loadingDescription.filePath, this, m_loadingDescription.rawDecodingSettings);
    }

    if (img.isNull())
    {
        DWarning() << "Cannot extract preview for " << m_loadingDescription.filePath << endl;
    }

    img.convertToEightBit();

    // Reduce size of image:
    // - only scale down if size is considerably larger
    // - only scale down, do not scale up
    QSize scaledSize = img.size();
    if (needToScale(scaledSize, size))
    {
        scaledSize.scale(size, size, QSize::ScaleMin);
        img = img.smoothScale(scaledSize.width(), scaledSize.height());
    }

    // Scale if hinted, Store previews rotated in the cache (?)
    if (m_loadingDescription.previewParameters.exifRotate)
        LoadSaveThread::exifRotate(img, m_loadingDescription.filePath);

    {
        LoadingCache::CacheLock lock(cache);
        // put (valid) image into cache of loaded images
        if (!img.isNull())
            cache->putImage(m_loadingDescription.cacheKey(), new DImg(img), m_loadingDescription.filePath);
        // remove this from the list of loading processes in cache
        cache->removeLoadingProcess(this);
    }

    // following the golden rule to avoid deadlocks, do this when CacheLock is not held
    m_thread->taskHasFinished();

    {
        LoadingCache::CacheLock lock(cache);
        // indicate that loading has finished so that listeners can stop waiting
        m_completed = true;

        // dispatch image to all listeners, including this
        for (LoadingProcessListener *l = m_listeners.first(); l; l = m_listeners.next())
        {
            QApplication::postEvent(l->eventReceiver(), new LoadedEvent(m_loadingDescription, img));
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

bool PreviewLoadingTask::needToScale(const QSize &imageSize, int previewSize)
{
    int maxSize = imageSize.width() > imageSize.height() ? imageSize.width() : imageSize.height();
    int acceptableUpperSize = lround(1.25 * (double)previewSize);
    return  maxSize >= acceptableUpperSize;
}

// -- Exif/IPTC preview extraction using Exiv2 --------------------------------------------------------

bool PreviewLoadingTask::loadImagePreview(QImage& image, const QString& path)
{
    DMetadata metadata(path);
    if (metadata.getImagePreview(image))
    {
        DDebug() << "Use Exif/Iptc preview extraction. Size of image: "
                  << image.width() << "x" << image.height() << endl;
        return true;
    }

    return false;
}

} // namespace Digikam

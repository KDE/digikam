/* ============================================================
 * Authors: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 *          Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2006-12-26
 * Description : Multithreaded loader for previews
 *
 * Copyright 2006-2007 by Marcel Wiesweg, Gilles Caulier
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

// Qt includes

#include <qapplication.h>
#include <qimage.h>
#include <qvariant.h>
#include <qwmatrix.h>

// LibKDcraw includes.

#include <libkdcraw/kdcraw.h>

// Local includes

#include "ddebug.h"
#include "dmetadata.h"
#include "jpegutils.h"
#include "previewloadthread.h"
#include "previewtask.h"

namespace Digikam
{

void PreviewLoadedEvent::notify(LoadSaveThread *thread)
{
    static_cast<PreviewLoadThread *>(thread)->previewLoaded(m_loadingDescription, m_image);
}

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

            // In the cache, the QImage is wrapped in a DImg object.
            QImage qimage;
            QVariant attribute(cachedImg->attribute("previewQImage"));
            if (attribute.isValid())
            {
                // the image was stored as a QImage-in-a-DImg
                qimage = attribute.toImage();
            }
            else
            {
                // we are using a normal DImg object. Convert it to QImage.
                qimage = cachedImg->copyQImage();

                // rotate if needed - images are unrotated in the cache
                if (m_loadingDescription.previewParameters.exifRotate)
                    exifRotate(m_loadingDescription.filePath, qimage);

            }

            QApplication::postEvent(m_thread, new PreviewLoadedEvent(m_loadingDescription.filePath, qimage));
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

    QImage qimage;

    // -- Get the image preview --------------------------------
    // In first, we trying to load with dcraw : RAW files.
    if ( !KDcrawIface::KDcraw::loadDcrawPreview(qimage, m_loadingDescription.filePath) )
    {
        // Try to extract Exif/Iptc preview.
        if ( !loadImagePreview(qimage, m_loadingDescription.filePath) )
        {
            // Try to load a JPEG with the fast scale-before-decoding method
            if (!loadJPEGScaled(qimage, m_loadingDescription.filePath, size))
            {
                // Try to load with Qt/KDE.
                qimage.load(m_loadingDescription.filePath);
            }
        }
    }

    if (qimage.isNull())
    {
        DWarning() << "Cannot extract preview for " << m_loadingDescription.filePath << endl;
    }

    if (qimage.depth() != 32)
        qimage = qimage.convertDepth(32);

    qimage = qimage.smoothScale(size, size, QImage::ScaleMin);

    if (m_loadingDescription.previewParameters.exifRotate)
        exifRotate(m_loadingDescription.filePath, qimage);

    {
        LoadingCache::CacheLock lock(cache);
        // put (valid) image into cache of loaded images
        if (!qimage.isNull())
        {
            // Wrap QImage into DImg object
            DImg img;
            img.setAttribute("previewQImage", qimage);

            cache->putImage(m_loadingDescription.cacheKey(), new DImg(img), m_loadingDescription.filePath);
        }
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
            QApplication::postEvent(l->eventReceiver(), new PreviewLoadedEvent(m_loadingDescription, qimage));
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



void PreviewLoadingTask::exifRotate(const QString& filePath, QImage& thumb)
{
    // Rotate thumbnail based on metadata orientation information

    DMetadata metadata(filePath);
    DMetadata::ImageOrientation orientation = metadata.getImageOrientation();

    if (orientation == DMetadata::ORIENTATION_NORMAL ||
        orientation == DMetadata::ORIENTATION_UNSPECIFIED)
        return;

    QWMatrix matrix;

    switch (orientation)
    {
        case DMetadata::ORIENTATION_NORMAL:
        case DMetadata::ORIENTATION_UNSPECIFIED:
            break;

        case DMetadata::ORIENTATION_HFLIP:
            matrix.scale(-1, 1);
            break;

        case DMetadata::ORIENTATION_ROT_180:
            matrix.rotate(180);
            break;

        case DMetadata::ORIENTATION_VFLIP:
            matrix.scale(1, -1);
            break;

        case DMetadata::ORIENTATION_ROT_90_HFLIP:
            matrix.scale(-1, 1);
            matrix.rotate(90);
            break;

        case DMetadata::ORIENTATION_ROT_90:
            matrix.rotate(90);
            break;

        case DMetadata::ORIENTATION_ROT_90_VFLIP:
            matrix.scale(1, -1);
            matrix.rotate(90);
            break;

        case DMetadata::ORIENTATION_ROT_270:
            matrix.rotate(270);
            break;
    }

    // transform accordingly
    thumb = thumb.xForm( matrix );
}

} // namespace Digikam


/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-12-26
 * Description : Multithreaded loader for previews
 *
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "previewtask.h"

// C ANSI includes

#include <cmath>

// Qt includes

#include <QApplication>
#include <QImage>
#include <QVariant>
#include <QMatrix>

// KDE includes

#include <kdebug.h>

// libkexiv2 includes

#include <libkexiv2/version.h>
#include <libkexiv2/kexiv2previews.h>

// LibKDcraw includes

#include <libkdcraw/kdcraw.h>

// Local includes

#include "dmetadata.h"
#include "jpegutils.h"
#include "previewloadthread.h"

namespace Digikam
{

void PreviewLoadingTask::execute()
{
    if (m_loadingTaskStatus == LoadingTaskStatusStopping)
    {
        return;
    }

    LoadingCache* cache = LoadingCache::cache();
    {
        LoadingCache::CacheLock lock(cache);

        // find possible cached images
        DImg* cachedImg        = 0;
        QStringList lookupKeys = m_loadingDescription.lookupCacheKeys();

        // lookupCacheKeys returns "best first". Prepend the cache key to make the list "fastest first":
        // Scaling a full version takes longer!
        lookupKeys.push_front(m_loadingDescription.cacheKey());

        foreach (const QString& key, lookupKeys)
        {
            if ( (cachedImg = cache->retrieveImage(key)) )
            {
                if (m_loadingDescription.needCheckRawDecoding())
                {
                    if (cachedImg->rawDecodingSettings() == m_loadingDescription.rawDecodingSettings)
                    {
                        break;
                    }
                    else
                    {
                        cachedImg = 0;
                    }
                }
                else
                {
                    break;
                }
            }
        }

        if (cachedImg)
        {
            // image is found in image cache, loading is successful
            m_img = *cachedImg;

            if (accessMode() == LoadSaveThread::AccessModeReadWrite)
            {
                m_img = m_img.copy();
            }

            // rotate if needed - images are unrotated in the cache,
            // except for RAW images, which are already rotated by dcraw.
            if (m_loadingDescription.previewParameters.exifRotate())
            {
                m_img = m_img.copy();
                LoadSaveThread::exifRotate(m_img, m_loadingDescription.filePath);
            }
        }
        else
        {
            // find possible running loading process
            m_usedProcess = 0;

            for ( QStringList::Iterator it = lookupKeys.begin(); it != lookupKeys.end(); ++it )
            {
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
                while ( m_loadingTaskStatus != LoadingTaskStatusStopping && m_usedProcess && !m_usedProcess->completed())
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
                // m_img is now set to the result
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

    if (!m_img.isNull())
    {
        // following the golden rule to avoid deadlocks, do this when CacheLock is not held

        // The image from the cache may or may not be rotated and post processed.
        // exifRotate() and postProcess() will detect if work is needed.
        if (m_loadingDescription.previewParameters.exifRotate())
        {
            LoadSaveThread::exifRotate(m_img, m_loadingDescription.filePath);
        }

        postProcess();
        m_thread->taskHasFinished();
        m_thread->imageLoaded(m_resultLoadingDescription, m_img);
        return;
    }

    // load image
    int  size = m_loadingDescription.previewParameters.size;

    QImage qimage;
    bool   fromEmbeddedPreview = false;

    // -- Get the image preview --------------------------------

    if (size)
    {
        DImg::FORMAT format = DImg::fileFormat(m_loadingDescription.filePath);
        // First the QImage-dependent loading methods

        // check embedded previews
        KExiv2Iface::KExiv2Previews previews(m_loadingDescription.filePath);

        int sizeLimit      = -1;
        QSize originalSize = previews.originalSize();
        int bestSize       = qMax(originalSize.width(), originalSize.height());

        // for RAWs, the alternative is the half preview, so best size is already originalSize / 2
        if (format == DImg::RAW)
        {
            bestSize /= 2;
        }

        if (m_loadingDescription.previewParameters.fastButLarge())
        {
            if (format == DImg::RAW)
            {
                sizeLimit = qMin(size, bestSize);
            }
        }
        else
        {
            int aBitSmallerThanSize = (int)lround(double(size) * 0.8);
            sizeLimit               = qMin(aBitSmallerThanSize, bestSize);
        }

        // Only check the first and largest preview
        if (sizeLimit != -1 && !previews.isEmpty() && continueQuery())
        {
            // require at least half preview size
            if (qMax(previews.width(), previews.height()) >= sizeLimit)
            {
                qimage = previews.image();

                if (!qimage.isNull())
                {
                    fromEmbeddedPreview = true;
                }
            }
        }

        if (qimage.isNull() && continueQuery())
        {
            //TODO: Use DImg based loader instead?
            KDcrawIface::KDcraw::loadHalfPreview(qimage, m_loadingDescription.filePath);
        }

        // Try to extract Exif/IPTC preview.
        if (qimage.isNull() && continueQuery())
        {
            loadImagePreview(qimage, m_loadingDescription.filePath);
        }

        if (!qimage.isNull() && continueQuery())
        {
            // convert from QImage
            m_img               = DImg(qimage);
            DImg::FORMAT format = DImg::fileFormat(m_loadingDescription.filePath);
            m_img.setAttribute("detectedFileFormat", format);
            m_img.setAttribute("originalFilePath", m_loadingDescription.filePath);

            DMetadata metadata(m_loadingDescription.filePath);
            m_img.setAttribute("originalSize", metadata.getPixelSize());

            // mark as embedded preview (for Exif rotation)
            if (fromEmbeddedPreview)
            {
                m_img.setAttribute("fromRawEmbeddedPreview", true);

                // If we loaded the embedded preview, the Exif of the RAW indicates
                // the color space of the preview (see bug 195950 for NEF files)
                m_img.setIccProfile(metadata.getIccProfile());
            }

            // free memory
            qimage = QImage();
        }

        // DImg-dependent loading methods
        if (m_img.isNull() && continueQuery())
        {
            // Set a hint to try to load a JPEG or PGF with the fast scale-before-decoding method
            if (!m_loadingDescription.previewParameters.fastButLarge())
            {
                m_img.setAttribute("scaledLoadingSize", size);
            }

            m_img.load(m_loadingDescription.filePath, this, m_loadingDescription.rawDecodingSettings);
        }

        if (m_img.isNull() && continueQuery())
        {
            kWarning() << "Cannot extract preview for " << m_loadingDescription.filePath;
        }
    }
    else
    {
        {
            // check embedded previews
            KExiv2Iface::KExiv2Previews previews(m_loadingDescription.filePath);

            QSize originalSize   = previews.originalSize();
            // discard if smaller than half preview
            int acceptableWidth  = lround(originalSize.width()  * 0.48);
            int acceptableHeight = lround(originalSize.height() * 0.48);

            if (qimage.isNull() && !previews.isEmpty() && continueQuery())
            {
                if (previews.width() >= acceptableWidth &&  previews.height() >= acceptableHeight)
                {
                    qimage = previews.image();

                    if (!qimage.isNull())
                    {
                        fromEmbeddedPreview = true;
                    }
                }
            }
        }

        if (qimage.isNull() && continueQuery())
        {
            KDcrawIface::KDcraw::loadHalfPreview(qimage, m_loadingDescription.filePath);
        }

        if (!qimage.isNull() && continueQuery())
        {
            // convert from QImage
            m_img               = DImg(qimage);
            DImg::FORMAT format = DImg::fileFormat(m_loadingDescription.filePath);
            m_img.setAttribute("detectedFileFormat", format);
            m_img.setAttribute("originalFilePath", m_loadingDescription.filePath);

            DMetadata metadata(m_loadingDescription.filePath);
            m_img.setAttribute("originalSize", metadata.getPixelSize());

            // mark as embedded preview (for Exif rotation)
            if (fromEmbeddedPreview)
            {
                m_img.setAttribute("fromRawEmbeddedPreview", true);

                // If we loaded the embedded preview, the Exif of the RAW indicates
                // the color space of the preview (see bug 195950 for NEF files)
                m_img.setIccProfile(metadata.getIccProfile());
            }

            // free memory
            qimage = QImage();
        }

        // DImg-dependent loading methods
        if (m_img.isNull() && continueQuery())
        {
            m_img.load(m_loadingDescription.filePath, this, m_loadingDescription.rawDecodingSettings);

            // Now that we did a full load of the image, consider putting it in the cache
            // but not for RAWs, there are so many cases to consider
            if (!m_img.isNull() && m_img.detectedFormat() != DImg::RAW)
            {
                LoadingCache::CacheLock lock(cache);
                LoadingDescription fullDescription(m_loadingDescription.filePath);
                cache->putImage(fullDescription.cacheKey(), new DImg(m_img.copy()), m_loadingDescription.filePath);
            }
        }

        if (m_img.isNull() && continueQuery())
        {
            kWarning() << "Cannot extract preview for " << m_loadingDescription.filePath;
        }
    }

    if (continueQuery())
    {
        m_img.convertToEightBit();

        // Reduce size of image:
        // - only scale down if size is considerably larger
        // - only scale down, do not scale up
        QSize scaledSize = m_img.size();

        if (needToScale(scaledSize, size))
        {
            scaledSize.scale(size, size, Qt::KeepAspectRatio);
            m_img = m_img.smoothScale(scaledSize.width(), scaledSize.height());
        }

        // Scale if hinted, Store previews rotated in the cache (?)
        if (m_loadingDescription.previewParameters.exifRotate())
        {
            LoadSaveThread::exifRotate(m_img, m_loadingDescription.filePath);
        }

        // For previews, we put the image post processed in the cache
        postProcess();
    }
    else
    {
        m_img = DImg();
    }

    {
        LoadingCache::CacheLock lock(cache);

        // put (valid) image into cache of loaded images
        if (!m_img.isNull())
        {
            cache->putImage(m_loadingDescription.cacheKey(), new DImg(m_img), m_loadingDescription.filePath);
        }

        // remove this from the list of loading processes in cache
        cache->removeLoadingProcess(this);
    }

    {
        LoadingCache::CacheLock lock(cache);
        // indicate that loading has finished so that listeners can stop waiting
        m_completed = true;

        // dispatch image to all listeners, including this
        for (int i = 0; i < m_listeners.count(); ++i)
        {
            LoadingProcessListener* l = m_listeners[i];

            if (l->accessMode() == LoadSaveThread::AccessModeReadWrite)
            {
                // If a listener requested ReadWrite access, it gets a deep copy.
                // DImg is explicitly shared.
                DImg copy = m_img.copy();
                l->setResult(m_loadingDescription, copy);
            }
            else
            {
                l->setResult(m_loadingDescription, m_img);
            }
        }

        for (int i = 0; i < m_listeners.count(); ++i)
        {
            m_listeners[i]->loadSaveNotifier()->imageLoaded(m_loadingDescription, m_img);
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
    m_thread->taskHasFinished();
    m_thread->imageLoaded(m_loadingDescription, m_img);
}

bool PreviewLoadingTask::needToScale(const QSize& imageSize, int previewSize)
{
    if (!previewSize)
    {
        return false;
    }

    if (m_loadingDescription.previewParameters.fastButLarge())
    {
        return false;
    }

    int maxSize             = imageSize.width() > imageSize.height() ? imageSize.width() : imageSize.height();
    int acceptableUpperSize = lround(1.25 * (double)previewSize);
    return (maxSize >= acceptableUpperSize);
}

// -- Exif/IPTC preview extraction using Exiv2 --------------------------------------------------------

bool PreviewLoadingTask::loadImagePreview(QImage& image, const QString& path)
{
    DMetadata metadata(path);

    if (metadata.getImagePreview(image))
    {
        kDebug(50003) << "Use Exif/IPTC preview extraction. Size of image: "
                      << image.width() << "x" << image.height();
        return true;
    }

    return false;
}

} // namespace Digikam

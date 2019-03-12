/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-12-26
 * Description : Multithreaded loader for previews
 *
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QImage>
#include <QVariant>
#include <QMatrix>

// Local includes

#include "drawdecoder.h"
#include "digikam_debug.h"
#include "dmetadata.h"
#include "jpegutils.h"
#include "metaenginesettings.h"
#include "previewloadthread.h"

namespace Digikam
{

void PreviewLoadingTask::execute()
{
    if (m_loadingTaskStatus == LoadingTaskStatusStopping)
    {
        return;
    }

    // Check if preview is in cache first.

    LoadingCache* const cache = LoadingCache::cache();
    {
        LoadingCache::CacheLock lock(cache);

        // find possible cached images
        DImg* cachedImg        = 0;
        QStringList lookupKeys = m_loadingDescription.lookupCacheKeys();

        // lookupCacheKeys returns "best first". Prepend the cache key to make the list "fastest first":
        // Scaling a full version takes longer!
        lookupKeys.prepend(m_loadingDescription.cacheKey());

        foreach (const QString& key, lookupKeys)
        {
            if ((cachedImg = cache->retrieveImage(key)))
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
        }
        else
        {
            // find possible running loading process
            m_usedProcess = 0;

            for (QStringList::const_iterator it = lookupKeys.constBegin() ; it != lookupKeys.constEnd() ; ++it)
            {
                if ((m_usedProcess = cache->retrieveLoadingProcess(*it)))
                {
                    break;
                }
            }

            if (m_usedProcess)
            {
                // Other process is right now loading this image.
                // Add this task to the list of listeners and
                // attach this thread to the other thread, wait until loading
                // has finished.
                m_usedProcess->addListener(this);

                // break loop when either the loading has completed, or this task is being stopped
                while (m_loadingTaskStatus != LoadingTaskStatusStopping &&
                       m_usedProcess                                    &&
                       !m_usedProcess->completed())
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

    if (m_img.isNull())
    {
        // Preview is not in cache, we will load image from file.

        DImg::FORMAT format      = DImg::fileFormat(m_loadingDescription.filePath);
        m_fromRawEmbeddedPreview = false;

        if (format == DImg::RAW)
        {
            MetaEnginePreviews previews(m_loadingDescription.filePath);
            // Check original image size using Exiv2.
            QSize originalSize  = previews.originalSize();

            // If not valid, get original size from LibRaw
            if (!originalSize.isValid())
            {
                DRawInfo container;

                if (DRawDecoder::rawFileIdentify(container, m_loadingDescription.filePath))
                {
                    originalSize = container.imageSize;
                }
            }

            switch (m_loadingDescription.previewParameters.previewSettings.quality)
            {
                case PreviewSettings::FastPreview:
                case PreviewSettings::FastButLargePreview:
                {
                    // Size calculations
                    int sizeLimit = -1;
                    int bestSize  = qMax(originalSize.width(), originalSize.height());
                    // for RAWs, the alternative is the half preview, so best size is already originalSize / 2
                    bestSize     /= 2;

                    if (m_loadingDescription.previewParameters.previewSettings.quality == PreviewSettings::FastButLargePreview)
                    {
                        sizeLimit = qMin(m_loadingDescription.previewParameters.size, bestSize);
                    }

                    if (loadExiv2Preview(previews, sizeLimit))
                    {
                        break;
                    }

                    if (loadLibRawPreview(sizeLimit))
                    {
                        break;
                    }

                    loadHalfSizeRaw();
                    break;
                }

                case PreviewSettings::HighQualityPreview:
                {
                    switch (m_loadingDescription.previewParameters.previewSettings.rawLoading)
                    {
                        case PreviewSettings::RawPreviewAutomatic:
                        {
                            // If we find a preview that is larger than half size (which is what we get from half-size original data), we take it
                            int acceptableSize = qMax(lround(originalSize.width()  * 0.48), lround(originalSize.height() * 0.48));

                            if (loadExiv2Preview(previews, acceptableSize))
                            {
                                break;
                            }

                            if (loadLibRawPreview(acceptableSize))
                            {
                                break;
                            }

                            loadHalfSizeRaw();
                            break;
                        }

                        case PreviewSettings::RawPreviewFromEmbeddedPreview:
                        {
                            if (loadExiv2Preview(previews))
                            {
                                break;
                            }

                            if (loadLibRawPreview())
                            {
                                break;
                            }

                            loadHalfSizeRaw();
                            break;
                        }

                        case PreviewSettings::RawPreviewFromRawHalfSize:
                        {
                            loadHalfSizeRaw();
                            break;
                        }
                    }
                }
            }

            // So far, everything loaded QImage. Convert to DImg.
            convertQImageToDImg();
        }
        else // Non-RAW images
        {
            bool isFast = (m_loadingDescription.previewParameters.previewSettings.quality == PreviewSettings::FastPreview);

            switch (m_loadingDescription.previewParameters.previewSettings.quality)
            {
                case PreviewSettings::FastPreview:
                case PreviewSettings::FastButLargePreview:
                {
                    if (isFast && loadImagePreview(m_loadingDescription.previewParameters.size))
                    {
                        convertQImageToDImg();
                        break;
                    }

                    if (continueQuery(&m_img))
                    {
                        // Set a hint to try to load a JPEG or PGF with the fast scale-before-decoding method
                        if (isFast)
                        {
                            m_img.setAttribute(QLatin1String("scaledLoadingSize"), m_loadingDescription.previewParameters.size);
                        }

                        m_img.load(m_loadingDescription.filePath, this, m_loadingDescription.rawDecodingSettings);
                    }

                    break;
                }

                case PreviewSettings::HighQualityPreview:
                {
                    if (continueQuery(&m_img))
                    {
                        m_img.load(m_loadingDescription.filePath, this, m_loadingDescription.rawDecodingSettings);
                    }

                    break;
                }
            }
        }

        if (!m_img.isNull() && MetaEngineSettings::instance()->settings().exifRotate)
        {
            LoadSaveThread::exifRotate(m_img, m_loadingDescription.filePath);
        }

        LoadingCache::CacheLock lock(cache);

        // Put valid image into cache of loaded images

        if (!m_img.isNull())
        {
            cache->putImage(m_loadingDescription.cacheKey(), m_img,
                            m_loadingDescription.filePath);
        }

        // remove this from the list of loading processes in cache
        cache->removeLoadingProcess(this);

        // indicate that loading has finished so that listeners can stop waiting
        m_completed = true;

        // dispatch image to all listeners, including this

        for (int i = 0 ; i < m_listeners.count() ; ++i)
        {
            LoadingProcessListener* const l  = m_listeners[i];
            LoadSaveNotifier* const notifier = l->loadSaveNotifier();

            if (l->accessMode() == LoadSaveThread::AccessModeReadWrite)
            {
                // If a listener requested ReadWrite access, it gets a deep copy.
                // DImg is explicitly shared.
                l->setResult(m_loadingDescription, m_img.copy());
            }
            else
            {
                l->setResult(m_loadingDescription, m_img);
            }

            if (notifier)
            {
                notifier->imageLoaded(m_loadingDescription, m_img);
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

    // following the golden rule to avoid deadlocks, do this when CacheLock is not held

    if (!m_img.isNull() && continueQuery(&m_img))
    {
        // The image from the cache may or may not be rotated and post processed.
        // exifRotate() and postProcess() will detect if work is needed.
        // We check before to find out if we need to provide a deep copy

        const bool needExifRotate        = MetaEngineSettings::instance()->settings().exifRotate &&
                                           !LoadSaveThread::wasExifRotated(m_img);
        const bool needImageScale        = needToScale();
        const bool needPostProcess       = needsPostProcessing();
        const bool needConvertToEightBit = m_loadingDescription.previewParameters.previewSettings.convertToEightBit;

        if (accessMode() == LoadSaveThread::AccessModeReadWrite  ||
            needExifRotate  || needImageScale || needPostProcess ||
            needConvertToEightBit)
        {
            m_img.detach();
        }

        if (needImageScale)
        {
            QSize scaledSize = m_img.size();
            scaledSize.scale(m_loadingDescription.previewParameters.size,
                             m_loadingDescription.previewParameters.size,
                             Qt::KeepAspectRatio);
            m_img = m_img.smoothScale(scaledSize.width(), scaledSize.height());
        }

        if (needConvertToEightBit)
        {
            m_img.convertToEightBit();
        }

        if (needExifRotate)
        {
            LoadSaveThread::exifRotate(m_img, m_loadingDescription.filePath);
        }

        if (needPostProcess)
        {
            postProcess();
        }
    }
    else if (continueQuery(&m_img))
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot extract preview for" << m_loadingDescription.filePath;
    }
    else
    {
        m_img = DImg();
    }

    if (m_thread)
    {
        m_thread->taskHasFinished();
        m_thread->imageLoaded(m_loadingDescription, m_img);
    }
}

bool PreviewLoadingTask::needToScale()
{
    switch (m_loadingDescription.previewParameters.previewSettings.quality)
    {
        case PreviewSettings::FastPreview:
            if (m_loadingDescription.previewParameters.size > 0)
            {
                int maxSize             = qMax(m_img.width(), m_img.height());
                int acceptableUpperSize = lround(1.25 * (double)m_loadingDescription.previewParameters.size);
                return (maxSize >= acceptableUpperSize);
            }
            break;

        case PreviewSettings::FastButLargePreview:
        case PreviewSettings::HighQualityPreview:
            break;
    }

    return false;
}

// -- Exif/IPTC preview extraction using Exiv2 --------------------------------------------------------

bool PreviewLoadingTask::loadExiv2Preview(MetaEnginePreviews& previews, int sizeLimit)
{
    if (previews.isEmpty() || !continueQuery(&m_img))
    {
        return false;
    }

    if (sizeLimit == -1 || qMax(previews.width(), previews.height()) >= sizeLimit)
    {
        m_qimage = previews.image();

        if (!m_qimage.isNull())
        {
            m_fromRawEmbeddedPreview = true;
            return true;
        }
    }

    return false;
}

bool PreviewLoadingTask::loadLibRawPreview(int sizeLimit)
{
    if (!continueQuery(&m_img))
    {
        return false;
    }

    QImage rawPreview;
    DRawDecoder::loadEmbeddedPreview(rawPreview, m_loadingDescription.filePath);

    if (!rawPreview.isNull() &&
        (sizeLimit == -1 || qMax(rawPreview.width(), rawPreview.height()) >= sizeLimit))
    {
        m_qimage                 = rawPreview;
        m_fromRawEmbeddedPreview = true;
        return true;
    }

    return false;
}

bool PreviewLoadingTask::loadHalfSizeRaw()
{
    if (!continueQuery(&m_img))
    {
        return false;
    }

    DRawDecoder::loadHalfPreview(m_qimage, m_loadingDescription.filePath);
    return (!m_qimage.isNull());
}

void PreviewLoadingTask::convertQImageToDImg()
{
    if (!continueQuery(&m_img))
    {
        return;
    }

    // convert from QImage
    m_img               = DImg(m_qimage);
    DImg::FORMAT format = DImg::fileFormat(m_loadingDescription.filePath);
    m_img.setAttribute(QLatin1String("detectedFileFormat"), format);
    m_img.setAttribute(QLatin1String("originalFilePath"),   m_loadingDescription.filePath);

    DMetadata metadata(m_loadingDescription.filePath);
    m_img.setAttribute(QLatin1String("originalSize"),       metadata.getPixelSize());
    m_img.setMetadata(metadata.data());

    // mark as embedded preview (for Exif rotation)

    if (m_fromRawEmbeddedPreview)
    {
        m_img.setAttribute(QLatin1String("fromRawEmbeddedPreview"), true);

        // If we loaded the embedded preview, the Exif of the RAW indicates
        // the color space of the preview (see bug 195950 for NEF files)
        m_img.setIccProfile(metadata.getIccProfile());
    }

    // free memory
    m_qimage = QImage();
}

bool PreviewLoadingTask::loadImagePreview(int sizeLimit)
{
    DMetadata metadata(m_loadingDescription.filePath);

    QImage previewImage;

    if (metadata.getItemPreview(previewImage))
    {
        if (sizeLimit == -1 || qMax(previewImage.width(), previewImage.height()) > sizeLimit)
        {
            m_qimage = previewImage;
            return true;
        }
    }

    return false;
}

} // namespace Digikam

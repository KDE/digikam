/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "ImageLoadingThread.h"

// Qt includes

#include <QCoreApplication>
#include <QSemaphore>
#include <QFile>
#include <QByteArray>
#include <QDataStream>
#include <QBuffer>
#include <QFileInfo>

// KDE includes

#include <klocalizedstring.h>
#include "digikam_debug.h"

// Libkdcaw includes

#include "drawdecoder.h"
#include "drawdecodersettings.h"

// Local includes

#include "dmetadata.h"
#include "ProgressEvent.h"
#include "photolayoutswindow.h"

using namespace Digikam;


namespace PhotoLayoutsEditor
{

class ImageLoadingThread::ImageLoadingThreadPrivate
{
    ImageLoadingThreadPrivate() :
        m_sem(1),
        m_size(0),
        m_loaded_bytes(0),
        m_max_progress(1)
    {
    }

    QList<QUrl> m_urls;
    QSemaphore m_sem;
    qint64     m_size;
    qint64     m_loaded_bytes;
    double     m_max_progress;

    friend class ImageLoadingThread;
};

class RAWLoader : public DRawDecoder
{
    double              m_max_progress;
    ImageLoadingThread* m_thread;

public:

    RAWLoader(ImageLoadingThread* const thread) :
        m_max_progress(100),
        m_thread(thread)
    {
    }

    void setMaxDataProgress(double value)
    {
        m_max_progress = value;
    }

protected:

    virtual void setWaitingDataProgress(double value)
    {
        ProgressEvent* event = new ProgressEvent(m_thread);
        event->setData(ProgressEvent::ProgressUpdate, value * m_max_progress / 0.4);
        QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), event);
        QCoreApplication::processEvents();
    }
};

ImageLoadingThread::ImageLoadingThread(QObject* const parent) :
    QThread(parent),
    d(new ImageLoadingThreadPrivate)
{
}

ImageLoadingThread::~ImageLoadingThread()
{
    delete d;
}

void ImageLoadingThread::run()
{
    QList<QUrl> urls = d->m_urls;

    // Calculating reading progress
    d->m_loaded_bytes = d->m_size = 0;

    foreach(const QUrl& url, urls)
    {
        QFile f(url.path());
        f.open(QIODevice::ReadOnly);
        if (f.isReadable())
        {
            d->m_sem.acquire();
            d->m_size += f.size();
            d->m_sem.release();
        }
        f.close();
    }

    if (!d->m_size)
        goto finish_thread;

    // Reading
    foreach(const QUrl& url, urls)
    {
        ProgressEvent* startEvent = new ProgressEvent(this);
        startEvent->setData(ProgressEvent::Init, 0);
        QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), startEvent);
        QCoreApplication::processEvents();

        if (DRawDecoder::isRawFile(url))
            loadRaw(url);
        else
            loadImage(url);

        ProgressEvent* finishEvent = new ProgressEvent(this);
        finishEvent->setData(ProgressEvent::Finish, 1);
        QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), finishEvent);
        QCoreApplication::processEvents();
    }

finish_thread:
    this->exit(0);
    this->deleteLater();
}

void ImageLoadingThread::setMaximumProgress(double limit)
{
    if (limit > 1)
        limit = 1;
    d->m_max_progress = limit;
}

void ImageLoadingThread::setImageUrl(const QUrl& url)
{
    d->m_sem.acquire();
    d->m_urls.clear();
    d->m_urls.append(url);
    d->m_sem.release();
}

void ImageLoadingThread::setImagesUrls(const QList<QUrl>& urls)
{
    d->m_sem.acquire();
    d->m_urls = urls;
    d->m_sem.release();
}

void ImageLoadingThread::loadRaw(const QUrl& url)
{
    ProgressEvent* loadingImageActionEvent = new ProgressEvent(this);
    loadingImageActionEvent->setData(ProgressEvent::ActionUpdate, QVariant( i18n("Loading ").append(url.fileName()) ));
    QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), loadingImageActionEvent);
    QCoreApplication::processEvents();

    RAWLoader* loader = new RAWLoader(this);
    loader->setMaxDataProgress(d->m_max_progress * 0.7);
    DRawDecoderSettings settings;
    QByteArray ba;
    int width;
    int height;
    int rgbamax;
    QImage img;

    bool b = loader->decodeRAWImage(url.path(), settings, ba, width, height, rgbamax);

    if (b)
    {
        ProgressEvent * buildImageEvent = new ProgressEvent(this);
        buildImageEvent->setData(ProgressEvent::ActionUpdate, QVariant( i18n("Decoding image") ));
        QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), buildImageEvent);
        QCoreApplication::processEvents();

        uchar* image = new uchar[width*height*4];
        if (image)
        {
            uchar* dst   = image;
            uchar* src   = (uchar*)ba.data();

            for (int h = 0; h < height; ++h)
            {
                ProgressEvent * event = new ProgressEvent(this);
                event->setData(ProgressEvent::ProgressUpdate, d->m_max_progress * (0.7 + 0.3 * (((float)h)/((float)height))) );
                QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), event);
                QCoreApplication::processEvents();

                for (int w = 0; w < width; ++w)
                {
                    // No need to adapt RGB components accordingly with rgbmax value because dcraw
                    // always return rgbmax to 255 in 8 bits/color/pixels.

                    dst[0] = src[2];    // Blue
                    dst[1] = src[1];    // Green
                    dst[2] = src[0];    // Red
                    dst[3] = 0xFF;      // Alpha

                    dst += 4;
                    src += 3;
                }
            }

            img = QImage(width, height, QImage::Format_ARGB32);

            uchar* sptr = image;
            uint*  dptr = reinterpret_cast<uint*>(img.bits());

            uint dim = width * height;

            for (uint i = 0; i < dim; ++i)
            {
                *dptr++ = qRgba(sptr[2], sptr[1], sptr[0], sptr[3]);
                sptr += 4;
            }
        }
        else
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Failed to allocate memory for loading raw file";
        }

        ProgressEvent* emitEvent = new ProgressEvent(this);
        emitEvent->setData(ProgressEvent::ActionUpdate, QVariant( i18n("Finishing...") ));
        QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), emitEvent);
        QCoreApplication::processEvents();

        delete [] image;
    }

    emit imageLoaded(url, img);
    delete loader;
}

void ImageLoadingThread::loadImage(const QUrl& url)
{
    ProgressEvent* loadingImageActionEvent = new ProgressEvent(this);
    loadingImageActionEvent->setData(ProgressEvent::ActionUpdate, QVariant( i18n("Loading ").append(url.fileName()) ));
    QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), loadingImageActionEvent);
    QCoreApplication::processEvents();

    QFile f(url.path());
    f.open(QIODevice::ReadOnly);
    QByteArray ba;
    QBuffer bf(&ba);
    bf.open(QIODevice::WriteOnly);
    QByteArray temp;
    int s = f.size() / 10;
    s = s < 1000 ? 1000 : s;

    do
    {
        temp = f.read(s);
        d->m_loaded_bytes += temp.size();
        bf.write(temp.data(), temp.size());
        this->yieldCurrentThread();
        ProgressEvent* event = new ProgressEvent(this);
        event->setData(ProgressEvent::ProgressUpdate, (d->m_loaded_bytes * d->m_max_progress) / (d->m_size * 1.4));
        QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), event);
        QCoreApplication::processEvents();
    }
    while (temp.size() == s);

    f.close();
    bf.close();

    ProgressEvent* buildImageEvent = new ProgressEvent(this);
    buildImageEvent->setData(ProgressEvent::ActionUpdate, QVariant( i18n("Decoding image") ));
    QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), buildImageEvent);
    QCoreApplication::processEvents();

    QImage img = QImage::fromData(ba);

    ProgressEvent* emitEvent = new ProgressEvent(this);
    emitEvent->setData(ProgressEvent::ActionUpdate, QVariant( i18n("Finishing...") ));
    QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), emitEvent);
    QCoreApplication::processEvents();

    emit imageLoaded(url, img);
}

} // namespace PhotoLayoutsEditor

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-14
 * Description : a presentation tool.
 *
 * Copyright (C) 2007-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * Copyright (C) 2012-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Parts of this code are based on smoothslidesaver by Carsten Weinhold
 * <carsten dot weinhold at gmx dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "kbimageloader.h"

// Qt includes

#include <QMatrix>
#include <QFileInfo>

// Local includes

#include "dimg.h"
#include "iccsettings.h"
#include "digikam_debug.h"
#include "presentationkb.h"
#include "previewloadthread.h"
#include "iccsettingscontainer.h"
#include "presentationcontainer.h"

using namespace Digikam;

namespace DigikamGenericPresentationPlugin
{

class Q_DECL_HIDDEN KBImageLoader::Private
{

public:

    explicit Private()
    {
        sharedData    = 0;
        fileIndex     = 0;
        width         = 0;
        height        = 0;
        initialized   = false;
        needImage     = true;
        haveImages    = false;
        quitRequested = false;
        textureAspect = 0.0;
    }

    PresentationContainer* sharedData;
    int                    fileIndex;

    int                    width;
    int                    height;

    QWaitCondition         imageRequest;
    QMutex                 condLock;
    QMutex                 imageLock;

    bool                   initialized;
    bool                   needImage;
    bool                   haveImages;
    bool                   quitRequested;

    float                  textureAspect;
    QImage                 texture;

    IccProfile             iccProfile;
};

KBImageLoader::KBImageLoader(PresentationContainer* const sharedData, int width, int height)
    : QThread(),
      d(new Private)
{
    d->sharedData = sharedData;
    d->width      = width;
    d->height     = height;

    ICCSettingsContainer settings = IccSettings::instance()->settings();

    if (settings.enableCM && settings.useManagedPreviews)
    {
        d->iccProfile = IccProfile(settings.monitorProfile);
    }
}

KBImageLoader::~KBImageLoader()
{
    delete d;
}

void KBImageLoader::quit()
{
    QMutexLocker locker(&d->condLock);

    d->quitRequested = true;
    d->imageRequest.wakeOne();
}

void KBImageLoader::requestNewImage()
{
    QMutexLocker locker(&d->condLock);

    if (!d->needImage)
    {
        d->needImage = true;
        d->imageRequest.wakeOne();
    }
}

void KBImageLoader::run()
{
    QMutexLocker locker(&d->condLock);

    // we enter the loop with d->needImage==true, so we will immediately
    // try to load an image

    while (true)
    {
        if (d->quitRequested)
            break;

        if (d->needImage)
        {
            if (d->fileIndex == (int)d->sharedData->urlList.count())
            {
                if (d->sharedData->loop)
                {
                    d->fileIndex = 0;
                }
                else
                {
                    d->needImage  = false;
                    d->haveImages = false;
                    continue;
                }
            }

            d->needImage = false;
            d->condLock.unlock();
            bool ok;

            do
            {
                ok = loadImage();

                if (!ok)
                    invalidateCurrentImageName();
            }
            while (!ok && d->fileIndex < (int)d->sharedData->urlList.count());

            if (d->fileIndex == (int)d->sharedData->urlList.count())
            {
                d->condLock.lock();
                continue;
            }

            if (!ok)
            {
                // generate a black dummy image
                d->texture = QImage(128, 128, QImage::Format_ARGB32);
                d->texture.fill(Qt::black);
            }

            d->condLock.lock();

            d->fileIndex++;

            if (!d->initialized)
            {
                d->haveImages  = ok;
                d->initialized = true;
            }
        }
        else
        {
            // wait for new requests from the consumer
            d->imageRequest.wait(&d->condLock);
        }
    }
}

bool KBImageLoader::loadImage()
{
    QString path  = d->sharedData->urlList[d->fileIndex].toLocalFile();
    QImage  image = PreviewLoadThread::loadHighQualitySynchronously(path,
                                                                    PreviewSettings::RawPreviewAutomatic,
                                                                    d->iccProfile).copyQImage();

    if (image.isNull())
    {
        return false;
    }

    d->imageLock.lock();

    d->textureAspect = (float)image.width() / (float)image.height();
    d->texture       = image.scaled(d->width, d->height,
                                    Qt::KeepAspectRatio, Qt::SmoothTransformation);

    d->imageLock.unlock();

    return true;
}

void KBImageLoader::invalidateCurrentImageName()
{
    d->sharedData->urlList.removeAll(d->sharedData->urlList[d->fileIndex]);
    d->fileIndex++;
}

bool KBImageLoader::grabImage()
{
    d->imageLock.lock();
    return d->haveImages;
}

void KBImageLoader::ungrabImage()
{
    d->imageLock.unlock();
}

bool KBImageLoader::ready() const
{
    return d->initialized;
}

const QImage& KBImageLoader::image() const
{
    return d->texture;
}

float KBImageLoader::imageAspect() const
{
    return d->textureAspect;
}

} // namespace DigikamGenericPresentationPlugin

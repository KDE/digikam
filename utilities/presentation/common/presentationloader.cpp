/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-11
 * Description : a presentation tool.
 *
 * Copyright (C) 2007-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "presentationloader.h"

// Qt includes

#include <QMap>
#include <QImage>
#include <QMatrix>
#include <QPainter>
#include <QThread>
#include <QMutex>
#include <QFileInfo>

// Local includes

#include "dimg.h"
#include "digikam_debug.h"
#include "previewloadthread.h"
#include "presentationcontainer.h"

namespace Digikam
{

typedef QMap<QUrl, QImage> LoadedImages;

class LoadThread : public QThread
{

public:

    LoadThread(LoadedImages* const loadedImages, QMutex* const imageLock, const QUrl& path,
               int width, int height)
    {
        m_path         = path;
        m_swidth       = width;
        m_sheight      = height;
        m_imageLock    = imageLock;
        m_loadedImages = loadedImages;
    }

    ~LoadThread()
    {
    }

protected:

    void run()
    {
        QImage newImage;

        newImage = PreviewLoadThread::loadHighQualitySynchronously(m_path.toLocalFile()).copyQImage();

        m_imageLock->lock();
        m_loadedImages->insert(m_path, newImage.scaled(m_swidth,
                                                       m_sheight,
                                                       Qt::KeepAspectRatio,
                                                       Qt::SmoothTransformation));
        m_imageLock->unlock();
    }

private:

    QMutex*       m_imageLock;
    LoadedImages* m_loadedImages;
    QUrl          m_path;
    QString       m_filename;
    int           m_swidth;
    int           m_sheight;
};

typedef QMap<QUrl, LoadThread*> LoadingThreads;

// -----------------------------------------------------------------------------------------

class PresentationLoader::Private
{

public:

    Private()
    {
        sharedData     = 0;
        loadingThreads = 0;
        loadedImages   = 0;
        imageLock      = 0;
        threadLock     = 0;
        cacheSize      = 0;
        currIndex      = 0;
        swidth         = 0;
        sheight        = 0;
    }

    PresentationContainer* sharedData;
    LoadingThreads*        loadingThreads;
    LoadedImages*          loadedImages;

    QMutex*                imageLock;
    QMutex*                threadLock;

    uint                   cacheSize;
    int                    currIndex;
    int                    swidth;
    int                    sheight;
};

PresentationLoader::PresentationLoader(PresentationContainer* const sharedData, int width, int height,
                                       int beginAtIndex)
    : d(new Private)
{
    d->sharedData     = sharedData;
    d->currIndex      = beginAtIndex;
    d->cacheSize      = d->sharedData->enableCache ? d->sharedData->cacheSize : 1;
    d->swidth         = width;
    d->sheight        = height;
    d->loadingThreads = new LoadingThreads();
    d->loadedImages   = new LoadedImages();
    d->imageLock      = new QMutex();
    d->threadLock     = new QMutex();

    QUrl filePath;

    for (uint i = 0; i < uint(d->cacheSize / 2) && i < uint(d->sharedData->urlList.count()); ++i)
    {
        filePath                    = d->sharedData->urlList[i];
        LoadThread* const newThread = new LoadThread(d->loadedImages, d->imageLock,
                                                     filePath, d->swidth, d->sheight);
        d->threadLock->lock();
        d->loadingThreads->insert(filePath, newThread);
        newThread->start();
        d->threadLock->unlock();
    }

    for (uint i = 0; i < (d->cacheSize % 2 == 0 ? (d->cacheSize % 2) : uint(d->cacheSize / 2) + 1); ++i)
    {
        int toLoad                  = (d->currIndex - i) % d->sharedData->urlList.count();
        filePath                    = d->sharedData->urlList[toLoad];
        LoadThread* const newThread = new LoadThread(d->loadedImages, d->imageLock,
                                                     filePath, d->swidth, d->sheight);
        d->threadLock->lock();
        d->loadingThreads->insert(filePath, newThread);
        newThread->start();
        d->threadLock->unlock();
    }
}

PresentationLoader::~PresentationLoader()
{
    d->threadLock->lock();
    LoadingThreads::Iterator it;

    for (it = d->loadingThreads->begin(); it != d->loadingThreads->end(); ++it)
    {
        // better check for a valid pointer here
        if (it.value())
            it.value()->wait();

        delete it.value();
    }

    d->loadingThreads->clear();

    d->threadLock->unlock();

    delete d->loadedImages;
    delete d->loadingThreads;
    delete d->imageLock;
    delete d->threadLock;
    delete d;
}

void PresentationLoader::next()
{
    int victim   = (d->currIndex - (d->cacheSize % 2 == 0 ? (d->cacheSize / 2) - 1
                                                          :  int(d->cacheSize / 2))) % d->sharedData->urlList.count();

    int newBorn  = (d->currIndex + int(d->cacheSize / 2) + 1) % d->sharedData->urlList.count();
    d->currIndex = (d->currIndex + 1) % d->sharedData->urlList.count();

    if (victim == newBorn)
        return;

    d->threadLock->lock();

    LoadThread* const oldThread = d->loadingThreads->value(d->sharedData->urlList[victim]);

    if (oldThread)
        oldThread->wait();

    delete oldThread;

    d->loadingThreads->remove(d->sharedData->urlList[victim]);
    d->imageLock->lock();
    d->loadedImages->remove(d->sharedData->urlList[victim]);
    d->imageLock->unlock();
    d->threadLock->unlock();

    QUrl filePath               = d->sharedData->urlList[newBorn];
    LoadThread* const newThread = new LoadThread(d->loadedImages, d->imageLock, filePath, d->swidth, d->sheight);

    d->threadLock->lock();

    d->loadingThreads->insert(filePath, newThread);
    newThread->start();

    d->threadLock->unlock();
}

void PresentationLoader::prev()
{
    int victim   = (d->currIndex + int(d->currIndex / 2)) % d->sharedData->urlList.count();
    int newBorn  = (d->currIndex - ((d->cacheSize & 2) == 0 ? (d->cacheSize / 2)
                                                            : int(d->cacheSize / 2) + 1)) % d->sharedData->urlList.count();

    d->currIndex = d->currIndex > 0 ? d->currIndex - 1 : d->sharedData->urlList.count() - 1;

    if (victim == newBorn)
        return;

    d->threadLock->lock();
    d->imageLock->lock();

    LoadThread* const oldThread = d->loadingThreads->value(d->sharedData->urlList[victim]);

    if (oldThread)
        oldThread->wait();

    delete oldThread;

    d->loadingThreads->remove(d->sharedData->urlList[victim]);
    d->loadedImages->remove(d->sharedData->urlList[victim]);

    d->imageLock->unlock();
    d->threadLock->unlock();

    QUrl filePath               = d->sharedData->urlList[newBorn];
    LoadThread* const newThread = new LoadThread(d->loadedImages, d->imageLock, filePath, d->swidth, d->sheight);

    d->threadLock->lock();

    d->loadingThreads->insert(filePath, newThread);
    newThread->start();

    d->threadLock->unlock();
}

QImage PresentationLoader::getCurrent() const
{
    checkIsIn(d->currIndex);
    d->imageLock->lock();
    QImage returned = (*d->loadedImages)[d->sharedData->urlList[d->currIndex]];
    d->imageLock->unlock();

    return returned;
}

QString PresentationLoader::currFileName() const
{
    return d->sharedData->urlList[d->currIndex].fileName();
}

QUrl PresentationLoader::currPath() const
{
    return d->sharedData->urlList[d->currIndex];
}

void PresentationLoader::checkIsIn(int index) const
{
    d->threadLock->lock();

    if (d->loadingThreads->contains(d->sharedData->urlList[index]))
    {
        if ((*d->loadingThreads)[d->sharedData->urlList[index]]->isRunning())
            (*d->loadingThreads)[d->sharedData->urlList[index]]->wait();

        d->threadLock->unlock();
    }
    else
    {
        QUrl filePath               = d->sharedData->urlList[index];
        LoadThread* const newThread = new LoadThread(d->loadedImages, d->imageLock, filePath, d->swidth, d->sheight);

        d->loadingThreads->insert(d->sharedData->urlList[index], newThread);
        newThread->start();
        (*d->loadingThreads)[d->sharedData->urlList[index]]->wait();
        d->threadLock->unlock();
    }
}

} // namespace Digikam

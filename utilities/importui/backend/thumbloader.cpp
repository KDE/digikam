#include <QThread>
#include <QTest>

#include <QMutex>
#include <QImage>
#include <kdebug.h>

#include "thumbloader.h"
#include "dkcamera.h"
#include "umscamera.h"
#include "gpcamera.h"
#include "cameracontroller.h"

namespace Digikam
{

class PixWorker::PixWorkerPriv
{
public:
    PixWorkerPriv()
    {
        stop = false;
    }
    DKCamera* cam;
    QHash<QString, QImage>* localPix;
    QMutex* localPixLock;
    bool stop;
};

PixWorker::PixWorker(QHash<QString, QImage>* localPix, QMutex* localPixLock)
          : QObject(), d(new PixWorkerPriv())
{
    d->localPix = localPix;
    d->localPixLock = localPixLock;
}

void PixWorker::setDKCamera(DKCamera* cam)
{
    d->cam = cam;
}

void PixWorker::stop()
{
    d->stop = true;
}

void PixWorker::doWork(CamItemInfo camItem, int thumbSize)
{

    if(d->stop)
        return;
    QImage thumbnail;
    QString path = camItem.url().prettyUrl();

    if (d->cam->getThumbnail(camItem.folder, camItem.name, thumbnail))
    {
        thumbnail = thumbnail.scaled(thumbSize, thumbSize, Qt::KeepAspectRatio);
        d->localPixLock->lock();
        d->localPix->insert(path, thumbnail);
        d->localPixLock->unlock();
        emit resultReady(camItem.url());
    }

}


class ThumbLoader::ThumbLoaderPriv
{
public:
    ThumbLoaderPriv()
    {
        cam = 0;
    }
    DKCamera* cam;
    QThread* thread;
    QHash<QString, QImage>* localPix;
    PixWorker* pworker;
    QMutex* localPixLock;
};

ThumbLoader::ThumbLoader(QObject* const parent,
                         QString title, QString model,
                         QString port, QString path)
            : QObject(parent),d(new ThumbLoaderPriv())
{
    Q_UNUSED(title);
    Q_UNUSED(model);
    Q_UNUSED(port);
    Q_UNUSED(path);
    d->thread = new QThread();
    d->localPixLock = new QMutex();
    d->localPix = new QHash<QString, QImage>();
    d->pworker = new PixWorker(d->localPix, d->localPixLock);

    d->pworker->moveToThread(d->thread);
    connect(d->thread, SIGNAL(finished()),
            d->pworker, SLOT(deleteLater()));
    connect(this, SIGNAL(signalStartWork(CamItemInfo,int)),
            d->pworker, SLOT(doWork(CamItemInfo, int)),Qt::QueuedConnection);
    connect(d->pworker, SIGNAL(resultReady(KUrl)),
            this, SIGNAL(signalUpdateModel(KUrl)));

    d->thread->start();
}

ThumbLoader::~ThumbLoader()
{
    d->pworker->stop();
    d->thread->quit();
    d->thread->wait();

    delete d->thread;
    delete d->localPix;
    delete d->localPixLock;
}

QImage ThumbLoader::getThumbnail(CamItemInfo camInfo, int thSize)
{
    QString path = camInfo.url().prettyUrl();
    QMutexLocker(d->localPixLock);
    if(d->localPix->contains(path))
    {
        return d->localPix->value(path);
    }
    else
    {
        d->localPix->insert(path, QImage());
        emit signalStartWork(camInfo,thSize);
    }
    return QImage();
}

void ThumbLoader::setDKCamera(CameraController* cam)
{
    d->pworker->setDKCamera(cam->getDKCamera());
}

void ThumbLoader::handleResult(KUrl result)
{
    kDebug() << "Got the result " << result;
    emit signalUpdateModel(result);
}

} // namespace Digikam

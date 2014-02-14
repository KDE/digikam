#include <QThread>
#include <QTest>

#include <kdebug.h>

#include "thumbloader.h"
#include "dkcamera.h"
#include "umscamera.h"
#include "gpcamera.h"

namespace Digikam
{

PixWorker::PixWorker(): QObject()
{

}

void PixWorker::doWork(QString result)
{
    QTest::qSleep(1000);
    emit resultReady(result + QString("gata"));
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
    QHash<KUrl, QPixmap> localPix;
    PixWorker* pworker;

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
    d->pworker = new PixWorker();
    d->pworker->moveToThread(d->thread);
    connect(d->thread, SIGNAL(finished()), d->pworker, SLOT(deleteLater()));
    connect(this, SIGNAL(signalStartWork(QString)), d->pworker, SLOT(doWork(QString)),Qt::QueuedConnection);
    connect(d->pworker, SIGNAL(resultReady(QString)), this, SLOT(handleResult(QString)));
    d->thread->start();
}

ThumbLoader::~ThumbLoader()
{
    d->thread->quit();
    kDebug() << "ThumbLoader destructor";
    /**
     * Remember to solve the problem with Q Thread exit. Terminate is not acceptable
     */
    d->thread->terminate();
         d->thread->quit();
         d->thread->wait();
}

void ThumbLoader::addToWork(QString data)
{
    kDebug() << "Adding work " << data;
    emit signalStartWork(data);
}

void ThumbLoader::handleResult(QString result)
{
    kDebug() << "Got the result " << result;
}

} // namespace Digikam

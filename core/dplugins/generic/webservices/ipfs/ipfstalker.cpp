/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-12
 * Description : a tool to export images to IPFS web service
 *
 * Copyright (C) 2018 by Amar Lakshya <amar dot lakshya at xaviers dot edu dot in>
 * Copyright (C) 2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#include "ipfstalker.h"

// Qt includes

#include <QQueue>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimerEvent>
#include <QUrlQuery>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "o0settingsstore.h"
#include "o0globals.h"

namespace DigikamGenericIpfsPlugin
{

static const QString ipfs_upload_url = QLatin1String("https://api.globalupload.io/transport/add");

class Q_DECL_HIDDEN IpfsTalker::Private
{
public:

    explicit Private()
    {
        workTimer = 0;
        reply     = nullptr;
        image     = nullptr;
    }

    // Work queue
    QQueue<IpfsTalkerAction>     workQueue;

    // ID of timer triggering on idle (0ms).
    int                          workTimer;

    // Current QNetworkReply
    QNetworkReply*               reply;

    // Current image being uploaded
    QFile*                       image;

    // The QNetworkAccessManager used for connections
    QNetworkAccessManager        netMngr;
};

IpfsTalker::IpfsTalker(QObject* const parent)
    : QObject(parent),
      d(new Private)
{
}

IpfsTalker::~IpfsTalker()
{
    // Disconnect all signals as cancelAllWork may emit
    disconnect(this, nullptr, nullptr, nullptr);
    cancelAllWork();
    delete d;
}

unsigned int IpfsTalker::workQueueLength()
{
    return d->workQueue.size();
}

void IpfsTalker::queueWork(const IpfsTalkerAction& action)
{
    d->workQueue.enqueue(action);
    startWorkTimer();
}

void IpfsTalker::cancelAllWork()
{
    stopWorkTimer();

    if (d->reply)
        d->reply->abort();

    // Should error be emitted for those actions?
    while (!d->workQueue.empty())
        d->workQueue.dequeue();
}

void IpfsTalker::uploadProgress(qint64 sent, qint64 total)
{
    if (total > 0) // Don't divide by 0
        emit progress((sent * 100) / total, d->workQueue.first());
}

void IpfsTalker::replyFinished()
{
    auto* reply = d->reply;
    reply->deleteLater();
    d->reply     = nullptr;

    if (d->image)
    {
        delete d->image;
        d->image = nullptr;
    }

    if (d->workQueue.empty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Received result without request";
        return;
    }

    // toInt() returns 0 if conversion fails. That fits nicely already.
    int code      = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    auto response = QJsonDocument::fromJson(reply->readAll());

    if (code == 200 && !response.isEmpty())
    {
        /* Success! */
        IpfsTalkerResult result;
        result.action = &d->workQueue.first();

        switch (result.action->type)
        {
            case IpfsTalkerActionType::IMG_UPLOAD:
                result.image.name = response.object()[QLatin1String("Name")].toString();
                result.image.size = response.object()[QLatin1String("Size")].toInt();
                result.image.url  = QLatin1String("https://ipfs.io/ipfs/") + response.object()[QLatin1String("Hash")].toString();
                break;
            default:
                qCWarning(DIGIKAM_WEBSERVICES_LOG) << "Unexpected action";
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << response.toJson();
                break;
        }

        emit success(result);
    }
    else
    {
        if (code == 403)
        {
            /* HTTP 403 Forbidden -> Invalid token?
             * That needs to be handled internally, so don't emit progress
             * and keep the action in the queue for later retries.
             */
            return;
        }
        else
        {
            /* Failed.
             */
            auto msg = response.object()[QLatin1String("data")]
                       .toObject()[QLatin1String("error")]
                       .toString(QLatin1String("Could not read response."));

            emit error(msg, d->workQueue.first());
        }
    }

    // Next work item.
    d->workQueue.dequeue();
    startWorkTimer();
}

void IpfsTalker::timerEvent(QTimerEvent* event)
{
    if (event->timerId() != d->workTimer)
        return QObject::timerEvent(event);

    event->accept();

    // One-shot only.
    QObject::killTimer(event->timerId());
    d->workTimer = 0;

    doWork();
}

void IpfsTalker::startWorkTimer()
{
    if (!d->workQueue.empty() && d->workTimer == 0)
    {
        d->workTimer = QObject::startTimer(0);
        emit busy(true);
    }
    else
    {
        emit busy(false);
    }
}

void IpfsTalker::stopWorkTimer()
{
    if (d->workTimer != 0)
    {
        QObject::killTimer(d->workTimer);
        d->workTimer = 0;
    }
}

void IpfsTalker::doWork()
{
    if (d->workQueue.empty() || d->reply != nullptr)
        return;

    auto &work = d->workQueue.first();

    switch (work.type)
    {
        case IpfsTalkerActionType::IMG_UPLOAD:
        {
            d->image = new QFile(work.upload.imgpath);

            if (!d->image->open(QIODevice::ReadOnly))
            {
                delete d->image;
                d->image = nullptr;

                /* Failed. */
                emit error(i18n("Could not open file"), d->workQueue.first());

                d->workQueue.dequeue();
                return doWork();
            }

            /* Set ownership to d->image to delete that as well. */
            auto* multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType, d->image);
            QHttpPart title;
            title.setHeader(QNetworkRequest::ContentDispositionHeader,
                            QLatin1String("form-data; name=\"keyphrase\""));
            multipart->append(title);

            QHttpPart description;
            description.setHeader(QNetworkRequest::ContentDispositionHeader,
                                  QLatin1String("form-data; name=\"user\""));
            multipart->append(description);

            QHttpPart image;
            image.setHeader(QNetworkRequest::ContentDispositionHeader,
                            QVariant(QString::fromLatin1("form-data; name=\"file\";  filename=\"%1\"")
                            .arg(QLatin1String(QFileInfo(work.upload.imgpath).fileName().toUtf8().toPercentEncoding()))));
            image.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("image/jpeg"));
            image.setBodyDevice(d->image);
            multipart->append(image);
            QNetworkRequest request(QUrl(QLatin1String("https://api.globalupload.io/transport/add")));
            d->reply = d->netMngr.post(request, multipart);

            break;
        }
    }

    if (d->reply)
    {
        connect(d->reply, &QNetworkReply::uploadProgress,
                this, &IpfsTalker::uploadProgress);

        connect(d->reply, &QNetworkReply::finished,
                this, &IpfsTalker::replyFinished);
    }
}

} // namespace DigikamGenericIpfsPlugin

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-05-27
 * Description : Implementation of v3 of the Imgur API
 *
 * Copyright (C) 2016      by Fabian Vogt <fabian at ritter dash vogt dot de>
 * Copyright (C) 2016-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#include "imgurtalker.h"

// Qt includes

#include <QFileInfo>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimerEvent>
#include <QUrlQuery>
#include <QNetworkAccessManager>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dinfointerface.h"
#include "digikam_debug.h"
#include "wstoolutils.h"
#include "o0settingsstore.h"
#include "o0globals.h"

namespace Digikam
{

static const QString imgur_auth_url       = QLatin1String("https://api.imgur.com/oauth2/authorize"),
imgur_token_url                           = QLatin1String("https://api.imgur.com/oauth2/token");
static const uint16_t imgur_redirect_port = 8000; // Redirect URI is http://127.0.0.1:8000

class ImgurTalker::Private
{
public:

    explicit Private()
    {
        workTimer = 0;
        reply     = 0;
        image     = 0;
    }

    /* Handler for OAuth 2 related requests.
     */
    O2                            auth;

    /* Work queue.
     */
    std::queue<ImgurTalkerAction> workQueue;

    /* ID of timer triggering on idle (0ms).
     */
    int                           workTimer;

    /* Current QNetworkReply instance.
     */
    QNetworkReply*                reply;

    /* Current image being uploaded.
     */
    QFile*                        image;

    /* The QNetworkAccessManager instance used for connections.
     */
    QNetworkAccessManager         net;
};

ImgurTalker::ImgurTalker(const QString& client_id,
                         const QString& client_secret,
                         QObject* const parent)
    : QObject(parent),
      d(new Private)
{
    d->auth.setClientId(client_id);
    d->auth.setClientSecret(client_secret);
    d->auth.setRequestUrl(imgur_auth_url);
    d->auth.setTokenUrl(imgur_token_url);
    d->auth.setRefreshTokenUrl(imgur_token_url);
    d->auth.setLocalPort(imgur_redirect_port);
    d->auth.setLocalhostPolicy(QString());

    QSettings* const settings    = WSToolUtils::getOauthSettings(this);
    O0SettingsStore* const store = new O0SettingsStore(settings,
                                                       QLatin1String(O2_ENCRYPTION_KEY), this);
    store->setGroupKey(QLatin1String("Imgur"));
    d->auth.setStore(store);

    connect(&d->auth, &O2::linkedChanged,
            this, &ImgurTalker::slotOauthAuthorized);

    connect(&d->auth, &O2::openBrowser,
            this, &ImgurTalker::slotOauthRequestPin);

    connect(&d->auth, &O2::linkingFailed,
            this, &ImgurTalker::slotOauthFailed);
}

ImgurTalker::~ImgurTalker()
{
    // Disconnect all signals as cancelAllWork may emit.
    disconnect(this, 0, 0, 0);
    cancelAllWork();

    delete d;
}

O2& ImgurTalker::getAuth()
{
    return d->auth;
}

unsigned int ImgurTalker::workQueueLength()
{
    return d->workQueue.size();
}

void ImgurTalker::queueWork(const ImgurTalkerAction& action)
{
    d->workQueue.push(action);
    startWorkTimer();
}

void ImgurTalker::cancelAllWork()
{
    stopWorkTimer();

    if (d->reply)
        d->reply->abort();

    // Should signalError be emitted for those actions?
    while (!d->workQueue.empty())
        d->workQueue.pop();
}

QUrl ImgurTalker::urlForDeletehash(const QString& deletehash)
{
    return QUrl{QLatin1String("https://imgur.com/delete/") + deletehash};
}

void ImgurTalker::slotOauthAuthorized()
{
    bool success = d->auth.linked();

    if (success)
        startWorkTimer();
    else
        emit signalBusy(false);

    emit signalAuthorized(success,
                          d->auth.extraTokens()[QLatin1String("account_username")].toString());
}

void ImgurTalker::slotOauthRequestPin(const QUrl& url)
{
    emit signalBusy(false);
    emit signalRequestPin(url);
}

void ImgurTalker::slotOauthFailed()
{
    cancelAllWork();
    emit signalAuthError(i18n("Could not authorize"));
}

void ImgurTalker::slotUploadProgress(qint64 sent, qint64 total)
{
    // Don't divide by 0
    if (total > 0)
    {
        emit signalProgress((sent * 100) / total, d->workQueue.front());
    }
}

void ImgurTalker::slotReplyFinished()
{
    auto* const reply = d->reply;
    reply->deleteLater();
    d->reply          = nullptr;

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

    // NOTE: toInt() returns 0 if conversion fails. That fits nicely already.
    int code      = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    auto response = QJsonDocument::fromJson(reply->readAll());

    if (code == 200 && !response.isEmpty())
    {
        // Success!
        ImgurTalkerResult result;
        result.action = &d->workQueue.front();
        auto data     = response.object()[QLatin1String("data")].toObject();

        switch (result.action->type)
        {
            case ImgurTalkerActionType::IMG_UPLOAD:
            case ImgurTalkerActionType::ANON_IMG_UPLOAD:
                result.image.animated    = data[QLatin1String("animated")].toBool();
                result.image.bandwidth   = data[QLatin1String("bandwidth")].toInt();
                result.image.datetime    = data[QLatin1String("datetime")].toInt();
                result.image.deletehash  = data[QLatin1String("deletehash")].toString();
                result.image.description = data[QLatin1String("description")].toString();
                result.image.height      = data[QLatin1String("height")].toInt();
                result.image.hash        = data[QLatin1String("id")].toString();
                result.image.name        = data[QLatin1String("name")].toString();
                result.image.size        = data[QLatin1String("size")].toInt();
                result.image.title       = data[QLatin1String("title")].toString();
                result.image.type        = data[QLatin1String("type")].toString();
                result.image.url         = data[QLatin1String("link")].toString();
                result.image.views       = data[QLatin1String("views")].toInt();
                result.image.width       = data[QLatin1String("width")].toInt();
                break;
            case ImgurTalkerActionType::ACCT_INFO:
                result.account.username = data[QLatin1String("url")].toString();
                // TODO: Other fields.
                break;
            default:
                qCWarning(DIGIKAM_WEBSERVICES_LOG) << "Unexpected action";
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << response.toJson();
                break;
        }

        emit signalSuccess(result);
    }
    else
    {
        if (code == 403)
        {
            /* HTTP 403 Forbidden -> Invalid token? 
             * That needs to be handled internally, so don't emit signalProgress
             * and keep the action in the queue for later retries.
             */
            d->auth.refresh();
            return;
        }
        else
        {
            // Failed.
            auto msg = response.object()[QLatin1String("data")]
                       .toObject()[QLatin1String("error")]
                       .toString(QLatin1String("Could not read response."));

            emit signalError(msg, d->workQueue.front());
        }
    }

    // Next work item.
    d->workQueue.pop();
    startWorkTimer();
}

void ImgurTalker::timerEvent(QTimerEvent* event)
{
    if (event->timerId() != d->workTimer)
        return QObject::timerEvent(event);

    event->accept();

    // One-shot only.
    QObject::killTimer(event->timerId());
    d->workTimer = 0;

    doWork();
}

void ImgurTalker::startWorkTimer()
{
    if (!d->workQueue.empty() && d->workTimer == 0)
    {
        d->workTimer = QObject::startTimer(0);
        emit signalBusy(true);
    }
    else
    {
        emit signalBusy(false);
    }
}

void ImgurTalker::stopWorkTimer()
{
    if (d->workTimer != 0)
    {
        QObject::killTimer(d->workTimer);
        d->workTimer = 0;
    }
}

void ImgurTalker::addAuthToken(QNetworkRequest* request)
{
    request->setRawHeader(QByteArray("Authorization"),
                          QString::fromLatin1("Bearer %1").arg(d->auth.token()).toUtf8());
}

void ImgurTalker::addAnonToken(QNetworkRequest* request)
{
    request->setRawHeader(QByteArray("Authorization"),
                          QString::fromLatin1("Client-ID %1").arg(d->auth.clientId()).toUtf8());
}

void ImgurTalker::doWork()
{
    if (d->workQueue.empty() || d->reply != nullptr)
        return;

    auto &work = d->workQueue.front();

    if (work.type != ImgurTalkerActionType::ANON_IMG_UPLOAD && !d->auth.linked())
    {
        d->auth.link();
        return; // Wait for the signalAuthorized() signal.
    }

    switch(work.type)
    {
        case ImgurTalkerActionType::ACCT_INFO:
        {
            QNetworkRequest request(QUrl(QString::fromLatin1("https://api.imgur.com/3/account/%1")
                                        .arg(QLatin1String(work.account.username.toUtf8().toPercentEncoding()))));
            addAuthToken(&request);

            d->reply = d->net.get(request);
            break;
        }
        case ImgurTalkerActionType::ANON_IMG_UPLOAD:
        case ImgurTalkerActionType::IMG_UPLOAD:
        {
            d->image = new QFile(work.upload.imgpath);

            if (!d->image->open(QIODevice::ReadOnly))
            {
                delete d->image;
                d->image = nullptr;

                // Failed.
                emit signalError(i18n("Could not open file"), d->workQueue.front());

                d->workQueue.pop();
                return doWork();
            }

            // Set ownership to d->image to delete that as well.
            auto* multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType, d->image);
            QHttpPart title;
            title.setHeader(QNetworkRequest::ContentDispositionHeader,
                            QLatin1String("form-data; name=\"title\""));
            title.setBody(work.upload.title.toUtf8().toPercentEncoding());
            multipart->append(title);

            QHttpPart description;
            description.setHeader(QNetworkRequest::ContentDispositionHeader,
                                  QLatin1String("form-data; name=\"description\""));
            description.setBody(work.upload.description.toUtf8().toPercentEncoding());
            multipart->append(description);

            QHttpPart image;
            image.setHeader(QNetworkRequest::ContentDispositionHeader,
                            QVariant(QString::fromLatin1("form-data; name=\"image\"; filename=\"%1\"")
                            .arg(QLatin1String(QFileInfo(work.upload.imgpath).fileName().toUtf8().toPercentEncoding()))));
            image.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/octet-stream"));
            image.setBodyDevice(d->image);
            multipart->append(image);

            QNetworkRequest request(QUrl(QLatin1String("https://api.imgur.com/3/image")));

            if (work.type == ImgurTalkerActionType::IMG_UPLOAD)
                addAuthToken(&request);
            else
                addAnonToken(&request);

            d->reply = d->net.post(request, multipart);

            break;
        }
    }

    if (d->reply)
    {
        connect(d->reply, &QNetworkReply::uploadProgress,
                this, &ImgurTalker::slotUploadProgress);

        connect(d->reply, &QNetworkReply::finished,
                this, &ImgurTalker::slotReplyFinished);
    }
}

} // namespace Digikam

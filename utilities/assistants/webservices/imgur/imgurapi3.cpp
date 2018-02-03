/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-05-27
 * Description : Implementation of v3 of the Imgur API
 *
 * Copyright (C) 2016 by Fabian Vogt <fabian at ritter dash vogt dot de>
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

#include "imgurapi3.h"

// Qt includes

#include <QFileInfo>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimerEvent>
#include <QUrlQuery>

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

static const QString imgur_auth_url = QLatin1String("https://api.imgur.com/oauth2/authorize"),
imgur_token_url = QLatin1String("https://api.imgur.com/oauth2/token");
static const uint16_t imgur_redirect_port = 8000; // Redirect URI is http://127.0.0.1:8000

ImgurAPI3::ImgurAPI3(const QString& client_id, const QString& client_secret, QObject* parent)
    : QObject(parent)
{
    m_auth.setClientId(client_id);
    m_auth.setClientSecret(client_secret);
    m_auth.setRequestUrl(imgur_auth_url);
    m_auth.setTokenUrl(imgur_token_url);
    m_auth.setRefreshTokenUrl(imgur_token_url);
    m_auth.setLocalPort(imgur_redirect_port);
    m_auth.setLocalhostPolicy(QString());

    QSettings* const settings    = WSToolUtils::getOauthSettings(this);
    O0SettingsStore* const store = new O0SettingsStore(settings, QLatin1String(O2_ENCRYPTION_KEY), this);
    store->setGroupKey(QLatin1String("Imgur"));
    m_auth.setStore(store);

    connect(&m_auth, &O2::linkedChanged,
            this, &ImgurAPI3::oauthAuthorized);

    connect(&m_auth, &O2::openBrowser,
            this, &ImgurAPI3::oauthRequestPin);

    connect(&m_auth, &O2::linkingFailed,
            this, &ImgurAPI3::oauthFailed);
}

ImgurAPI3::~ImgurAPI3()
{
    /* Disconnect all signals as cancelAllWork may emit */
    disconnect(this, 0, 0, 0);
    cancelAllWork();
}

O2 &ImgurAPI3::getAuth()
{
    return m_auth;
}

unsigned int ImgurAPI3::workQueueLength()
{
    return m_work_queue.size();
}

void ImgurAPI3::queueWork(const ImgurAPI3Action& action)
{
    m_work_queue.push(action);
    startWorkTimer();
}

void ImgurAPI3::cancelAllWork()
{
    stopWorkTimer();

    if (m_reply)
        m_reply->abort();

    /* Should error be emitted for those actions? */
    while (!m_work_queue.empty())
        m_work_queue.pop();
}

QUrl ImgurAPI3::urlForDeletehash(const QString& deletehash)
{
    return QUrl{QLatin1String("https://imgur.com/delete/") + deletehash};
}

void ImgurAPI3::oauthAuthorized()
{
    bool success = m_auth.linked();

    if (success)
        startWorkTimer();
    else
        emit busy(false);

    emit authorized(success, m_auth.extraTokens()[QLatin1String("account_username")].toString());
}

void ImgurAPI3::oauthRequestPin(const QUrl& url)
{
    emit busy(false);
    emit requestPin(url);
}

void ImgurAPI3::oauthFailed()
{
    emit authError(i18n("Could not authorize"));
}

void ImgurAPI3::uploadProgress(qint64 sent, qint64 total)
{
    if (total > 0) /* Don't divide by 0 */
        emit progress((sent * 100) / total, m_work_queue.front());
}

void ImgurAPI3::replyFinished()
{
    auto* reply = m_reply;
    reply->deleteLater();
    m_reply = nullptr;

    if (this->m_image)
    {
        delete this->m_image;
        this->m_image = nullptr;
    }

    if (m_work_queue.empty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Received result without request";
        return;
    }

    /* toInt() returns 0 if conversion fails. That fits nicely already. */
    int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    auto response = QJsonDocument::fromJson(reply->readAll());

    if (code == 200 && !response.isEmpty())
    {
        /* Success! */
        ImgurAPI3Result result;
        result.action = &m_work_queue.front();
        auto data = response.object()[QLatin1String("data")].toObject();

        switch (result.action->type)
        {
            case ImgurAPI3ActionType::IMG_UPLOAD:
            case ImgurAPI3ActionType::ANON_IMG_UPLOAD:
                result.image.animated = data[QLatin1String("animated")].toBool();
                result.image.bandwidth = data[QLatin1String("bandwidth")].toInt();
                result.image.datetime = data[QLatin1String("datetime")].toInt();
                result.image.deletehash = data[QLatin1String("deletehash")].toString();
                result.image.description = data[QLatin1String("description")].toString();
                result.image.height = data[QLatin1String("height")].toInt();
                result.image.hash = data[QLatin1String("id")].toString();
                result.image.name = data[QLatin1String("name")].toString();
                result.image.size = data[QLatin1String("size")].toInt();
                result.image.title = data[QLatin1String("title")].toString();
                result.image.type = data[QLatin1String("type")].toString();
                result.image.url = data[QLatin1String("link")].toString();
                result.image.views = data[QLatin1String("views")].toInt();
                result.image.width = data[QLatin1String("width")].toInt();
                break;
            case ImgurAPI3ActionType::ACCT_INFO:
                result.account.username = data[QLatin1String("url")].toString();
                /* TODO: Other fields */
                break;
            default:
                qCWarning(DIGIKAM_GENERAL_LOG) << "Unexpected action";
                qCDebug(DIGIKAM_GENERAL_LOG) << response.toJson();
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
             * and keep the action in the queue for later retries. */

            m_auth.refresh();
            return;
        }
        else
        {
            /* Failed. */
            auto msg = response.object()[QLatin1String("data")]
                       .toObject()[QLatin1String("error")]
                       .toString(QLatin1String("Could not read response."));

            emit error(msg, m_work_queue.front());
        }
    }

    /* Next work item. */
    m_work_queue.pop();
    startWorkTimer();
}

void ImgurAPI3::timerEvent(QTimerEvent* event)
{
    if (event->timerId() != m_work_timer)
        return QObject::timerEvent(event);

    event->accept();

    /* One-shot only. */
    QObject::killTimer(event->timerId());
    m_work_timer = 0;

    doWork();
}

void ImgurAPI3::startWorkTimer()
{
    if (!m_work_queue.empty() && m_work_timer == 0)
    {
        m_work_timer = QObject::startTimer(0);
        emit busy(true);
    }
    else
        emit busy(false);
}

void ImgurAPI3::stopWorkTimer()
{
    if (m_work_timer != 0)
    {
        QObject::killTimer(m_work_timer);
        m_work_timer = 0;
    }
}

void ImgurAPI3::addAuthToken(QNetworkRequest* request)
{
    request->setRawHeader(QByteArray("Authorization"),
                          QString::fromLatin1("Bearer %1").arg(m_auth.token()).toUtf8());
}

void ImgurAPI3::addAnonToken(QNetworkRequest* request)
{
    request->setRawHeader(QByteArray("Authorization"),
                          QString::fromLatin1("Client-ID %1").arg(m_auth.clientId()).toUtf8());
}

void ImgurAPI3::doWork()
{
    if (m_work_queue.empty() || m_reply != nullptr)
        return;

    auto &work = m_work_queue.front();

    if (work.type != ImgurAPI3ActionType::ANON_IMG_UPLOAD && !m_auth.linked())
    {
        m_auth.link();
        return; /* Wait for the authorized() signal. */
    }

    switch(work.type)
    {
        case ImgurAPI3ActionType::ACCT_INFO:
        {
            QNetworkRequest request(QUrl(QString::fromLatin1("https://api.imgur.com/3/account/%1")
                                        .arg(QLatin1String(work.account.username.toUtf8().toPercentEncoding()))));
            addAuthToken(&request);

            this->m_reply = m_net.get(request);
            break;
        }
        case ImgurAPI3ActionType::ANON_IMG_UPLOAD:
        case ImgurAPI3ActionType::IMG_UPLOAD:
        {
            this->m_image = new QFile(work.upload.imgpath);

            if (!m_image->open(QIODevice::ReadOnly))
            {
                delete this->m_image;
                this->m_image = nullptr;

                /* Failed. */
                emit error(i18n("Could not open file"), m_work_queue.front());

                m_work_queue.pop();
                return doWork();
            }

            /* Set ownership to m_image to delete that as well. */
            auto* multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType, m_image);
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
            image.setBodyDevice(this->m_image);
            multipart->append(image);

            QNetworkRequest request(QUrl(QLatin1String("https://api.imgur.com/3/image")));

            if (work.type == ImgurAPI3ActionType::IMG_UPLOAD)
                addAuthToken(&request);
            else
                addAnonToken(&request);

            this->m_reply = this->m_net.post(request, multipart);

            break;
        }
    }

    if (this->m_reply)
    {
        connect(m_reply, &QNetworkReply::uploadProgress,
                this, &ImgurAPI3::uploadProgress);

        connect(m_reply, &QNetworkReply::finished,
                this, &ImgurAPI3::replyFinished);
    }
}

} // namespace Digikam

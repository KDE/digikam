/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * Copyright (C) 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * Copyright (C) 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#include "imageshacktalker.h"

// Qt includes

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QMap>
#include <QString>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QXmlStreamReader>
#include <QApplication>
#include <QMimeDatabase>
#include <QMimeType>
#include <QUrlQuery>

// Local includes

#include "digikam_version.h"
#include "imageshacksession.h"
#include "imageshackmpform.h"
#include "digikam_debug.h"

namespace Digikam
{

class ImageShackTalker::Private
{
public:

    enum State
    {
        IMGHCK_AUTHENTICATING,
        IMGHCK_DONOTHING,
        IMGHCK_GETGALLERIES,
        IMGHCK_ADDPHOTO,
        IMGHCK_ADDVIDEO,
        IMGHCK_ADDPHOTOGALLERY
    };

public:

    explicit Private()
    {
        //userAgent   = QString::fromLatin1("KIPI-Plugin-ImageShack/%1").arg(kipipluginsVersion());
        userAgent       = QString::fromLatin1("digiKam-ImageShack/%1").arg(digiKamVersion());
        photoApiUrl     = QUrl(QString::fromLatin1("https://api.imageshack.com/v2/images"));
        videoApiUrl     = QUrl(QString::fromLatin1("http://render.imageshack.us/upload_api.php"));
        loginApiUrl     = QUrl(QString::fromLatin1("http://my.imageshack.us/setlogin.php"));
        galleryUrl      = QUrl(QString::fromLatin1("http://www.imageshack.us/gallery_api.php"));
        appKey          = QString::fromLatin1("YPZ2L9WV2de2a1e08e8fbddfbcc1c5c39f94f92a");
        session         = 0;
        loginInProgress = false;
        reply           = 0;
        state           = IMGHCK_DONOTHING;
        netMngr         = 0;
    }

public:

    ImageShackSession*     session;

    QByteArray             buffer;

    QString                userAgent;
    QUrl                   photoApiUrl;
    QUrl                   videoApiUrl;
    QUrl                   loginApiUrl;
    QUrl                   galleryUrl;
    QString                appKey;

    bool                   loginInProgress;

    QNetworkAccessManager* netMngr;

    QNetworkReply*         reply;

    State                  state;
};

ImageShackTalker::ImageShackTalker(ImageShackSession* const session)
    : d(new Private)
{
    d->session     = session;
    d->netMngr     = new QNetworkAccessManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));
}

ImageShackTalker::~ImageShackTalker()
{
    if (d->reply)
        d->reply->abort();

    delete d;
}

void ImageShackTalker::cancel()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(false);
}

QString ImageShackTalker::getCallString(QMap< QString, QString >& args) const
{
    QString result;

    for (QMap<QString, QString>::const_iterator it = args.constBegin();
         it != args.constEnd();
         ++it)
    {
        if (!result.isEmpty())
            result.append(QString::fromLatin1("&"));

        result.append(it.key());
        result.append(QString::fromLatin1("="));
        result.append(it.value());
    }

    return result;
}

void ImageShackTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != d->reply)
    {
        return;
    }

    d->reply = 0;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (d->state == Private::IMGHCK_AUTHENTICATING)
        {
            checkRegistrationCodeDone(reply->error(), reply->errorString());
            emit signalBusy(false);
        }
        else if (d->state == Private::IMGHCK_GETGALLERIES)
        {
            emit signalBusy(false);
            emit signalGetGalleriesDone(reply->error(), reply->errorString());
        }
        else if (d->state == Private::IMGHCK_ADDPHOTO || d->state == Private::IMGHCK_ADDPHOTOGALLERY)
        {
            emit signalBusy(false);
            emit signalAddPhotoDone(reply->error(), reply->errorString());
        }

        d->state = Private::IMGHCK_DONOTHING;
        reply->deleteLater();
        return;
    }

    d->buffer.append(reply->readAll());

    switch (d->state)
    {
        case Private::IMGHCK_AUTHENTICATING:
            parseAccessToken(d->buffer);
            break;
        case Private::IMGHCK_ADDPHOTOGALLERY:
            parseAddPhotoToGalleryDone(d->buffer);
            break;
        case Private::IMGHCK_ADDVIDEO:
        case Private::IMGHCK_ADDPHOTO:
            parseUploadPhotoDone(d->buffer);
            break;
        case Private::IMGHCK_GETGALLERIES:
            parseGetGalleries(d->buffer);
            break;
        default:
            break;
    }

    reply->deleteLater();
}

void ImageShackTalker::authenticate()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);
    emit signalJobInProgress(1, 4, i18n("Authenticating the user"));

    QUrl url(QString::fromLatin1("https://api.imageshack.com/v2/user/login"));
    QUrlQuery q(url);
    q.addQueryItem(QString::fromLatin1("user"), d->session->email());
    q.addQueryItem(QString::fromLatin1("password"), d->session->password());
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    d->reply = d->netMngr->post(netRequest, QByteArray());

    d->state = Private::IMGHCK_AUTHENTICATING;
    d->buffer.resize(0);
}

void ImageShackTalker::getGalleries()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);
    emit signalJobInProgress(3, 4, i18n("Getting galleries from server"));

    QUrl gUrl(d->galleryUrl);

    QUrlQuery q(gUrl);
    q.addQueryItem(QString::fromLatin1("action"), QString::fromLatin1("gallery_list"));
    q.addQueryItem(QString::fromLatin1("user"), d->session->username());
    gUrl.setQuery(q);

    d->reply = d->netMngr->get(QNetworkRequest(gUrl));

    d->state = Private::IMGHCK_GETGALLERIES;
    d->buffer.resize(0);
}

void ImageShackTalker::checkRegistrationCodeDone(int errCode, const QString& errMsg)
{
    emit signalBusy(false);
    emit signalLoginDone(errCode, errMsg);
    d->loginInProgress = false;
}

void ImageShackTalker::parseAccessToken(const QByteArray &data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Data received is "<< data;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        return;
    }

    QJsonObject jsonObject = doc.object();

    if (jsonObject[QString::fromLatin1("success")].toBool())
    {
        d->session->setLoggedIn(true);
        QJsonObject obj          = jsonObject[QString::fromLatin1("result")].toObject();
        d->session->setUsername(obj[QString::fromLatin1("username")].toString());
        d->session->setEmail(obj[QString::fromLatin1("email")].toString());
        d->session->setAuthToken(obj[QString::fromLatin1("auth_token")].toString());
        checkRegistrationCodeDone(0,QString::fromLatin1(""));
    }
    else
    {
        d->session->setLoggedIn(false);
        QJsonObject obj          = jsonObject[QString::fromLatin1("error")].toObject();
        checkRegistrationCodeDone(obj[QString::fromLatin1("error_code")].toInt(), obj[QString::fromLatin1("error_message")].toString());
    }
}

void ImageShackTalker::parseGetGalleries(const QByteArray &data)
{
    QDomDocument document;

    if (!document.setContent(data))
        return;

    QDomElement rootElem  = document.documentElement();
    QDomNodeList children = rootElem.childNodes();

    QStringList gTexts;
    QStringList gNames;

    for (int i = 0; i < children.size(); ++i)
    {
        QDomElement e = children.at(i).toElement();

        if (e.tagName() == QString::fromLatin1("gallery"))
        {
            QDomElement nameElem   = e.firstChildElement(QString::fromLatin1("name"));
            QDomElement titleElem  = e.firstChildElement(QString::fromLatin1("title"));
            QDomElement serverElem = e.firstChildElement(QString::fromLatin1("server"));

            if (!nameElem.isNull())
            {
                QString fmt;
                fmt          = nameElem.firstChild().toText().data();
                gNames << nameElem.firstChild().toText().data();
                gTexts << titleElem.firstChild().toText().data();
            }
        }
    }

    d->state = Private::IMGHCK_DONOTHING;

    emit signalUpdateGalleries(gTexts, gNames);
    emit signalGetGalleriesDone(0, i18n("Successfully retrieved galleries"));
}

void ImageShackTalker::authenticationDone(int errCode, const QString& errMsg)
{
    if (errCode)
    {
        d->session->logOut();
    }

    emit signalBusy(false);
    emit signalLoginDone(errCode, errMsg);
    d->loginInProgress = false;
}

void ImageShackTalker::logOut()
{
    d->session->logOut();
    d->loginInProgress = false;
}

void ImageShackTalker::cancelLogIn()
{
    logOut();
    emit signalLoginDone(-1, QString::fromLatin1("Canceled by the user!"));
}

QString ImageShackTalker::mimeType(const QString& path) const
{
    QMimeDatabase db;
    QMimeType ptr = db.mimeTypeForUrl(QUrl::fromLocalFile(path));

    return ptr.name();
}

void ImageShackTalker::uploadItem(const QString& path, const QMap<QString, QString>& opts)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);
    QMap<QString, QString> args;
    args[QString::fromLatin1("key")]        = d->appKey;
    args[QString::fromLatin1("fileupload")] = QUrl(path).fileName();

    ImageShackMPForm form;

    for (QMap<QString, QString>::const_iterator it = opts.constBegin();
         it != opts.constEnd();
         ++it)
    {
        form.addPair(it.key(), it.value());
    }

    for (QMap<QString, QString>::const_iterator it = args.constBegin();
         it != args.constEnd();
         ++it)
    {
        form.addPair(it.key(), it.value());
    }

    if (!form.addFile(QUrl(path).fileName(), path))
    {
        emit signalBusy(false);
        return;
    }

    form.finish();

    QUrl uploadUrl = QUrl(d->photoApiUrl);
    d->state        = Private::IMGHCK_ADDPHOTO;

    QNetworkRequest netRequest(uploadUrl);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());
    netRequest.setHeader(QNetworkRequest::UserAgentHeader, d->userAgent);

    d->reply = d->netMngr->post(netRequest, form.formData());

    d->buffer.resize(0);

    //uploadItemToGallery(path, QString::fromLatin1(""), opts);
}

void ImageShackTalker::uploadItemToGallery(const QString& path, const QString& /*gallery*/, const QMap<QString, QString>& opts)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);
    QMap<QString, QString> args;
    args[QString::fromLatin1("key")]        = d->appKey;
    args[QString::fromLatin1("fileupload")] = QUrl(path).fileName();

    ImageShackMPForm form;

    for (QMap<QString, QString>::const_iterator it = opts.constBegin();
         it != opts.constEnd();
         ++it)
    {
        form.addPair(it.key(), it.value());
    }

    for (QMap<QString, QString>::const_iterator it = args.constBegin();
         it != args.constEnd();
         ++it)
    {
        form.addPair(it.key(), it.value());
    }

    if (!form.addFile(QUrl(path).fileName(), path))
    {
        emit signalBusy(false);
        return;
    }

    form.finish();

    // Check where to upload
    QString mime        = mimeType(path);

    QUrl uploadUrl;

    uploadUrl           = QUrl(d->photoApiUrl);
    d->state            = Private::IMGHCK_ADDPHOTO;

    QNetworkRequest netRequest(uploadUrl);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());
    netRequest.setHeader(QNetworkRequest::UserAgentHeader, d->userAgent);

    d->reply = d->netMngr->post(netRequest, form.formData());

    d->buffer.resize(0);
}

int ImageShackTalker::parseErrorResponse(QDomElement elem, QString& errMsg) const
{
    int errCode = -1;
    QString err_code;

    for (QDomNode node = elem.firstChild();
         !node.isNull();
         node = node.nextSibling())
    {
        if (!node.isElement())
            continue;

        QDomElement e = node.toElement();

        if (e.tagName() == QString::fromLatin1("error"))
        {
            err_code = e.attributeNode(QString::fromLatin1("id")).value();
            errMsg   = e.text();
        }
    }

    if (err_code == QString::fromLatin1("file_too_big"))
    {
        errCode = 501;
    }
    else
    {
        errCode = 502;
    }

    return errCode;
}

void ImageShackTalker::parseUploadPhotoDone(QByteArray data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "ParseUploadPhotoDone data is "<<data;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        return;
    }

    QJsonObject jsonObject = doc.object();

    if (d->state == Private::IMGHCK_ADDPHOTO || d->state == Private::IMGHCK_ADDVIDEO || (d->state == Private::IMGHCK_ADDPHOTOGALLERY))
    {
        if(jsonObject[QString::fromLatin1("success")].toBool())
        {
            emit signalBusy(false);
            emit signalAddPhotoDone(0,QString::fromLatin1(""));
        }
        else
        {
            QJsonObject obj = jsonObject[QString::fromLatin1("error")].toObject();
            emit signalAddPhotoDone(obj[QString::fromLatin1("error_code")].toInt(), obj[QString::fromLatin1("error_message")].toString());
            emit signalBusy(false);
        }
    }
}

void ImageShackTalker::parseAddPhotoToGalleryDone(QByteArray data)
{
    //int errCode = -1;
    QString errMsg = QString::fromLatin1("");
    QDomDocument domDoc(QString::fromLatin1("galleryXML"));

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << data;

    if (!domDoc.setContent(data))
        return;

    QDomElement rootElem = domDoc.documentElement();

    if (rootElem.isNull() || rootElem.tagName() != QString::fromLatin1("gallery"))
    {
        // TODO error cheking
    }
    else
    {
        emit signalBusy(false);
        emit signalAddPhotoDone(0, QString::fromLatin1(""));
    }
}

} // namespace Digikam

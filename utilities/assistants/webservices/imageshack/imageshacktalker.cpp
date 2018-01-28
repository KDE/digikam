/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * Copyright (C) 2012 Dodon Victor <dodonvictor at gmail dot com>
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
#include "imageshack.h"
#include "imageshackmpform.h"
#include "digikam_debug.h"

namespace Digikam
{

ImageShackTalker::ImageShackTalker(ImageShack* const imghack)
    : m_imageshack(imghack),
      m_loginInProgress(false),
      m_reply(0),
      m_state(IMGHCK_DONOTHING)
{
//    m_userAgent   = QString::fromLatin1("KIPI-Plugin-ImageShack/%1").arg(kipipluginsVersion());
    m_userAgent   = QString::fromLatin1("digiKam-ImageShack/%1").arg(digiKamVersion());
    m_photoApiUrl = QUrl(QString::fromLatin1("https://api.imageshack.com/v2/images"));
    m_videoApiUrl = QUrl(QString::fromLatin1("http://render.imageshack.us/upload_api.php"));
    m_loginApiUrl = QUrl(QString::fromLatin1("http://my.imageshack.us/setlogin.php"));
    m_galleryUrl  = QUrl(QString::fromLatin1("http://www.imageshack.us/gallery_api.php"));
    m_appKey      = QString::fromLatin1("YPZ2L9WV2de2a1e08e8fbddfbcc1c5c39f94f92a");

    m_netMngr     = new QNetworkAccessManager(this);

    connect(m_netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));
}

ImageShackTalker::~ImageShackTalker()
{
    if (m_reply)
        m_reply->abort();
}

void ImageShackTalker::cancel()
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(false);
}

QString ImageShackTalker::getCallString(QMap< QString, QString >& args)
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
    if (reply != m_reply)
    {
        return;
    }

    m_reply = 0;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (m_state == IMGHCK_AUTHENTICATING)
        {
            checkRegistrationCodeDone(reply->error(), reply->errorString());
            emit signalBusy(false);
        }
        else if (m_state == IMGHCK_GETGALLERIES)
        {
            emit signalBusy(false);
            emit signalGetGalleriesDone(reply->error(), reply->errorString());
        }
        else if (m_state == IMGHCK_ADDPHOTO || m_state == IMGHCK_ADDPHOTOGALLERY)
        {
            emit signalBusy(false);
            emit signalAddPhotoDone(reply->error(), reply->errorString());
        }

        m_state = IMGHCK_DONOTHING;
        reply->deleteLater();
        return;
    }

    m_buffer.append(reply->readAll());

    switch (m_state)
    {
        case IMGHCK_AUTHENTICATING:
            parseAccessToken(m_buffer);
            break;
        case IMGHCK_ADDPHOTOGALLERY:
            parseAddPhotoToGalleryDone(m_buffer);
            break;
        case IMGHCK_ADDVIDEO:
        case IMGHCK_ADDPHOTO:
            parseUploadPhotoDone(m_buffer);
            break;
        case IMGHCK_GETGALLERIES:
            parseGetGalleries(m_buffer);
            break;
        default:
            break;
    }

    reply->deleteLater();
}

void ImageShackTalker::authenticate()
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);
    emit signalJobInProgress(1, 4, i18n("Authenticating the user"));

    QUrl url(QString::fromLatin1("https://api.imageshack.com/v2/user/login"));
    QUrlQuery q(url);
    q.addQueryItem(QString::fromLatin1("user"), m_imageshack->email());
    q.addQueryItem(QString::fromLatin1("password"), m_imageshack->password());
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    m_reply = m_netMngr->post(netRequest, QByteArray());

    m_state = IMGHCK_AUTHENTICATING;
    m_buffer.resize(0);
}

void ImageShackTalker::getGalleries()
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);
    emit signalJobInProgress(3, 4, i18n("Getting galleries from server"));

    QUrl gUrl(m_galleryUrl);

    QUrlQuery q(gUrl);
    q.addQueryItem(QString::fromLatin1("action"), QString::fromLatin1("gallery_list"));
    q.addQueryItem(QString::fromLatin1("user"), m_imageshack->username());
    gUrl.setQuery(q);

    m_reply = m_netMngr->get(QNetworkRequest(gUrl));

    m_state = IMGHCK_GETGALLERIES;
    m_buffer.resize(0);
}

void ImageShackTalker::checkRegistrationCodeDone(int errCode, const QString& errMsg)
{
    emit signalBusy(false);
    emit signalLoginDone(errCode, errMsg);
    m_loginInProgress = false;
}

void ImageShackTalker::parseAccessToken(const QByteArray &data)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Data received is "<< data;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if(err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        return;
    }

    QJsonObject jsonObject = doc.object();

    if(jsonObject[QString::fromLatin1("success")].toBool())
    {
        m_imageshack->m_loggedIn = true;
        QJsonObject obj          = jsonObject[QString::fromLatin1("result")].toObject();
        m_imageshack->setUsername(obj[QString::fromLatin1("username")].toString());
        m_imageshack->setEmail(obj[QString::fromLatin1("email")].toString());
        m_imageshack->setAuthToken(obj[QString::fromLatin1("auth_token")].toString());
        checkRegistrationCodeDone(0,QString::fromLatin1(""));
    }
    else
    {
        m_imageshack->m_loggedIn = false;
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

    m_state = IMGHCK_DONOTHING;

    emit signalUpdateGalleries(gTexts, gNames);
    emit signalGetGalleriesDone(0, i18n("Successfully retrieved galleries"));
}

void ImageShackTalker::authenticationDone(int errCode, const QString& errMsg)
{
    if (errCode)
    {
        m_imageshack->logOut();
    }

    emit signalBusy(false);
    emit signalLoginDone(errCode, errMsg);
    m_loginInProgress = false;
}

void ImageShackTalker::logOut()
{
    m_imageshack->logOut();
    m_loginInProgress = false;
}

void ImageShackTalker::cancelLogIn()
{
    logOut();
    emit signalLoginDone(-1, QString::fromLatin1("Canceled by the user!"));
}

QString ImageShackTalker::mimeType(const QString& path)
{
    QMimeDatabase db;
    QMimeType ptr = db.mimeTypeForUrl(QUrl::fromLocalFile(path));
    return ptr.name();
}

void ImageShackTalker::uploadItem(const QString& path, const QMap<QString, QString>& opts)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);
    QMap<QString, QString> args;
    args[QString::fromLatin1("key")]        = m_appKey;
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

    QUrl uploadUrl = QUrl(m_photoApiUrl);
    m_state        = IMGHCK_ADDPHOTO;

    QNetworkRequest netRequest(uploadUrl);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());
    netRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);

    m_reply = m_netMngr->post(netRequest, form.formData());

    m_buffer.resize(0);

    //uploadItemToGallery(path, QString::fromLatin1(""), opts);
}

void ImageShackTalker::uploadItemToGallery(const QString& path, const QString& /*gallery*/, const QMap<QString, QString>& opts)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);
    QMap<QString, QString> args;
    args[QString::fromLatin1("key")]        = m_appKey;
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

    uploadUrl           = QUrl(m_photoApiUrl);
    m_state             = IMGHCK_ADDPHOTO;

    QNetworkRequest netRequest(uploadUrl);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());
    netRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);

    m_reply = m_netMngr->post(netRequest, form.formData());

    m_buffer.resize(0);
}

int ImageShackTalker::parseErrorResponse(QDomElement elem, QString& errMsg)
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
    qCDebug(DIGIKAM_GENERAL_LOG) << "ParseUploadPhotoDone data is "<<data;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        return;
    }

    QJsonObject jsonObject = doc.object();

    if (m_state == IMGHCK_ADDPHOTO || m_state == IMGHCK_ADDVIDEO || (m_state == IMGHCK_ADDPHOTOGALLERY))
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

    qCDebug(DIGIKAM_GENERAL_LOG) << data;

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

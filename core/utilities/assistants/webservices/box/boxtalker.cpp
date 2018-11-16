/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Box web service
 *
 * Copyright (C) 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
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

#include "boxtalker.h"

// Qt includes

#include <QMimeDatabase>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QByteArray>
#include <QList>
#include <QPair>
#include <QFileInfo>
#include <QWidget>
#include <QSettings>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopServices>
#include <QHttpMultiPart>
#include <QNetworkAccessManager>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "wstoolutils.h"
#include "boxwindow.h"
#include "boxitem.h"
#include "previewloadthread.h"
#include "o0settingsstore.h"

namespace Digikam
{

class Q_DECL_HIDDEN BOXTalker::Private
{
public:

    enum State
    {
        BOX_USERNAME = 0,
        BOX_LISTFOLDERS,
        BOX_CREATEFOLDER,
        BOX_ADDPHOTO
    };

public:

    explicit Private()
    {
        clientId     = QLatin1String("yvd43v8av9zgg9phig80m2dc3r7mks4t");
        clientSecret = QLatin1String("KJkuMjvzOKDMyp3oxweQBEYixg678Fh5");

        authUrl      = QLatin1String("https://account.box.com/api/oauth2/authorize");
        tokenUrl     = QLatin1String("https://api.box.com/oauth2/token");
        redirectUrl  = QLatin1String("https://app.box.com");

        state        = BOX_USERNAME;

        parent       = 0;
        netMngr      = 0;
        reply        = 0;
        settings     = 0;
        o2           = 0;
    }

public:

    QString                         clientId;
    QString                         clientSecret;
    QString                         authUrl;
    QString                         tokenUrl;
    QString                         redirectUrl;

    QWidget*                        parent;
    QNetworkAccessManager*          netMngr;
    QNetworkReply*                  reply;
    QSettings*                      settings;
    O2*                             o2;

    State                           state;

    QByteArray                      buffer;
    DMetadata                       meta;
    QMap<QString, QString>          urlParametersMap;
    QList<QPair<QString, QString> > foldersList;
};

BOXTalker::BOXTalker(QWidget* const parent)
    : d(new Private)
{
    d->parent  = parent;
    d->netMngr = new QNetworkAccessManager(this);

    connect(this, SIGNAL(boxLinkingFailed()),
            this, SLOT(slotLinkingFailed()));

    connect(this, SIGNAL(boxLinkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    d->o2      = new O2(this);

    d->o2->setClientId(d->clientId);
    d->o2->setClientSecret(d->clientSecret);
    d->o2->setRefreshTokenUrl(d->tokenUrl);
    d->o2->setRequestUrl(d->authUrl);
    d->o2->setTokenUrl(d->tokenUrl);
    d->o2->setLocalPort(8000);

    d->settings                  = WSToolUtils::getOauthSettings(this);
    O0SettingsStore* const store = new O0SettingsStore(d->settings, QLatin1String(O2_ENCRYPTION_KEY), this);
    store->setGroupKey(QLatin1String("Box"));
    d->o2->setStore(store);

    connect(d->o2, SIGNAL(linkingFailed()),
            this, SLOT(slotLinkingFailed()));

    connect(d->o2, SIGNAL(linkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));

    connect(d->o2, SIGNAL(openBrowser(QUrl)),
            this, SLOT(slotOpenBrowser(QUrl)));
}

BOXTalker::~BOXTalker()
{
    if (d->reply)
    {
        d->reply->abort();
    }

    WSToolUtils::removeTemporaryDir("box");

    delete d;
}

void BOXTalker::link()
{
    emit signalBusy(true);
    d->o2->link();
}

void BOXTalker::unLink()
{
    d->o2->unlink();
    d->settings->beginGroup(QLatin1String("Box"));
    d->settings->remove(QString());
    d->settings->endGroup();
}

void BOXTalker::slotLinkingFailed()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Box fail";
    emit signalBusy(false);
}

void BOXTalker::slotLinkingSucceeded()
{
    if (!d->o2->linked())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UNLINK to Box ok";
        emit signalBusy(false);
        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Box ok";
    emit signalLinkingSucceeded();
}

bool BOXTalker::authenticated()
{
    return d->o2->linked();
}

void BOXTalker::cancel()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(false);
}

void BOXTalker::slotOpenBrowser(const QUrl& url)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Open Browser...";
    QDesktopServices::openUrl(url);
}

void BOXTalker::createFolder(QString& path)
{
    QString name       = path.section(QLatin1Char('/'), -1);
    QString folderPath = path.section(QLatin1Char('/'), -2, -2);

    QString id;

    for (int i = 0 ; i < d->foldersList.size() ; ++i)
    {
        if (d->foldersList.value(i).second == folderPath)
        {
            id = d->foldersList.value(i).first;
        }
    }

    QUrl url(QLatin1String("https://api.box.com/2.0/folders"));
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(d->o2->token()).toUtf8());

    QByteArray postData = QString::fromUtf8("{\"name\": \"%1\",\"parent\": {\"id\": \"%2\"}}").arg(name).arg(id).toUtf8();

    d->reply = d->netMngr->post(netRequest, postData);
    d->state = Private::BOX_CREATEFOLDER;
    d->buffer.resize(0);
    emit signalBusy(true);
}

void BOXTalker::getUserName()
{
    QUrl url(QLatin1String("https://api.box.com/2.0/users/me"));

    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(d->o2->token()).toUtf8());
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));

    d->reply = d->netMngr->get(netRequest);
    d->state = Private::BOX_USERNAME;
    d->buffer.resize(0);
    emit signalBusy(true);
}

void BOXTalker::listFolders(const QString& /*path*/)
{
    QUrl url(QLatin1String("https://api.box.com/2.0/folders/0/items"));;

    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(d->o2->token()).toUtf8());
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));

    d->reply = d->netMngr->get(netRequest);

    d->state = Private::BOX_LISTFOLDERS;
    d->buffer.resize(0);
    emit signalBusy(true);
}

bool BOXTalker::addPhoto(const QString& imgPath, const QString& uploadFolder, bool rescale, int maxDim, int imageQuality)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);

    QMimeDatabase mimeDB;
    QString path     = imgPath;
    QString mimeType = mimeDB.mimeTypeForFile(path).name();

    if (mimeType.startsWith(QLatin1String("image/")))
    {
        QImage image = PreviewLoadThread::loadHighQualitySynchronously(imgPath).copyQImage();

        if (image.isNull())
        {
            emit signalBusy(false);
            return false;
        }

        path = WSToolUtils::makeTemporaryDir("box").filePath(QFileInfo(imgPath)
                                             .baseName().trimmed() + QLatin1String(".jpg"));

        if (rescale && (image.width() > maxDim || image.height() > maxDim))
        {
            image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        image.save(path, "JPEG", imageQuality);

        if (d->meta.load(imgPath))
        {
            d->meta.setItemDimensions(image.size());
            d->meta.setItemOrientation(DMetadata::ORIENTATION_NORMAL);
            d->meta.setMetadataWritingMode((int)DMetadata::WRITE_TO_FILE_ONLY);
            d->meta.save(path, true);
        }
    }

    QString id;

    for (int i = 0 ; i < d->foldersList.size() ; ++i)
    {
        if (d->foldersList.value(i).second == uploadFolder)
        {
            id = d->foldersList.value(i).first;
        }
    }

    QHttpMultiPart* const multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart attributes;
    QString attributesHeader  = QLatin1String("form-data; name=\"attributes\"");
    attributes.setHeader(QNetworkRequest::ContentDispositionHeader, attributesHeader);

    QString postData = QLatin1String("{\"name\":\"") + QFileInfo(imgPath).fileName() + QLatin1Char('"') +
                       QLatin1String(", \"parent\":{\"id\":\"") + id + QLatin1String("\"}}");
    attributes.setBody(postData.toUtf8());
    multiPart->append(attributes);

    QFile* const file = new QFile(path);
    file->open(QIODevice::ReadOnly);

    QHttpPart imagePart;
    QString imagePartHeader = QLatin1String("form-data; name=\"file\"; filename=\"") +
                              QFileInfo(imgPath).fileName() + QLatin1Char('"');

    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, imagePartHeader);
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, mimeType);

    imagePart.setBodyDevice(file);
    multiPart->append(imagePart);

    QUrl url(QString::fromLatin1("https://upload.box.com/api/2.0/files/content?access_token=%1").arg(d->o2->token()));

    QNetworkRequest netRequest(url);
    QString content = QLatin1String("multipart/form-data;boundary=") + multiPart->boundary();
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, content);
    d->reply        = d->netMngr->post(netRequest, multiPart);
    // delete the multiPart and file with the reply
    multiPart->setParent(d->reply);

    d->state        = Private::BOX_ADDPHOTO;
    d->buffer.resize(0);

    return true;
}

void BOXTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != d->reply)
    {
        return;
    }

    d->reply = 0;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (d->state != Private::BOX_CREATEFOLDER)
        {
            emit signalBusy(false);
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18n("Error"), reply->errorString());
            reply->deleteLater();
            return;
        }
    }

    d->buffer.append(reply->readAll());

    switch (d->state)
    {
        case Private::BOX_LISTFOLDERS:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In BOX_LISTFOLDERS";
            parseResponseListFolders(d->buffer);
            break;

        case Private::BOX_CREATEFOLDER:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In BOX_CREATEFOLDER";
            parseResponseCreateFolder(d->buffer);
            break;

        case Private::BOX_ADDPHOTO:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In BOX_ADDPHOTO";
            parseResponseAddPhoto(d->buffer);
            break;

        case Private::BOX_USERNAME:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In BOX_USERNAME";
            parseResponseUserName(d->buffer);
            break;

        default:
            break;
    }

    reply->deleteLater();
}

void BOXTalker::parseResponseAddPhoto(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object();
    bool success           = jsonObject.contains(QLatin1String("total_count"));
    emit signalBusy(false);

    if (!success)
    {
        emit signalAddPhotoFailed(i18n("Failed to upload photo"));
    }
    else
    {
        emit signalAddPhotoSucceeded();
    }
}

void BOXTalker::parseResponseUserName(const QByteArray& data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QString name      = doc.object()[QLatin1String("name")].toString();
    emit signalBusy(false);
    emit signalSetUserName(name);
}

void BOXTalker::parseResponseListFolders(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        emit signalListAlbumsFailed(i18n("Failed to list folders"));
        return;
    }

    QJsonObject jsonObject = doc.object();
    QJsonArray jsonArray   = jsonObject[QLatin1String("entries")].toArray();

    d->foldersList.clear();
    d->foldersList.append(qMakePair(QLatin1String("0"), QLatin1String("root")));

    foreach (const QJsonValue& value, jsonArray)
    {
        QString path;
        QString folderName;
        QString type;
        QString id;

        QJsonObject obj = value.toObject();
        type            = obj[QLatin1String("type")].toString();

        if (type == "folder")
        {
            folderName = obj[QLatin1String("name")].toString();
            id         = obj[QLatin1String("id")].toString();
            d->foldersList.append(qMakePair(id, folderName));
        }
    }

    emit signalBusy(false);
    emit signalListAlbumsDone(d->foldersList);
}

void BOXTalker::parseResponseCreateFolder(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object();
    bool fail              = jsonObject.contains(QLatin1String("error"));

    emit signalBusy(false);

    if (fail)
    {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        emit signalCreateFolderFailed(jsonObject[QLatin1String("error_summary")].toString());
    }
    else
    {
        emit signalCreateFolderSucceeded();
    }
}

} // namespace Digikam

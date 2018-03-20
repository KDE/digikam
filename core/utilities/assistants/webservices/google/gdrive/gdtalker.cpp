/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export items to Google web services
 *
 * Copyright (C) 2013      by Pankaj Kumar <me at panks dot me>
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

#include <gdtalker.h>

// Qt includes

#include <QMimeDatabase>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QByteArray>
#include <QtAlgorithms>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QList>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QPair>
#include <QFileInfo>
#include <QDebug>
#include <QMessageBox>
#include <QStandardPaths>
#include <QUrlQuery>

// Local includes

#include "wstoolutils.h"
#include "digikam_version.h"
#include "gswindow.h"
#include "gdmpform.h"
#include "digikam_debug.h"
#include "previewloadthread.h"
#include "dmetadata.h"

namespace Digikam
{

static bool gdriveLessThan(const GSFolder& p1, const GSFolder& p2)
{
    return (p1.title.toLower() < p2.title.toLower());
}

class GDTalker::Private
{
public:

    enum State
    {
        GD_LOGOUT      = -1,
        GD_LISTFOLDERS = 0,
        GD_CREATEFOLDER,
        GD_ADDPHOTO,
        GD_USERNAME,
    };

public:

    explicit Private()
    {
        state          = GD_LOGOUT;
        netMngr        = 0;
        rootid         = QString::fromLatin1("root");
        rootfoldername = QString::fromLatin1("GoogleDrive Root");
    }

public:

    QString                rootid;
    QString                rootfoldername;
    QString                username;
    State                  state;

    QNetworkAccessManager* netMngr;
};

GDTalker::GDTalker(QWidget* const parent)
    : GSTalkerBase(parent, QString::fromLatin1("https://www.googleapis.com/auth/drive")), 
      d(new Private)
{
    d->netMngr = new QNetworkAccessManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));
}

GDTalker::~GDTalker()
{
    delete d;
}

/**
 * Gets username
 */
void GDTalker::getUserName()
{
    QUrl url(QString::fromLatin1("https://www.googleapis.com/drive/v2/about"));
    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QString::fromLatin1("scope"),        m_scope);
    urlQuery.addQueryItem(QString::fromLatin1("access_token"), m_accessToken);
    url.setQuery(urlQuery);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1());

    m_reply  = d->netMngr->get(netRequest);
    d->state = Private::GD_USERNAME;
    m_buffer.resize(0);

    emit signalBusy(true);
}

/**
 * Gets list of folder of user in json format
 */
void GDTalker::listFolders()
{
    QUrl url(QString::fromLatin1("https://www.googleapis.com/drive/v2/files?q=mimeType = 'application/vnd.google-apps.folder'"));

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1());

    m_reply  = d->netMngr->get(netRequest);
    d->state = Private::GD_LISTFOLDERS;
    m_buffer.resize(0);

    emit signalBusy(true);
}

/**
 * Creates folder inside any folder(of which id is passed)
 */
void GDTalker::createFolder(const QString& title, const QString& id)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    QUrl url(QString::fromLatin1("https://www.googleapis.com/drive/v2/files"));
    QByteArray data;
    data += "{\"title\":\"";
    data += title.toLatin1();
    data += "\",\r\n";
    data += "\"parents\":";
    data += "[{";
    data += "\"id\":\"";
    data += id.toLatin1();
    data += "\"}],\r\n";
    data += "\"mimeType\":";
    data += "\"application/vnd.google-apps.folder\"";
    data += "}\r\n";

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "data:" << data;

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1());

    m_reply  = d->netMngr->post(netRequest, data);
    d->state = Private::GD_CREATEFOLDER;
    m_buffer.resize(0);

    emit signalBusy(true);
}

bool GDTalker::addPhoto(const QString& imgPath, const GSPhoto& info,
                        const QString& id, bool rescale, int maxDim, int imageQuality)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);
    GDMPForm form;
    form.addPair(QUrl::fromLocalFile(imgPath).fileName(),info.description,imgPath,id);
    QString path = imgPath;

    QMimeDatabase mimeDB;

    if (!mimeDB.mimeTypeForFile(path).name().startsWith(QLatin1String("video/")))
    {
        QImage image = PreviewLoadThread::loadHighQualitySynchronously(imgPath).copyQImage();

        if (image.isNull())
        {
            image.load(imgPath);
        }

        if (image.isNull())
        {
            return false;
        }

        path                  = WSToolUtils::makeTemporaryDir("google")
                                             .filePath(QFileInfo(imgPath)
                                             .baseName().trimmed() +
                                             QLatin1String(".jpg"));
        int imgQualityToApply = 100;

        if (rescale)
        {
            if (image.width() > maxDim || image.height() > maxDim)
                image = image.scaled(maxDim,maxDim,Qt::KeepAspectRatio,Qt::SmoothTransformation);

            imgQualityToApply = imageQuality;
        }

        image.save(path, "JPEG", imgQualityToApply);

        DMetadata meta;

        if (meta.load(imgPath))
        {
            meta.setImageDimensions(image.size());
            meta.setImageOrientation(MetaEngine::ORIENTATION_NORMAL);
            meta.setImageProgramId(QString::fromLatin1("digiKam"), digiKamVersion());
            meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
            meta.save(path);
        }
    }

    if (!form.addFile(path))
    {
        emit signalBusy(false);
        return false;
    }

    form.finish();

    QUrl url(QString::fromLatin1("https://www.googleapis.com/upload/drive/v2/files?uploadType=multipart"));

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1());
    netRequest.setRawHeader("Host", "www.googleapis.com");

    m_reply = d->netMngr->post(netRequest, form.formData());

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In add photo";
    d->state = Private::GD_ADDPHOTO;
    m_buffer.resize(0);
    
    emit signalBusy(true);

    return true;
}

void GDTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != m_reply)
    {
        return;
    }

    m_reply = 0;

    if (reply->error() != QNetworkReply::NoError)
    {
        emit signalBusy(false);
        QMessageBox::critical(QApplication::activeWindow(),
                              i18n("Error"), reply->errorString());

        reply->deleteLater();
        return;
    }

    m_buffer.append(reply->readAll());

    switch (d->state)
    {
        case (Private::GD_LOGOUT):
            break;
        case (Private::GD_LISTFOLDERS):
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In Private::GD_LISTFOLDERS";
            parseResponseListFolders(m_buffer);
            break;
        case (Private::GD_CREATEFOLDER):
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In Private::GD_CREATEFOLDER";
            parseResponseCreateFolder(m_buffer);
            break;
        case (Private::GD_ADDPHOTO):
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In Private::GD_ADDPHOTO"; // << m_buffer;
            parseResponseAddPhoto(m_buffer);
            break;
        case (Private::GD_USERNAME):
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In Private::GD_USERNAME"; // << m_buffer;
            parseResponseUserName(m_buffer);
            break;
        default:
            break;
    }

    reply->deleteLater();
}

void GDTalker::parseResponseUserName(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        return;
    }

    QJsonObject jsonObject = doc.object();
    qCDebug(DIGIKAM_WEBSERVICES_LOG)<<"User Name is: " << jsonObject[QString::fromLatin1("name")].toString();
    QString temp           = jsonObject[QString::fromLatin1("name")].toString();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "in parseResponseUserName";

    emit signalBusy(false);
    emit signalSetUserName(temp);
}

void GDTalker::parseResponseListFolders(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << data;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        emit signalListAlbumsDone(0,i18n("Failed to list folders"),QList<GSFolder>());
        return;
    }

    QJsonObject jsonObject = doc.object();
    QJsonArray jsonArray   = jsonObject[QString::fromLatin1("items")].toArray();

    QList<GSFolder> albumList;
    GSFolder fps;
    fps.id    = d->rootid;
    fps.title = d->rootfoldername;
    albumList.append(fps);

    foreach (const QJsonValue& value, jsonArray)
    {
        QJsonObject obj = value.toObject();
        fps.id          = obj[QString::fromLatin1("id")].toString();
        fps.title       = obj[QString::fromLatin1("title")].toString();
        albumList.append(fps);
    }

    std::sort(albumList.begin(), albumList.end(), gdriveLessThan);

    emit signalBusy(false);
    emit signalListAlbumsDone(1,QString(),albumList);
}

void GDTalker::parseResponseCreateFolder(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        return;
    }

    QJsonObject jsonObject = doc.object();
    QString temp           = jsonObject[QString::fromLatin1("alternateLink")].toString();
    bool success           = false;

    if (!(QString::compare(temp, QString::fromLatin1(""), Qt::CaseInsensitive) == 0))
        success = true;

    emit signalBusy(false);

    if (!success)
    {
        emit signalCreateFolderDone(0,i18n("Failed to create folder"));
    }
    else
    {
        emit signalCreateFolderDone(1,QString());
    }
}

void GDTalker::parseResponseAddPhoto(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        return;
    }

    QJsonObject jsonObject = doc.object();
    QString altLink        = jsonObject[QString::fromLatin1("alternateLink")].toString();
    QString photoId        = jsonObject[QString::fromLatin1("id")].toString();
    bool success           = false;

    if (!(QString::compare(altLink, QString::fromLatin1(""), Qt::CaseInsensitive) == 0))
        success = true;

    emit signalBusy(false);

    if (!success)
    {
        emit signalAddPhotoDone(0, i18n("Failed to upload photo"), QString::fromLatin1("-1"));
    }
    else
    {
        emit signalAddPhotoDone(1, QString(), photoId);
    }
}

void GDTalker::cancel()
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(false);
}

} // namespace Digikam

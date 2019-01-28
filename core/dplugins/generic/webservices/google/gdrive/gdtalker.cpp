/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
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

#include "gdtalker.h"

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

using namespace Digikam;

namespace DigikamGenericGoogleServicesPlugin
{

static bool gdriveLessThan(const GSFolder& p1, const GSFolder& p2)
{
    return (p1.title.toLower() < p2.title.toLower());
}

class Q_DECL_HIDDEN GDTalker::Private
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
        apiUrl         = QLatin1String("https://www.googleapis.com/drive/v2/%1");
        uploadUrl      = QLatin1String("https://www.googleapis.com/upload/drive/v2/files");
        state          = GD_LOGOUT;
        netMngr        = 0;
        rootid         = QLatin1String("root");
        rootfoldername = QLatin1String("GoogleDrive Root");
        listPhotoId    = QStringList();
    }

public:

    QString                apiUrl;
    QString                uploadUrl;
    QString                rootid;
    QString                rootfoldername;
    QString                username;
    State                  state;
    QStringList            listPhotoId;

    QNetworkAccessManager* netMngr;
};

GDTalker::GDTalker(QWidget* const parent)
    : GSTalkerBase(parent, QStringList(QLatin1String("https://www.googleapis.com/auth/drive")), QLatin1String("GoogleDrive")), 
      d(new Private)
{
    d->netMngr = new QNetworkAccessManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    connect(this, SIGNAL(signalReadyToUpload()),
            this, SLOT(slotUploadPhoto()));
}

GDTalker::~GDTalker()
{
    if (m_reply)
    {
        m_reply->abort();
    }

    WSToolUtils::removeTemporaryDir("google");

    delete d;
}

/**
 * Gets username
 */
void GDTalker::getUserName()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "getUserName";

    QUrl url(d->apiUrl.arg(QLatin1String("about")));

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1());

    m_reply  = d->netMngr->get(netRequest);
    d->state = Private::GD_USERNAME;
    emit signalBusy(true);
}

/**
 * Gets list of folder of user in json format
 */
void GDTalker::listFolders()
{
    QUrl url(d->apiUrl.arg(QLatin1String("files")));

    QUrlQuery q;
    q.addQueryItem(QLatin1String("q"), QLatin1String("mimeType = 'application/vnd.google-apps.folder'"));

    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1());

    m_reply  = d->netMngr->get(netRequest);
    d->state = Private::GD_LISTFOLDERS;
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

    QUrl url(d->apiUrl.arg(QLatin1String("files")));
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

    QString path(imgPath);

    QMimeDatabase mimeDB;

    if (mimeDB.mimeTypeForFile(imgPath).name().startsWith(QLatin1String("image/")))
    {
        QImage image = PreviewLoadThread::loadHighQualitySynchronously(imgPath).copyQImage();

        if (image.isNull())
        {
            image.load(imgPath);
        }

        if (image.isNull())
        {
            emit signalBusy(false);
            return false;
        }

        path = WSToolUtils::makeTemporaryDir("google").filePath(QFileInfo(imgPath)
                                             .baseName().trimmed() + QLatin1String(".jpg"));

        if (rescale && (image.width() > maxDim || image.height() > maxDim))
        {
            image = image.scaled(maxDim,maxDim,Qt::KeepAspectRatio,Qt::SmoothTransformation);
        }

        image.save(path, "JPEG", imageQuality);

        DMetadata meta;

        if (meta.load(imgPath))
        {
            meta.setItemDimensions(image.size());
            meta.setItemOrientation(MetaEngine::ORIENTATION_NORMAL);
            meta.setMetadataWritingMode((int)DMetadata::WRITE_TO_FILE_ONLY);
            meta.save(path, true);
        }
    }

    GDMPForm form;
    form.addPair(QUrl::fromLocalFile(imgPath).fileName(), info.description, imgPath, id);

    if (!form.addFile(path))
    {
        emit signalBusy(false);
        return false;
    }

    form.finish();

    QUrl url(d->uploadUrl);

    QUrlQuery q;
    q.addQueryItem(QLatin1String("uploadType"), QLatin1String("multipart"));

    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1());
    netRequest.setRawHeader("Host", "www.googleapis.com");

    m_reply = d->netMngr->post(netRequest, form.formData());

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In add photo";
    d->state = Private::GD_ADDPHOTO;
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

    QByteArray buffer =  reply->readAll();

    switch (d->state)
    {
        case (Private::GD_LOGOUT):
            break;
        case (Private::GD_LISTFOLDERS):
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In Private::GD_LISTFOLDERS";
            parseResponseListFolders(buffer);
            break;
        case (Private::GD_CREATEFOLDER):
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In Private::GD_CREATEFOLDER";
            parseResponseCreateFolder(buffer);
            break;
        case (Private::GD_ADDPHOTO):
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In Private::GD_ADDPHOTO"; // << buffer;
            parseResponseAddPhoto(buffer);
            break;
        case (Private::GD_USERNAME):
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In Private::GD_USERNAME"; // << buffer;
            parseResponseUserName(buffer);
            break;
        default:
            break;
    }

    reply->deleteLater();
}

void GDTalker::slotUploadPhoto()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << d->listPhotoId.join(QLatin1String(", "));
    emit signalUploadPhotoDone(1, QString(), d->listPhotoId);
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
    qCDebug(DIGIKAM_WEBSERVICES_LOG)<<"User Name is: " << jsonObject[QLatin1String("name")].toString();
    QString temp           = jsonObject[QLatin1String("name")].toString();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "in parseResponseUserName";

    emit signalBusy(false);
    emit signalSetUserName(temp);
}

void GDTalker::parseResponseListFolders(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << doc;

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        emit signalListAlbumsDone(0,i18n("Failed to list folders"),QList<GSFolder>());
        return;
    }

    QJsonObject jsonObject = doc.object();
    QJsonArray jsonArray   = jsonObject[QLatin1String("items")].toArray();

    QList<GSFolder> albumList;
    GSFolder fps;
    fps.id    = d->rootid;
    fps.title = d->rootfoldername;
    albumList.append(fps);

    foreach (const QJsonValue& value, jsonArray)
    {
        QJsonObject obj      = value.toObject();

        // Verify if album is in trash
        QJsonObject labels   = obj[QLatin1String("labels")].toObject();
        bool        trashed  = labels[QLatin1String("trashed")].toBool();

        // Verify if album is editable
        bool        editable = obj[QLatin1String("editable")].toBool();

        /* Verify if album is visualized in a folder inside My Drive
         * If parents is empty, album is shared by another person and not added to My Drive yet
         */
        QJsonArray  parents  = obj[QLatin1String("parents")].toArray();

        fps.id          = obj[QLatin1String("id")].toString();
        fps.title       = obj[QLatin1String("title")].toString();

        if(editable && !trashed && !parents.isEmpty())
        {
            albumList.append(fps);
        }
    }

    std::sort(albumList.begin(), albumList.end(), gdriveLessThan);

    emit signalBusy(false);
    emit signalListAlbumsDone(1, QString(), albumList);
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
    QString temp           = jsonObject[QLatin1String("alternateLink")].toString();
    bool success           = false;

    if (!(QString::compare(temp, QLatin1String(""), Qt::CaseInsensitive) == 0))
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
    QString altLink        = jsonObject[QLatin1String("alternateLink")].toString();
    QString photoId        = jsonObject[QLatin1String("id")].toString();
    bool success           = false;

    if (!(QString::compare(altLink, QLatin1String(""), Qt::CaseInsensitive) == 0))
        success = true;

    emit signalBusy(false);

    if (!success)
    {
        emit signalAddPhotoDone(0, i18n("Failed to upload photo"));
    }
    else
    {
        d->listPhotoId << photoId;
        emit signalAddPhotoDone(1, QString());
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

} // namespace DigikamGenericGoogleServicesPlugin

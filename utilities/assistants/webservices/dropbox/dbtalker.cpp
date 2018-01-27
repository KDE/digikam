/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export images to Dropbox web service
 *
 * Copyright (C) 2013 by Pankaj Kumar <me at panks dot me>
 * Copyright (C) 2018 by Maik Qualmann <metzpinguin at gmail dot com>
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

#include <dbtalker.h>

// Qt includes

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
#include <QMessageBox>
#include <QApplication>
#include <QStandardPaths>
#include <QDesktopServices>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "exportutils.h"
#include "dbwindow.h"
#include "dbitem.h"
#include "dbmpform.h"
#include "previewloadthread.h"

namespace Digikam
{

DBTalker::DBTalker(QWidget* const parent)
{
    m_parent               = parent;
    m_apikey               = QLatin1String("mv2pk07ym9bx3r8");
    m_secret               = QLatin1String("f33sflc8jhiozqu");

    m_authUrl              = QLatin1String("https://www.dropbox.com/oauth2/authorize");
    m_tokenUrl             = QLatin1String("https://api.dropboxapi.com/oauth2/token");

    m_state                = DB_USERNAME;
    m_netMngr              = 0;
    m_reply                = 0;
    m_o2                   = 0;
    m_store                = 0;

    m_netMngr = new QNetworkAccessManager(this);

    connect(m_netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    m_o2      = new O2(this);

    m_o2->setClientId(m_apikey);
    m_o2->setClientSecret(m_secret);
    m_o2->setRefreshTokenUrl(m_tokenUrl);
    m_o2->setRequestUrl(m_authUrl);
    m_o2->setTokenUrl(m_tokenUrl);
    m_o2->setLocalPort(8000);

    QString dkoauth = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1String("/digikam_oauthrc");

    m_settings = new QSettings(dkoauth, QSettings::IniFormat, this);
    m_store    = new O0SettingsStore(m_settings, QLatin1String(O2_ENCRYPTION_KEY), this);
    m_store->setGroupKey(QLatin1String("Dropbox"));
    m_o2->setStore(m_store);

    connect(m_o2, SIGNAL(linkingFailed()),
            this, SLOT(slotLinkingFailed()));

    connect(m_o2, SIGNAL(linkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));

    connect(m_o2, SIGNAL(openBrowser(QUrl)),
            this, SLOT(slotOpenBrowser(QUrl)));
}

DBTalker::~DBTalker()
{
    if (m_reply)
    {
        m_reply->abort();
    }
}

void DBTalker::link()
{
    emit signalBusy(true);
    m_o2->link();
}

void DBTalker::unLink()
{
    m_o2->unlink();

    m_settings->beginGroup(QLatin1String("Dropbox"));
    m_settings->remove(QString());
    m_settings->endGroup();
}

void DBTalker::slotLinkingFailed()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "LINK to Dropbox fail";
    emit signalBusy(false);
}

void DBTalker::slotLinkingSucceeded()
{
    if (!m_o2->linked())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "UNLINK to Dropbox ok";
        emit signalBusy(false);
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "LINK to Dropbox ok";
    emit signalLinkingSucceeded();
}

void DBTalker::slotOpenBrowser(const QUrl& url)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Open Browser...";
    QDesktopServices::openUrl(url);
}

bool DBTalker::authenticated()
{
    return m_o2->linked();
}

/** Creates folder at specified path
 */
void DBTalker::createFolder(const QString& path)
{
    //path also has name of new folder so send path parameter accordingly
    qCDebug(DIGIKAM_GENERAL_LOG) << "createFolder:" << path;

    QUrl url(QLatin1String("https://api.dropboxapi.com/2/files/create_folder_v2"));

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_JSON));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(m_o2->token()).toUtf8());

    QByteArray postData = QString::fromUtf8("{\"path\": \"%1\"}").arg(path).toUtf8();

    m_reply = m_netMngr->post(netRequest, postData);

    m_state = DB_CREATEFOLDER;
    m_buffer.resize(0);
    emit signalBusy(true);
}

/** Get username of dropbox user
 */
void DBTalker::getUserName()
{
    QUrl url(QLatin1String("https://api.dropboxapi.com/2/users/get_current_account"));

    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(m_o2->token()).toUtf8());

    m_reply = m_netMngr->post(netRequest, QByteArray());

    m_state = DB_USERNAME;
    m_buffer.resize(0);
    emit signalBusy(true);
}

/** Get list of folders by parsing json sent by dropbox
 */
void DBTalker::listFolders(const QString& path)
{
    QUrl url(QLatin1String("https://api.dropboxapi.com/2/files/list_folder"));

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_JSON));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(m_o2->token()).toUtf8());

    QByteArray postData = QString::fromUtf8("{\"path\": \"%1\",\"recursive\": true}").arg(path).toUtf8();

    m_reply = m_netMngr->post(netRequest, postData);

    m_state = DB_LISTFOLDERS;
    m_buffer.resize(0);
    emit signalBusy(true);
}

bool DBTalker::addPhoto(const QString& imgPath, const QString& uploadFolder, bool rescale, int maxDim, int imageQuality)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);

    DBMPForm form;
    QImage image = PreviewLoadThread::loadHighQualitySynchronously(imgPath).copyQImage();

    if (image.isNull())
    {
        return false;
    }

    QString path = ExportUtils::makeTemporaryDir("dropbox").filePath(QFileInfo(imgPath)
                                                 .baseName().trimmed() + QLatin1String(".jpg"));

    if (rescale && (image.width() > maxDim || image.height() > maxDim))
    {
        image = image.scaled(maxDim,maxDim, Qt::KeepAspectRatio,Qt::SmoothTransformation);
    }

    image.save(path,"JPEG",imageQuality);

    if (m_meta.load(imgPath))
    {
        m_meta.setImageDimensions(image.size());
        m_meta.setImageOrientation(DMetadata::ORIENTATION_NORMAL);
        m_meta.setImageProgramId(QLatin1String("digiKam"), digiKamVersion());
        m_meta.save(path);
    }

    if (!form.addFile(path))
    {
        emit signalBusy(false);
        return false;
    }

    QString uploadPath = uploadFolder + QUrl(QUrl::fromLocalFile(imgPath)).fileName();
    QUrl url(QLatin1String("https://content.dropboxapi.com/2/files/upload"));

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/octet-stream"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(m_o2->token()).toUtf8());

    QByteArray postData = QString::fromUtf8("{\"path\": \"%1\",\"mode\": \"add\"}").arg(uploadPath).toUtf8();
    netRequest.setRawHeader("Dropbox-API-Arg", postData);

    m_reply = m_netMngr->post(netRequest, form.formData());

    m_state = DB_ADDPHOTO;
    m_buffer.resize(0);
    emit signalBusy(true);
    return true;
}

void DBTalker::cancel()
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(false);
}

void DBTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != m_reply)
    {
        return;
    }

    m_reply = 0;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (m_state != DB_CREATEFOLDER)
        {
            emit signalBusy(false);
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18n("Error"), reply->errorString());

            reply->deleteLater();
            return;
        }
    }

    m_buffer.append(reply->readAll());

    switch (m_state)
    {
        case (DB_LISTFOLDERS):
            qCDebug(DIGIKAM_GENERAL_LOG) << "In DB_LISTFOLDERS";
            parseResponseListFolders(m_buffer);
            break;
        case (DB_CREATEFOLDER):
            qCDebug(DIGIKAM_GENERAL_LOG) << "In DB_CREATEFOLDER";
            parseResponseCreateFolder(m_buffer);
            break;
        case (DB_ADDPHOTO):
            qCDebug(DIGIKAM_GENERAL_LOG) << "In DB_ADDPHOTO";
            parseResponseAddPhoto(m_buffer);
            break;
        case (DB_USERNAME):
            qCDebug(DIGIKAM_GENERAL_LOG) << "In DB_USERNAME";
            parseResponseUserName(m_buffer);
            break;
        default:
            break;
    }

    reply->deleteLater();
}

void DBTalker::parseResponseAddPhoto(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object();
    bool success           = jsonObject.contains(QLatin1String("size"));
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

void DBTalker::parseResponseUserName(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object()[QLatin1String("name")].toObject();

    QString name           = jsonObject[QLatin1String("display_name")].toString();

    emit signalBusy(false);
    emit signalSetUserName(name);
}

void DBTalker::parseResponseListFolders(const QByteArray& data)
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

    QList<QPair<QString, QString> > list;
    list.append(qMakePair(QLatin1String(""), QLatin1String("root")));

    foreach (const QJsonValue& value, jsonArray)
    {
        QString path;
        QString folder;

        QJsonObject obj = value.toObject();
        path            = obj[QLatin1String("path_display")].toString();
        folder          = obj[QLatin1String(".tag")].toString();

        if (folder == QLatin1String("folder"))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Path is" << path;
            QString name = path.section(QLatin1Char('/'), -1);
            list.append(qMakePair(path, name));
        }
    }

    emit signalBusy(false);
    emit signalListAlbumsDone(list);
}

void DBTalker::parseResponseCreateFolder(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object();
    bool fail              = jsonObject.contains(QLatin1String("error"));

    emit signalBusy(false);

    if (fail)
    {
        emit signalCreateFolderFailed(jsonObject[QLatin1String("error_summary")].toString());
    }
    else
    {
        emit signalCreateFolderSucceeded();
    }
}

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-30
 * Description : a tool to export items to Piwigo web service
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006      by Colin Guthrie <kde at colin dot guthr dot ie>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008      by Andrea Diamantini <adjam7 at gmail dot com>
 * Copyright (C) 2010-2014 by Frederic Coiffier <frederic dot coiffier at free dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "piwigotalker.h"

// Qt includes

#include <QByteArray>
#include <QImage>
#include <QRegExp>
#include <QXmlStreamReader>
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>
#include <QCryptographicHash>
#include <QUuid>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmetadata.h"
#include "digikam_debug.h"
#include "piwigoitem.h"
#include "digikam_version.h"
#include "wstoolutils.h"
#include "previewloadthread.h"

namespace Digikam
{

class PiwigoTalker::Private
{
public:

    explicit Private()
    {
        parent     = 0;
        state      = GE_LOGOUT;
        netMngr    = 0;
        reply      = 0;
        loggedIn   = false;
        chunkId    = 0;
        nbOfChunks = 0;
        version    = -1;
        albumId    = 0;
        photoId    = 0;
        iface      = 0;
    }

    QWidget*               parent;
    State                  state;
    QString                cookie;
    QUrl                   url;
    QNetworkAccessManager* netMngr;
    QNetworkReply*         reply;
    bool                   loggedIn;
    QByteArray             talker_buffer;
    uint                   chunkId;
    uint                   nbOfChunks;
    int                    version;

    QByteArray             md5sum;
    QString                path;
    QString                tmpPath;    // If set, contains a temporary file which must be deleted
    int                    albumId;
    int                    photoId;    // Filled when the photo already exist
    QString                comment;    // Synchronized with Piwigo comment
    QString                title;      // Synchronized with Piwigo name
    QString                author;     // Synchronized with Piwigo author
    QDateTime              date;       // Synchronized with Piwigo date
    DInfoInterface*        iface;
};
    
QString PiwigoTalker::s_authToken = QString::fromLatin1("");

PiwigoTalker::PiwigoTalker(DInfoInterface* const iface, QWidget* const parent)
    : d(new Private)
{
    d->parent  = parent;
    d->iface   = iface;
    d->netMngr = new QNetworkAccessManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));
}

PiwigoTalker::~PiwigoTalker()
{
    cancel();
    delete d;
}

void PiwigoTalker::cancel()
{
    deleteTemporaryFile();

    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }
}

QString PiwigoTalker::getAuthToken()
{
    return s_authToken;
}

QByteArray PiwigoTalker::computeMD5Sum(const QString& filepath)
{
    QFile file(filepath);

    if (!file.open(QIODevice::ReadOnly))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "File open error:" << filepath;
        return QByteArray();
    }

    QByteArray md5sum = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5);
    file.close();

    return md5sum;
}

bool PiwigoTalker::loggedIn() const
{
    return d->loggedIn;
}

void PiwigoTalker::login(const QUrl& url, const QString& name, const QString& passwd)
{
    d->url   = url;
    d->state = GE_LOGIN;
    d->talker_buffer.resize(0);

    // Add the page to the URL
    if (!d->url.url().endsWith(QLatin1String(".php")))
    {
        d->url.setPath(d->url.path() + QLatin1Char('/') + QLatin1String("ws.php"));
    }

    s_authToken = QString::fromLatin1(QUuid::createUuid().toByteArray().toBase64());

    QStringList qsl;
    qsl.append(QLatin1String("password=") + QString::fromUtf8(passwd.toUtf8().toPercentEncoding()));
    qsl.append(QLatin1String("method=pwg.session.login"));
    qsl.append(QLatin1String("username=") + QString::fromUtf8(name.toUtf8().toPercentEncoding()));
    QString dataParameters = qsl.join(QLatin1String("&"));
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    QNetworkRequest netRequest(d->url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

    d->reply = d->netMngr->post(netRequest, buffer);

    emit signalBusy(true);
}

void PiwigoTalker::listAlbums()
{
    d->state = GE_LISTALBUMS;
    d->talker_buffer.resize(0);

    QStringList qsl;
    qsl.append(QString::fromLatin1("method=pwg.categories.getList"));
    qsl.append(QString::fromLatin1("recursive=true"));
    QString dataParameters = qsl.join(QString::fromLatin1("&"));
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    QNetworkRequest netRequest(d->url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

    d->reply = d->netMngr->post(netRequest, buffer);

    emit signalBusy(true);
}

bool PiwigoTalker::addPhoto(int   albumId,
                            const QString& mediaPath,
                            bool  rescale,
                            int   maxWidth,
                            int   maxHeight,
                            int   quality)
{
    d->state       = GE_CHECKPHOTOEXIST;
    d->talker_buffer.resize(0);

    d->path        = mediaPath;           // By default, d->path contains the original file
    d->tmpPath     = QString::fromLatin1(""); // By default, no temporary file (except with rescaling)
    d->albumId     = albumId;

    d->md5sum      = computeMD5Sum(mediaPath);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << mediaPath << " " << d->md5sum.toHex();

    if (mediaPath.endsWith(QString::fromLatin1(".mp4"))  || mediaPath.endsWith(QString::fromLatin1(".MP4")) ||
        mediaPath.endsWith(QString::fromLatin1(".ogg"))  || mediaPath.endsWith(QString::fromLatin1(".OGG")) ||
        mediaPath.endsWith(QString::fromLatin1(".webm")) || mediaPath.endsWith(QString::fromLatin1(".WEBM")))
    {
        // Video management
        // Nothing to do
    }
    else
    {
        // Image management

        QImage image = PreviewLoadThread::loadHighQualitySynchronously(mediaPath).copyQImage();

        if (image.isNull())
        {
            image.load(mediaPath);
        }

        if (image.isNull())
        {
            // Invalid image
            return false;
        }

        if (!rescale)
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Upload the original version: " << d->path;
        }
        else
        {
            // Rescale the image
            if (image.width() > maxWidth || image.height() > maxHeight)
            {
                image = image.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }

            d->path = WSToolUtils::makeTemporaryDir("piwigo")
                     .filePath(QUrl::fromLocalFile(mediaPath).fileName());
            d->tmpPath = d->path;
            image.save(d->path, "JPEG", quality);

            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Upload a resized version: " << d->path ;

            // Restore all metadata with EXIF
            // in the resized version

            DMetadata meta;

            if (meta.load(mediaPath))
            {
                meta.setImageDimensions(image.size());
                meta.setImageOrientation(MetaEngine::ORIENTATION_NORMAL);
                meta.setImageProgramId(QString::fromLatin1("digiKam"), digiKamVersion());
                meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
                meta.save(d->path);
            }
            else
            {
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Image " << mediaPath << " has no exif data";
            }
        }
    }

    // Metadata management

    // Complete name and comment for summary sending
    QFileInfo fi(mediaPath);
    d->title   = fi.completeBaseName();
    d->comment = QString::fromLatin1("");
    d->author  = QString::fromLatin1("");
    d->date    = fi.created();

    // Look in the host database

    DItemInfo info(d->iface->itemInfo(mediaPath));

    if (!info.title().isEmpty())
        d->title = info.title();

    if (!info.comment().isEmpty())
        d->comment = info.comment();

    if (!info.creators().isEmpty())
        d->author = info.creators().join(QString::fromLatin1(" / "));

    if (!info.dateTime().isNull())
        d->date = info.dateTime();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Title: "   << d->title;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Comment: " << d->comment;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Author: "  << d->author;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Date: "    << d->date;

    QStringList qsl;
    qsl.append(QLatin1String("method=pwg.images.exist"));
    qsl.append(QLatin1String("md5sud->list=") + QString::fromLatin1(d->md5sum.toHex()));
    QString dataParameters = qsl.join(QLatin1String("&"));
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    QNetworkRequest netRequest(d->url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

    d->reply = d->netMngr->post(netRequest, buffer);

    emit signalProgressInfo(i18n("Check if %1 already exists", QUrl(mediaPath).fileName()));

    emit signalBusy(true);

    return true;
}

void PiwigoTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != d->reply)
    {
        return;
    }

    d->reply     = 0;
    State state = d->state; // Can change in the treatment itself, so we cache it

    if (reply->error() != QNetworkReply::NoError)
    {
        if (state == GE_LOGIN)
        {
            emit signalLoginFailed(reply->errorString());
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << reply->errorString();
        }
        else if (state == GE_GETVERSION)
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << reply->errorString();
            // Version isn't mandatory and errors can be ignored
            // As login succeeded, albums can be listed
            listAlbums();
        }
        else if (state == GE_CHECKPHOTOEXIST || state == GE_GETINFO           ||
                 state == GE_SETINFO         || state == GE_ADDPHOTOCHUNK     ||
                 state == GE_ADDPHOTOSUMMARY)
        {
            deleteTemporaryFile();
            emit signalAddPhotoFailed(reply->errorString());
        }
        else
        {
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18n("Error"), reply->errorString());
        }

        emit signalBusy(false);
        reply->deleteLater();
        return;
    }

    d->talker_buffer.append(reply->readAll());

    switch (state)
    {
        case (GE_LOGIN):
            parseResponseLogin(d->talker_buffer);
            break;
        case (GE_GETVERSION):
            parseResponseGetVersion(d->talker_buffer);
            break;
        case (GE_LISTALBUMS):
            parseResponseListAlbums(d->talker_buffer);
            break;
        case (GE_CHECKPHOTOEXIST):
            parseResponseDoesPhotoExist(d->talker_buffer);
            break;
        case (GE_GETINFO):
            parseResponseGetInfo(d->talker_buffer);
            break;
        case (GE_SETINFO):
            parseResponseSetInfo(d->talker_buffer);
            break;
        case (GE_ADDPHOTOCHUNK):
            // Support for Web API >= 2.4
            parseResponseAddPhotoChunk(d->talker_buffer);
            break;
        case (GE_ADDPHOTOSUMMARY):
            parseResponseAddPhotoSummary(d->talker_buffer);
            break;
        default:   // GE_LOGOUT
            break;
    }

    if (state == GE_GETVERSION && d->loggedIn)
    {
        listAlbums();
    }

    emit signalBusy(false);
    reply->deleteLater();
}

void PiwigoTalker::parseResponseLogin(const QByteArray& data)
{
    QXmlStreamReader ts(data);
    QString line;
    bool foundResponse = false;
    d->loggedIn         = false;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseLogin: " << QString::fromUtf8(data);

    while (!ts.atEnd())
    {
        ts.readNext();

        if (ts.isStartElement())
        {
            foundResponse = true;

            if (ts.name() == QString::fromLatin1("rsp") &&
                ts.attributes().value(QString::fromLatin1("stat")) == QString::fromLatin1("ok"))
            {
                d->loggedIn = true;

                /** Request Version */
                d->state           = GE_GETVERSION;
                d->talker_buffer.resize(0);
                d->version         = -1;

                QByteArray buffer = "method=pwg.getVersion";

                QNetworkRequest netRequest(d->url);
                netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
                netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

                d->reply = d->netMngr->post(netRequest, buffer);

                emit signalBusy(true);

                return;
            }
        }
    }

    if (!foundResponse)
    {
        emit signalLoginFailed(i18n("Piwigo URL probably incorrect"));
        return;
    }

    if (!d->loggedIn)
    {
        emit signalLoginFailed(i18n("Incorrect username or password specified"));
    }
}

void PiwigoTalker::parseResponseGetVersion(const QByteArray& data)
{
    QXmlStreamReader ts(data);
    QString line;
    QRegExp verrx(QString::fromLatin1(".?(\\d)\\.(\\d).*"));

    bool foundResponse = false;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseGetVersion: " << QString::fromUtf8(data);

    while (!ts.atEnd())
    {
        ts.readNext();

        if (ts.isStartElement())
        {
            foundResponse = true;

            if (ts.name() == QString::fromLatin1("rsp") &&
                ts.attributes().value(QString::fromLatin1("stat")) == QString::fromLatin1("ok"))
            {
                QString v = ts.readElementText();

                if (verrx.exactMatch(v))
                {
                    QStringList qsl = verrx.capturedTexts();
                    d->version       = qsl[1].toInt() * 10 + qsl[2].toInt();
                    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Version: " << d->version;
                    break;
                }
            }
        }
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "foundResponse : " << foundResponse;

    if (d->version < PIWIGO_VER_2_4)
    {
        d->loggedIn = false;
        emit signalLoginFailed(i18n("Upload to Piwigo version < 2.4 is no longer supported"));
        return;
    }
}

void PiwigoTalker::parseResponseListAlbums(const QByteArray& data)
{
    QString str        = QString::fromUtf8(data);
    QXmlStreamReader ts(data);
    QString line;
    bool foundResponse = false;
    bool success       = false;

    typedef QList<PiwigoAlbum> PiwigoAlbumList;
    PiwigoAlbumList albumList;
    PiwigoAlbumList::iterator iter = albumList.begin();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseListAlbums";

    while (!ts.atEnd())
    {
        ts.readNext();

        if (ts.isEndElement() && ts.name() == QString::fromLatin1("categories"))
            break;

        if (ts.isStartElement())
        {
            if (ts.name() == QString::fromLatin1("rsp") &&
                ts.attributes().value(QString::fromLatin1("stat")) == QString::fromLatin1("ok"))
            {
                foundResponse = true;
            }

            if (ts.name() == QString::fromLatin1("categories"))
            {
                success = true;
            }

            if (ts.name() == QString::fromLatin1("category"))
            {
                PiwigoAlbum album;
                album.m_refNum       = ts.attributes().value(QString::fromLatin1("id")).toString().toInt();
                album.m_parentRefNum = -1;

                qCDebug(DIGIKAM_WEBSERVICES_LOG) << album.m_refNum << "\n";

                iter = albumList.insert(iter, album);
            }

            if (ts.name() == QString::fromLatin1("name"))
            {
                (*iter).m_name = ts.readElementText();
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << (*iter).m_name << "\n";
            }

            if (ts.name() == QString::fromLatin1("uppercats"))
            {
                QString uppercats   = ts.readElementText();
                QStringList catlist = uppercats.split(QLatin1Char(','));

                if (catlist.size() > 1 && catlist.at((uint)catlist.size() - 2).toInt() != (*iter).m_refNum)
                {
                    (*iter).m_parentRefNum = catlist.at((uint)catlist.size() - 2).toInt();
                    qCDebug(DIGIKAM_WEBSERVICES_LOG) << (*iter).m_parentRefNum << "\n";
                }
            }
        }
    }

    if (!foundResponse)
    {
        emit signalError(i18n("Invalid response received from remote Piwigo"));
        return;
    }

    if (!success)
    {
        emit signalError(i18n("Failed to list albums"));
        return;
    }

    // We need parent albums to come first for rest of the code to work
    std::sort(albumList.begin(), albumList.end());

    emit signalAlbums(albumList);
}

void PiwigoTalker::parseResponseDoesPhotoExist(const QByteArray& data)
{
    QString str        = QString::fromUtf8(data);
    QXmlStreamReader ts(data);
    QString line;
    bool foundResponse = false;
    bool success       = false;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseDoesPhotoExist: " << QString::fromUtf8(data);

    while (!ts.atEnd())
    {
        ts.readNext();

        if (ts.name() == QString::fromLatin1("rsp"))
        {
            foundResponse = true;

            if (ts.attributes().value(QString::fromLatin1("stat")) == QString::fromLatin1("ok"))
                success = true;

            // Originally, first versions of Piwigo 2.4.x returned an invalid XML as the element started with a digit
            // New versions are corrected (starting with _) : This code works with both versions
            QRegExp md5rx(QString::fromLatin1("_?([a-f0-9]+)>([0-9]+)</.+"));

            ts.readNext();

            if (md5rx.exactMatch(QString::fromUtf8(data.mid(ts.characterOffset()))))
            {
                QStringList qsl = md5rx.capturedTexts();

                if (qsl[1] == QString::fromLatin1(d->md5sum.toHex()))
                {
                    d->photoId = qsl[2].toInt();
                    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "d->photoId: " << d->photoId;

                    emit signalProgressInfo(i18n("Photo '%1' already exists.", d->title));

                    d->state   = GE_GETINFO;
                    d->talker_buffer.resize(0);

                    QStringList qsl;
                    qsl.append(QString::fromLatin1("method=pwg.images.getInfo"));
                    qsl.append(QString::fromLatin1("image_id=") + QString::number(d->photoId));
                    QString dataParameters = qsl.join(QString::fromLatin1("&"));
                    QByteArray buffer;
                    buffer.append(dataParameters.toUtf8());

                    QNetworkRequest netRequest(d->url);
                    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
                    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

                    d->reply = d->netMngr->post(netRequest, buffer);

                    return;
                }
            }
        }
    }

    if (!foundResponse)
    {
        emit signalAddPhotoFailed(i18n("Invalid response received from remote Piwigo"));
        return;
    }

    if (!success)
    {
        emit signalAddPhotoFailed(i18n("Failed to upload photo"));
        return;
    }

    if (d->version >= PIWIGO_VER_2_4)
    {
        QFileInfo fi(d->path);

        d->state      = GE_ADDPHOTOCHUNK;
        d->talker_buffer.resize(0);
        // Compute the number of chunks for the image
        d->nbOfChunks = (fi.size() / CHUNK_MAX_SIZE) + 1;
        d->chunkId    = 0;

        addNextChunk();
    }
    else
    {
        emit signalAddPhotoFailed(i18n("Upload to Piwigo version < 2.4 is no longer supported"));
        return;
    }
}

void PiwigoTalker::parseResponseGetInfo(const QByteArray& data)
{
    QString str        = QString::fromUtf8(data);
    QXmlStreamReader ts(data);
    QString line;
    bool foundResponse = false;
    bool success       = false;
    QList<int> categories;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseGetInfo: " << QString::fromUtf8(data);

    while (!ts.atEnd())
    {
        ts.readNext();

        if (ts.isStartElement())
        {
            if (ts.name() == QString::fromLatin1("rsp"))
            {
                foundResponse = true;

                if (ts.attributes().value(QString::fromLatin1("stat")) == QString::fromLatin1("ok"))
                    success = true;
            }

            if (ts.name() == QString::fromLatin1("category"))
            {
                if (ts.attributes().hasAttribute(QString::fromLatin1("id")))
                {
                    QString id(ts.attributes().value(QString::fromLatin1("id")).toString());
                    categories.append(id.toInt());
                }
            }
        }
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "success : " << success;

    if (!foundResponse)
    {
        emit signalAddPhotoFailed(i18n("Invalid response received from remote Piwigo"));
        return;
    }

    if (categories.contains(d->albumId))
    {
        emit signalAddPhotoFailed(i18n("Photo '%1' already exists in this album.", d->title));
        return;
    }
    else
    {
        categories.append(d->albumId);
    }

    d->state = GE_SETINFO;
    d->talker_buffer.resize(0);

    QStringList qsl_cat;

    for (int i = 0; i < categories.size(); ++i)
    {
        qsl_cat.append(QString::number(categories.at(i)));
    }

    QStringList qsl;
    qsl.append(QLatin1String("method=pwg.images.setInfo"));
    qsl.append(QLatin1String("image_id=") + QString::number(d->photoId));
    qsl.append(QLatin1String("categories=") + QString::fromUtf8(qsl_cat.join(QLatin1String(";")).toUtf8().toPercentEncoding()));
    QString dataParameters = qsl.join(QLatin1String("&"));
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    QNetworkRequest netRequest(d->url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

    d->reply = d->netMngr->post(netRequest, buffer);

    return;
}

void PiwigoTalker::parseResponseSetInfo(const QByteArray& data)
{
    QString str        = QString::fromUtf8(data);
    QXmlStreamReader ts(data);
    QString line;
    bool foundResponse = false;
    bool success       = false;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseSetInfo: " << QString::fromUtf8(data);

    while (!ts.atEnd())
    {
        ts.readNext();

        if (ts.isStartElement())
        {
            if (ts.name() == QString::fromLatin1("rsp"))
            {
                foundResponse = true;

                if (ts.attributes().value(QString::fromLatin1("stat")) == QString::fromLatin1("ok"))
                    success = true;

                break;
            }
        }
    }

    if (!foundResponse)
    {
        emit signalAddPhotoFailed(i18n("Invalid response received from remote Piwigo"));
        return;
    }

    if (!success)
    {
        emit signalAddPhotoFailed(i18n("Failed to upload photo"));
        return;
    }

    deleteTemporaryFile();

    emit signalAddPhotoSucceeded();
}

void PiwigoTalker::addNextChunk()
{
    QFile imagefile(d->path);

    if (!imagefile.open(QIODevice::ReadOnly))
    {
        emit signalProgressInfo(i18n("Error : Cannot open photo: %1", QUrl(d->path).fileName()));
        return;
    }

    d->chunkId++; // We start with chunk 1

    imagefile.seek((d->chunkId - 1) * CHUNK_MAX_SIZE);

    d->talker_buffer.resize(0);
    QStringList qsl;
    qsl.append(QLatin1String("method=pwg.images.addChunk"));
    qsl.append(QLatin1String("original_sum=") + QString::fromLatin1(d->md5sum.toHex()));
    qsl.append(QLatin1String("position=") + QString::number(d->chunkId));
    qsl.append(QLatin1String("type=file"));
    qsl.append(QLatin1String("data=") + QString::fromUtf8(imagefile.read(CHUNK_MAX_SIZE).toBase64().toPercentEncoding()));
    QString dataParameters = qsl.join(QLatin1String("&"));
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    imagefile.close();

    QNetworkRequest netRequest(d->url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

    d->reply = d->netMngr->post(netRequest, buffer);

    emit signalProgressInfo(i18n("Upload the chunk %1/%2 of %3", d->chunkId, d->nbOfChunks, QUrl(d->path).fileName()));
}

void PiwigoTalker::parseResponseAddPhotoChunk(const QByteArray& data)
{
    QString str        = QString::fromUtf8(data);
    QXmlStreamReader ts(data);
    QString line;
    bool foundResponse = false;
    bool success       = false;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseAddPhotoChunk: " << QString::fromUtf8(data);

    while (!ts.atEnd())
    {
        ts.readNext();

        if (ts.isStartElement())
        {
            if (ts.name() == QString::fromLatin1("rsp"))
            {
                foundResponse = true;

                if (ts.attributes().value(QString::fromLatin1("stat")) == QString::fromLatin1("ok"))
                    success = true;

                break;
            }
        }
    }

    if (!foundResponse || !success)
    {
        emit signalProgressInfo(i18n("Warning : The full size photo cannot be uploaded."));
    }

    if (d->chunkId < d->nbOfChunks)
    {
        addNextChunk();
    }
    else
    {
        addPhotoSummary();
    }
}

void PiwigoTalker::addPhotoSummary()
{
    d->state = GE_ADDPHOTOSUMMARY;
    d->talker_buffer.resize(0);

    QStringList qsl;
    qsl.append(QLatin1String("method=pwg.images.add"));
    qsl.append(QLatin1String("original_sum=") + QString::fromLatin1(d->md5sum.toHex()));
    qsl.append(QLatin1String("original_filename=") + QString::fromUtf8(QUrl(d->path).fileName().toUtf8().toPercentEncoding()));
    qsl.append(QLatin1String("name=") + QString::fromUtf8(d->title.toUtf8().toPercentEncoding()));

    if (!d->author.isEmpty())
        qsl.append(QLatin1String("author=") + QString::fromUtf8(d->author.toUtf8().toPercentEncoding()));

    if (!d->comment.isEmpty())
        qsl.append(QLatin1String("comment=") + QString::fromUtf8(d->comment.toUtf8().toPercentEncoding()));

    qsl.append(QLatin1String("categories=") + QString::number(d->albumId));
    qsl.append(QLatin1String("file_sum=") + QString::fromLatin1(computeMD5Sum(d->path).toHex()));
    qsl.append(QLatin1String("date_creation=") +
               QString::fromUtf8(d->date.toString(QLatin1String("yyyy-MM-dd hh:mm:ss")).toUtf8().toPercentEncoding()));

    //qsl.append("tag_ids="); // TODO Implement this function
    QString dataParameters = qsl.join(QLatin1String("&"));
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    QNetworkRequest netRequest(d->url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

    d->reply = d->netMngr->post(netRequest, buffer);

    emit signalProgressInfo(i18n("Upload the metadata of %1", QUrl(d->path).fileName()));
}

void PiwigoTalker::parseResponseAddPhotoSummary(const QByteArray& data)
{
    QString str        = QString::fromUtf8(data);
    QXmlStreamReader ts(data.mid(data.indexOf("<?xml")));
    QString line;
    bool foundResponse = false;
    bool success       = false;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseAddPhotoSummary: " << QString::fromUtf8(data);

    while (!ts.atEnd())
    {
        ts.readNext();

        if (ts.isStartElement())
        {
            if (ts.name() == QString::fromLatin1("rsp"))
            {
                foundResponse = true;

                if (ts.attributes().value(QString::fromLatin1("stat")) == QString::fromLatin1("ok"))
                    success = true;

                break;
            }
        }
    }

    if (!foundResponse)
    {
        emit signalAddPhotoFailed(i18n("Invalid response received from remote Piwigo (%1)", QString::fromUtf8(data)));
        return;
    }

    if (!success)
    {
        emit signalAddPhotoFailed(i18n("Failed to upload photo"));
        return;
    }

    deleteTemporaryFile();

    emit signalAddPhotoSucceeded();
}

void PiwigoTalker::deleteTemporaryFile()
{
    if (d->tmpPath.size())
    {
        QFile(d->tmpPath).remove();
        d->tmpPath = QString::fromLatin1("");
    }
}

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of kipi-plugins project
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

#include "digikam_debug.h"
#include "piwigoitem.h"
#include "digikam_version.h"
#include "exportutils.h"

namespace Digikam
{

QString PiwigoTalker::s_authToken = QString::fromLatin1("");

PiwigoTalker::PiwigoTalker(QWidget* const parent)
    : m_parent(parent),
      m_state(GE_LOGOUT),
      m_netMngr(0),
      m_reply(0),
      m_loggedIn(false),
      m_chunkId(0),
      m_nbOfChunks(0),
      m_version(-1),
      m_albumId(0),
      m_photoId(0),
      m_iface(0)
{
    m_netMngr = new QNetworkAccessManager(this);

    connect(m_netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    PluginLoader* const pl = PluginLoader::instance();

    if (pl)
    {
        m_iface = pl->interface();
    }
}

PiwigoTalker::~PiwigoTalker()
{
    cancel();
}

void PiwigoTalker::cancel()
{
    deleteTemporaryFile();

    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }
}

QByteArray PiwigoTalker::computeMD5Sum(const QString& filepath)
{
    QFile file(filepath);

    if (!file.open(QIODevice::ReadOnly))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "File open error:" << filepath;
        return QByteArray();
    }

    QByteArray md5sum = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5);
    file.close();

    return md5sum;
}

bool PiwigoTalker::loggedIn() const
{
    return m_loggedIn;
}

void PiwigoTalker::login(const QUrl& url, const QString& name, const QString& passwd)
{
    m_url   = url;
    m_state = GE_LOGIN;
    m_talker_buffer.resize(0);

    // Add the page to the URL
    if (!m_url.url().endsWith(QLatin1String(".php")))
    {
        m_url.setPath(m_url.path() + QLatin1Char('/') + QLatin1String("ws.php"));
    }

    s_authToken = QString::fromLatin1(QUuid::createUuid().toByteArray().toBase64());

    QStringList qsl;
    qsl.append(QLatin1String("password=") + QString::fromUtf8(passwd.toUtf8().toPercentEncoding()));
    qsl.append(QLatin1String("method=pwg.session.login"));
    qsl.append(QLatin1String("username=") + QString::fromUtf8(name.toUtf8().toPercentEncoding()));
    QString dataParameters = qsl.join(QLatin1String("&"));
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    QNetworkRequest netRequest(m_url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

    m_reply = m_netMngr->post(netRequest, buffer);

    emit signalBusy(true);
}

void PiwigoTalker::listAlbums()
{
    m_state = GE_LISTALBUMS;
    m_talker_buffer.resize(0);

    QStringList qsl;
    qsl.append(QString::fromLatin1("method=pwg.categories.getList"));
    qsl.append(QString::fromLatin1("recursive=true"));
    QString dataParameters = qsl.join(QString::fromLatin1("&"));
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    QNetworkRequest netRequest(m_url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

    m_reply = m_netMngr->post(netRequest, buffer);

    emit signalBusy(true);
}

bool PiwigoTalker::addPhoto(int   albumId,
                            const QString& mediaPath,
                            bool  rescale,
                            int   maxWidth,
                            int   maxHeight,
                            int   quality)
{
    m_state       = GE_CHECKPHOTOEXIST;
    m_talker_buffer.resize(0);

    m_path        = mediaPath;           // By default, m_path contains the original file
    m_tmpPath     = QString::fromLatin1(""); // By default, no temporary file (except with rescaling)
    m_albumId     = albumId;

    m_md5sum      = computeMD5Sum(mediaPath);

    qCDebug(DIGIKAM_GENERAL_LOG) << mediaPath << " " << m_md5sum.toHex();

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
        QImage image;

        if (m_iface)
        {
            image = m_iface->preview(QUrl::fromLocalFile(mediaPath));
        }

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
            qCDebug(DIGIKAM_GENERAL_LOG) << "Upload the original version: " << m_path;
        }
        else
        {
            // Rescale the image
            if (image.width() > maxWidth || image.height() > maxHeight)
            {
                image = image.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }

            m_path = m_tmpPath = makeTemporaryDir("piwigo").filePath(QUrl::fromLocalFile(mediaPath).fileName());
            image.save(m_path, "JPEG", quality);

            qCDebug(DIGIKAM_GENERAL_LOG) << "Upload a resized version: " << m_path ;

            // Restore all metadata with EXIF
            // in the resized version
            if (m_iface)
            {
                QPointer<MetadataProcessor> meta = m_iface->createMetadataProcessor();

                if (meta && meta->load(QUrl::fromLocalFile(mediaPath)))
                {
                    meta->setImageDimensions(image.size());
                    meta->setImageOrientation(MetadataProcessor::NORMAL);
                    meta->setImageProgramId(QString::fromLatin1("Kipi-plugins"), kipipluginsVersion());
                    meta->save(QUrl::fromLocalFile(m_path), true);
                }
                else
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) << "Image " << mediaPath << " has no exif data";
                }
            }
        }
    }

    // Metadata management

    // Complete name and comment for summary sending
    QFileInfo fi(mediaPath);
    m_title   = fi.completeBaseName();
    m_comment = QString::fromLatin1("");
    m_author  = QString::fromLatin1("");
    m_date    = fi.created();

    // Look in the Kipi host database
    KPImageInfo info(QUrl::fromLocalFile(mediaPath));

    if (info.hasTitle() && !info.title().isEmpty())
        m_title = info.title();

    if (info.hasDescription() && !info.description().isEmpty())
        m_comment = info.description();

    if (info.hasCreators() && !info.creators().isEmpty())
        m_author = info.creators().join(QString::fromLatin1(" / "));

    if (info.hasDate())
        m_date = info.date();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Title: "   << m_title;
    qCDebug(DIGIKAM_GENERAL_LOG) << "Comment: " << m_comment;
    qCDebug(DIGIKAM_GENERAL_LOG) << "Author: "  << m_author;
    qCDebug(DIGIKAM_GENERAL_LOG) << "Date: "    << m_date;

    QStringList qsl;
    qsl.append(QLatin1String("method=pwg.images.exist"));
    qsl.append(QLatin1String("md5sum_list=") + QString::fromLatin1(m_md5sum.toHex()));
    QString dataParameters = qsl.join(QLatin1String("&"));
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    QNetworkRequest netRequest(m_url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

    m_reply = m_netMngr->post(netRequest, buffer);

    emit signalProgressInfo(i18n("Check if %1 already exists", QUrl(mediaPath).fileName()));

    emit signalBusy(true);

    return true;
}

void PiwigoTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != m_reply)
    {
        return;
    }

    m_reply     = 0;
    State state = m_state; // Can change in the treatment itself, so we cache it

    if (reply->error() != QNetworkReply::NoError)
    {
        if (state == GE_LOGIN)
        {
            emit signalLoginFailed(reply->errorString());
            qCDebug(DIGIKAM_GENERAL_LOG) << reply->errorString();
        }
        else if (state == GE_GETVERSION)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << reply->errorString();
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

    m_talker_buffer.append(reply->readAll());

    switch (state)
    {
        case (GE_LOGIN):
            parseResponseLogin(m_talker_buffer);
            break;
        case (GE_GETVERSION):
            parseResponseGetVersion(m_talker_buffer);
            break;
        case (GE_LISTALBUMS):
            parseResponseListAlbums(m_talker_buffer);
            break;
        case (GE_CHECKPHOTOEXIST):
            parseResponseDoesPhotoExist(m_talker_buffer);
            break;
        case (GE_GETINFO):
            parseResponseGetInfo(m_talker_buffer);
            break;
        case (GE_SETINFO):
            parseResponseSetInfo(m_talker_buffer);
            break;
        case (GE_ADDPHOTOCHUNK):
            // Support for Web API >= 2.4
            parseResponseAddPhotoChunk(m_talker_buffer);
            break;
        case (GE_ADDPHOTOSUMMARY):
            parseResponseAddPhotoSummary(m_talker_buffer);
            break;
        default:   // GE_LOGOUT
            break;
    }

    if (state == GE_GETVERSION && m_loggedIn)
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
    m_loggedIn         = false;

    qCDebug(DIGIKAM_GENERAL_LOG) << "parseResponseLogin: " << QString::fromUtf8(data);

    while (!ts.atEnd())
    {
        ts.readNext();

        if (ts.isStartElement())
        {
            foundResponse = true;

            if (ts.name() == QString::fromLatin1("rsp") &&
                ts.attributes().value(QString::fromLatin1("stat")) == QString::fromLatin1("ok"))
            {
                m_loggedIn = true;

                /** Request Version */
                m_state           = GE_GETVERSION;
                m_talker_buffer.resize(0);
                m_version         = -1;

                QByteArray buffer = "method=pwg.getVersion";

                QNetworkRequest netRequest(m_url);
                netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
                netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

                m_reply = m_netMngr->post(netRequest, buffer);

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

    if (!m_loggedIn)
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

    qCDebug(DIGIKAM_GENERAL_LOG) << "parseResponseGetVersion: " << QString::fromUtf8(data);

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
                    m_version       = qsl[1].toInt() * 10 + qsl[2].toInt();
                    qCDebug(DIGIKAM_GENERAL_LOG) << "Version: " << m_version;
                    break;
                }
            }
        }
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "foundResponse : " << foundResponse;

    if (m_version < PIWIGO_VER_2_4)
    {
        m_loggedIn = false;
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

    typedef QList<GAlbum> GAlbumList;
    GAlbumList albumList;
    GAlbumList::iterator iter = albumList.begin();

    qCDebug(DIGIKAM_GENERAL_LOG) << "parseResponseListAlbums";

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
                GAlbum album;
                album.ref_num = ts.attributes().value(QString::fromLatin1("id")).toString().toInt();
                album.parent_ref_num = -1;

                qCDebug(DIGIKAM_GENERAL_LOG) << album.ref_num << "\n";

                iter = albumList.insert(iter, album);
            }

            if (ts.name() == QString::fromLatin1("name"))
            {
                (*iter).name = ts.readElementText();
                qCDebug(DIGIKAM_GENERAL_LOG) << (*iter).name << "\n";
            }

            if (ts.name() == QString::fromLatin1("uppercats"))
            {
                QString uppercats   = ts.readElementText();
                QStringList catlist = uppercats.split(QLatin1Char(','));

                if (catlist.size() > 1 && catlist.at(catlist.size() - 2).toInt() != (*iter).ref_num)
                {
                    (*iter).parent_ref_num = catlist.at(catlist.size() - 2).toInt();
                    qCDebug(DIGIKAM_GENERAL_LOG) << (*iter).parent_ref_num << "\n";
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

    qCDebug(DIGIKAM_GENERAL_LOG) << "parseResponseDoesPhotoExist: " << QString::fromUtf8(data);

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

                if (qsl[1] == QString::fromLatin1(m_md5sum.toHex()))
                {
                    m_photoId = qsl[2].toInt();
                    qCDebug(DIGIKAM_GENERAL_LOG) << "m_photoId: " << m_photoId;

                    emit signalProgressInfo(i18n("Photo '%1' already exists.", m_title));

                    m_state   = GE_GETINFO;
                    m_talker_buffer.resize(0);

                    QStringList qsl;
                    qsl.append(QString::fromLatin1("method=pwg.images.getInfo"));
                    qsl.append(QString::fromLatin1("image_id=") + QString::number(m_photoId));
                    QString dataParameters = qsl.join(QString::fromLatin1("&"));
                    QByteArray buffer;
                    buffer.append(dataParameters.toUtf8());

                    QNetworkRequest netRequest(m_url);
                    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
                    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

                    m_reply = m_netMngr->post(netRequest, buffer);

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

    if (m_version >= PIWIGO_VER_2_4)
    {
        QFileInfo fi(m_path);

        m_state      = GE_ADDPHOTOCHUNK;
        m_talker_buffer.resize(0);
        // Compute the number of chunks for the image
        m_nbOfChunks = (fi.size() / CHUNK_MAX_SIZE) + 1;
        m_chunkId    = 0;

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

    qCDebug(DIGIKAM_GENERAL_LOG) << "parseResponseGetInfo: " << QString::fromUtf8(data);

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

    qCDebug(DIGIKAM_GENERAL_LOG) << "success : " << success;

    if (!foundResponse)
    {
        emit signalAddPhotoFailed(i18n("Invalid response received from remote Piwigo"));
        return;
    }

    if (categories.contains(m_albumId))
    {
        emit signalAddPhotoFailed(i18n("Photo '%1' already exists in this album.", m_title));
        return;
    }
    else
    {
        categories.append(m_albumId);
    }

    m_state = GE_SETINFO;
    m_talker_buffer.resize(0);

    QStringList qsl_cat;

    for (int i = 0; i < categories.size(); ++i)
    {
        qsl_cat.append(QString::number(categories.at(i)));
    }

    QStringList qsl;
    qsl.append(QLatin1String("method=pwg.images.setInfo"));
    qsl.append(QLatin1String("image_id=") + QString::number(m_photoId));
    qsl.append(QLatin1String("categories=") + QString::fromUtf8(qsl_cat.join(QLatin1String(";")).toUtf8().toPercentEncoding()));
    QString dataParameters = qsl.join(QLatin1String("&"));
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    QNetworkRequest netRequest(m_url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

    m_reply = m_netMngr->post(netRequest, buffer);

    return;
}

void PiwigoTalker::parseResponseSetInfo(const QByteArray& data)
{
    QString str        = QString::fromUtf8(data);
    QXmlStreamReader ts(data);
    QString line;
    bool foundResponse = false;
    bool success       = false;

    qCDebug(DIGIKAM_GENERAL_LOG) << "parseResponseSetInfo: " << QString::fromUtf8(data);

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
    QFile imagefile(m_path);

    if (!imagefile.open(QIODevice::ReadOnly))
    {
        emit signalProgressInfo(i18n("Error : Cannot open photo: %1", QUrl(m_path).fileName()));
        return;
    }

    m_chunkId++; // We start with chunk 1

    imagefile.seek((m_chunkId - 1) * CHUNK_MAX_SIZE);

    m_talker_buffer.resize(0);
    QStringList qsl;
    qsl.append(QLatin1String("method=pwg.images.addChunk"));
    qsl.append(QLatin1String("original_sum=") + QString::fromLatin1(m_md5sum.toHex()));
    qsl.append(QLatin1String("position=") + QString::number(m_chunkId));
    qsl.append(QLatin1String("type=file"));
    qsl.append(QLatin1String("data=") + QString::fromUtf8(imagefile.read(CHUNK_MAX_SIZE).toBase64().toPercentEncoding()));
    QString dataParameters = qsl.join(QLatin1String("&"));
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    imagefile.close();

    QNetworkRequest netRequest(m_url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

    m_reply = m_netMngr->post(netRequest, buffer);

    emit signalProgressInfo(i18n("Upload the chunk %1/%2 of %3", m_chunkId, m_nbOfChunks, QUrl(m_path).fileName()));
}

void PiwigoTalker::parseResponseAddPhotoChunk(const QByteArray& data)
{
    QString str        = QString::fromUtf8(data);
    QXmlStreamReader ts(data);
    QString line;
    bool foundResponse = false;
    bool success       = false;

    qCDebug(DIGIKAM_GENERAL_LOG) << "parseResponseAddPhotoChunk: " << QString::fromUtf8(data);

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

    if (m_chunkId < m_nbOfChunks)
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
    m_state = GE_ADDPHOTOSUMMARY;
    m_talker_buffer.resize(0);

    QStringList qsl;
    qsl.append(QLatin1String("method=pwg.images.add"));
    qsl.append(QLatin1String("original_sum=") + QString::fromLatin1(m_md5sum.toHex()));
    qsl.append(QLatin1String("original_filename=") + QString::fromUtf8(QUrl(m_path).fileName().toUtf8().toPercentEncoding()));
    qsl.append(QLatin1String("name=") + QString::fromUtf8(m_title.toUtf8().toPercentEncoding()));

    if (!m_author.isEmpty())
        qsl.append(QLatin1String("author=") + QString::fromUtf8(m_author.toUtf8().toPercentEncoding()));

    if (!m_comment.isEmpty())
        qsl.append(QLatin1String("comment=") + QString::fromUtf8(m_comment.toUtf8().toPercentEncoding()));

    qsl.append(QLatin1String("categories=") + QString::number(m_albumId));
    qsl.append(QLatin1String("file_sum=") + QString::fromLatin1(computeMD5Sum(m_path).toHex()));
    qsl.append(QLatin1String("date_creation=") +
               QString::fromUtf8(m_date.toString(QLatin1String("yyyy-MM-dd hh:mm:ss")).toUtf8().toPercentEncoding()));

    //qsl.append("tag_ids="); // TODO Implement this function
    QString dataParameters = qsl.join(QLatin1String("&"));
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    QNetworkRequest netRequest(m_url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

    m_reply = m_netMngr->post(netRequest, buffer);

    emit signalProgressInfo(i18n("Upload the metadata of %1", QUrl(m_path).fileName()));
}

void PiwigoTalker::parseResponseAddPhotoSummary(const QByteArray& data)
{
    QString str        = QString::fromUtf8(data);
    QXmlStreamReader ts(data.mid(data.indexOf("<?xml")));
    QString line;
    bool foundResponse = false;
    bool success       = false;

    qCDebug(DIGIKAM_GENERAL_LOG) << "parseResponseAddPhotoSummary: " << QString::fromUtf8(data);

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
    if (m_tmpPath.size())
    {
        QFile(m_tmpPath).remove();
        m_tmpPath = QString::fromLatin1("");
    }
}

} // namespace Digikam

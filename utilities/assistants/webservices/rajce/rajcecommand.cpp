/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * Copyright (C) 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "rajcecommand.h"

// Qt includes

#include <QWidget>
#include <QMutex>
#include <QQueue>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QCryptographicHash>
#include <QXmlResultItems>
#include <QFileInfo>
#include <QUrl>

// Local includes

#include "digikam_debug.h"
#include "rajcempform.h"
#include "digikam_version.h"
#include "dmetadata.h"
#include "wstoolutils.h"
#include "previewloadthread.h"

namespace Digikam
{

const unsigned THUMB_SIZE = 100;

struct PreparedImage
{
    QString scaledImagePath;
    QString thumbPath;
};

PreparedImage s_prepareImageForUpload(const QString& saveDir, const QImage& img, const QString& imagePath,
                                      unsigned maxDimension, unsigned thumbDimension, int jpgQuality)
{
    PreparedImage ret;

    if (img.isNull())
        return ret;

    QImage image(img);

    // get temporary file name
    QString baseName    = saveDir  + QFileInfo(imagePath).baseName().trimmed();
    ret.scaledImagePath = baseName + QString::fromLatin1(".jpg");
    ret.thumbPath       = baseName + QString::fromLatin1(".thumb.jpg");

    if (maxDimension > 0 && ((unsigned) image.width() > maxDimension || (unsigned) image.height() > maxDimension))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Resizing to " << maxDimension;
        image = image.scaled(maxDimension, maxDimension, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Saving to temp file: " << ret.scaledImagePath;
    image.save(ret.scaledImagePath, "JPEG", jpgQuality);

    QImage thumb = image.scaled(thumbDimension, thumbDimension, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Saving thumb to temp file: " << ret.thumbPath;
    thumb.save(ret.thumbPath, "JPEG", jpgQuality);

    // copy meta data to temporary image

    DMetadata meta;

    if (meta.load(imagePath))
    {
        meta.setImageDimensions(image.size());
        meta.setImageOrientation(MetaEngine::ORIENTATION_NORMAL);
        meta.setImageProgramId(QString::fromLatin1("digiKam"), digiKamVersion());
        meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
        meta.save(ret.scaledImagePath);
    }

    return ret;
}

// -----------------------------------------------------------------------

RajceCommand::RajceCommand(const QString& name, RajceCommandType type)
    : m_name(name),
      m_commandType(type)
{
}

RajceCommand::~RajceCommand()
{
}

QMap<QString, QString>& RajceCommand::parameters() const
{
    return const_cast<QMap<QString, QString> &>(m_parameters);
}

QString RajceCommand::getXml() const
{
    QString ret(QString::fromLatin1("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"));

    ret.append(QString::fromLatin1("<request>\n"));
    ret.append(QString::fromLatin1("  <command>")).append(m_name).append(QString::fromLatin1("</command>\n"));
    ret.append(QString::fromLatin1("  <parameters>\n"));

    foreach (QString key, m_parameters.keys())
    {
        ret.append(QString::fromLatin1("    <")).append(key).append(QString::fromLatin1(">"));
        ret.append(m_parameters[key]);
        ret.append(QString::fromLatin1("</")).append(key).append(QString::fromLatin1(">\n"));
    }

    ret.append(QString::fromLatin1("</parameters>\n"));
    ret.append(additionalXml());
    ret.append(QString::fromLatin1("\n</request>\n"));

    return ret;
}

bool RajceCommand::_parseError(QXmlQuery& query, RajceSession& state)
{
    QString results;

    query.setQuery(QString::fromLatin1("/response/string(errorCode)"));
    query.evaluateTo(&results);

    if (results.trimmed().length() > 0)
    {
        state.lastErrorCode()    = results.toUInt();
        query.setQuery(QString::fromLatin1("/response/string(result)"));
        query.evaluateTo(&results);
        state.lastErrorMessage() = results.trimmed();

        return true;
    }

    return false;
}

void RajceCommand::processResponse(const QString& response, RajceSession& state)
{
    QXmlQuery q;
    q.setFocus(response);

    state.lastCommand() = m_commandType;

    if (_parseError(q, state))
    {
        cleanUpOnError(state);
    }
    else
    {
        parseResponse(q, state);
    }
}

QString RajceCommand::additionalXml() const
{
    return QString();
}

QByteArray RajceCommand::encode() const
{
    QByteArray ret = QString::fromLatin1("data=").toLatin1();
    ret.append(QUrl::toPercentEncoding(getXml()));

    return ret;
}

QString RajceCommand::contentType() const
{
    return QString::fromLatin1("application/x-www-form-urlencoded");
}

RajceCommandType RajceCommand::commandType() const
{
    return m_commandType;
}

// -----------------------------------------------------------------------

OpenAlbumCommand::OpenAlbumCommand(unsigned albumId, const RajceSession& state)
    : RajceCommand(QString::fromLatin1("openAlbum"), OpenAlbum)
{
    parameters()[QString::fromLatin1("token")]   = state.sessionToken();
    parameters()[QString::fromLatin1("albumID")] = QString::number(albumId);
}

void OpenAlbumCommand::parseResponse(QXmlQuery& q, RajceSession& state)
{
    state.openAlbumToken() = QString();

    QString result;

    q.setQuery(QString::fromLatin1("/response/data(albumToken)"));
    q.evaluateTo(&result);

    state.openAlbumToken() = result.trimmed();
}

void OpenAlbumCommand::cleanUpOnError(RajceSession& state)
{
    state.openAlbumToken() = QString();
}

// -----------------------------------------------------------------------

LoginCommand::LoginCommand(const QString& username, const QString& password)
    : RajceCommand(QString::fromLatin1("login"), Login)
{
    parameters()[QString::fromLatin1("login")]    = username;
    parameters()[QString::fromLatin1("password")] = QString::fromLatin1(
        QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5).toHex());
}

void LoginCommand::parseResponse(QXmlQuery& q, RajceSession& state)
{
    QString results;

    q.setQuery(QString::fromLatin1("/response/string(maxWidth)"));
    q.evaluateTo(&results);
    state.maxWidth()     = results.toUInt();

    q.setQuery(QString::fromLatin1("/response/string(maxHeight)"));
    q.evaluateTo(&results);
    state.maxHeight()    = results.toUInt();

    q.setQuery(QString::fromLatin1("/response/string(quality)"));
    q.evaluateTo(&results);
    state.imageQuality() = results.toUInt();

    q.setQuery(QString::fromLatin1("/response/string(nick)"));
    q.evaluateTo(&results);
    state.nickname()     = results.trimmed();

    q.setQuery(QString::fromLatin1("data(/response/sessionToken)"));
    q.evaluateTo(&results);
    state.sessionToken() = results.trimmed();

    state.username()     = parameters()[QString::fromLatin1("login")];
}

void LoginCommand::cleanUpOnError(RajceSession& state)
{
    state.openAlbumToken() = QString::fromLatin1("");
    state.nickname()       = QString::fromLatin1("");
    state.username()       = QString::fromLatin1("");
    state.imageQuality()   = 0;
    state.maxHeight()      = 0;
    state.maxWidth()       = 0;
    state.sessionToken()   = QString::fromLatin1("");
    state.albums().clear();
}

// -----------------------------------------------------------------------

CreateAlbumCommand::CreateAlbumCommand(const QString& name, const QString& description,
                                       bool visible, const RajceSession& state)
    : RajceCommand(QString::fromLatin1("createAlbum"), CreateAlbum)
{
    parameters()[QString::fromLatin1("token")]            = state.sessionToken();
    parameters()[QString::fromLatin1("albumName")]        = name;
    parameters()[QString::fromLatin1("albumDescription")] = description;
    parameters()[QString::fromLatin1("albumVisible")]     = visible ? QString::fromLatin1("1") : QString::fromLatin1("0");
}

void CreateAlbumCommand::parseResponse(QXmlQuery&, RajceSession&)
{
}

void CreateAlbumCommand::cleanUpOnError(RajceSession&)
{
}

void CloseAlbumCommand::parseResponse(QXmlQuery&, RajceSession&)
{
}

void CloseAlbumCommand::cleanUpOnError(RajceSession&)
{
}

// -----------------------------------------------------------------------

CloseAlbumCommand::CloseAlbumCommand(const RajceSession& state)
    : RajceCommand(QString::fromLatin1("closeAlbum"), CloseAlbum)
{
    parameters()[QString::fromLatin1("token")]      = state.sessionToken();
    parameters()[QString::fromLatin1("albumToken")] = state.openAlbumToken();
}

// -----------------------------------------------------------------------

AlbumListCommand::AlbumListCommand(const RajceSession& state)
    : RajceCommand(QString::fromLatin1("getAlbumList"), ListAlbums)
{
    parameters()[QString::fromLatin1("token")] = state.sessionToken();
}

void AlbumListCommand::parseResponse(QXmlQuery& q, RajceSession& state)
{
    state.albums().clear();

    QXmlResultItems results;

    q.setQuery(QString::fromLatin1("/response/albums/album"));
    q.evaluateTo(&results);

    QXmlItem item(results.next());

    while(!item.isNull())
    {
        q.setFocus(item);
        RajceAlbum album;
        QString detail;

        q.setQuery(QString::fromLatin1("data(./@id)"));
        q.evaluateTo(&detail);
        album.id          = detail.toUInt();

        q.setQuery(QString::fromLatin1("data(./albumName)"));
        q.evaluateTo(&detail);
        album.name        = detail.trimmed();

        q.setQuery(QString::fromLatin1("data(./description)"));
        q.evaluateTo(&detail);
        album.description = detail.trimmed();

        q.setQuery(QString::fromLatin1("data(./url)"));
        q.evaluateTo(&detail);
        album.url         = detail.trimmed();

        q.setQuery(QString::fromLatin1("data(./thumbUrl)"));
        q.evaluateTo(&detail);
        album.thumbUrl    = detail.trimmed();

        q.setQuery(QString::fromLatin1("data(./createDate)"));
        q.evaluateTo(&detail);
        album.createDate  = QDateTime::fromString(detail.trimmed(), QString::fromLatin1("yyyy-MM-dd hh:mm:ss"));

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Create date: " << detail.trimmed() << " = " << QDateTime::fromString(detail.trimmed(), QString::fromLatin1("yyyy-MM-dd hh:mm:ss"));

        q.setQuery(QString::fromLatin1("data(./updateDate)"));
        q.evaluateTo(&detail);
        album.updateDate  = QDateTime::fromString(detail.trimmed(), QString::fromLatin1("yyyy-MM-dd hh:mm:ss"));

        q.evaluateTo(&detail);
        album.isHidden    = detail.toUInt() != 0;

        q.setQuery(QString::fromLatin1("data(./secure)"));
        q.evaluateTo(&detail);
        album.isSecure    = detail.toUInt() != 0;

        q.setQuery(QString::fromLatin1("data(./startDateInterval)"));
        q.evaluateTo(&detail);

        if (detail.trimmed().length() > 0)
        {
            album.validFrom = QDateTime::fromString(detail, QString::fromLatin1("yyyy-MM-dd hh:mm:ss"));
        }

        q.setQuery(QString::fromLatin1("data(./endDateInterval)"));
        q.evaluateTo(&detail);

        if (detail.trimmed().length() > 0)
        {
            album.validTo = QDateTime::fromString(detail, QString::fromLatin1("yyyy-MM-dd hh:mm:ss"));
        }

        q.setQuery(QString::fromLatin1("data(./thumbUrlBest)"));
        q.evaluateTo(&detail);
        album.bestQualityThumbUrl = detail.trimmed();

        state.albums().append(album);
        item = results.next();
    }
}

void AlbumListCommand::cleanUpOnError(RajceSession& state)
{
    state.albums().clear();
}

// -----------------------------------------------------------------------

AddPhotoCommand::AddPhotoCommand(const QString& tmpDir, const QString& path, unsigned dimension,
                                 int jpgQuality, const RajceSession& state)
    : RajceCommand(QString::fromLatin1("addPhoto"), AddPhoto),
      m_jpgQuality(jpgQuality),
      m_desiredDimension(dimension),
      m_maxDimension(0),
      m_tmpDir(tmpDir),
      m_imagePath(path),
      m_form(0)
{
    m_image = PreviewLoadThread::loadHighQualitySynchronously(path).copyQImage();

    if (m_image.isNull())
    {
        m_image.load(path);
    }

    if (m_image.isNull())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Could not read in an image from "
                                         << path << ". Adding the photo will not work.";
        return;
    }

    m_maxDimension                                  = (state.maxHeight() > state.maxWidth()) ? state.maxWidth()
                                                                                             : state.maxHeight();
    parameters()[QString::fromLatin1("token")]      = state.sessionToken();
    parameters()[QString::fromLatin1("albumToken")] = state.openAlbumToken();
    m_form                                          = new RajceMPForm;
}

AddPhotoCommand::~AddPhotoCommand()
{
    delete m_form;
}

void AddPhotoCommand::cleanUpOnError(Digikam::RajceSession&)
{
}

void AddPhotoCommand::parseResponse(QXmlQuery&, Digikam::RajceSession&)
{
}

QString AddPhotoCommand::additionalXml() const
{
    if (m_image.isNull())
    {
        return QString();
    }

    QMap<QString, QString> metadata;
    QFileInfo f(m_imagePath);

    metadata[QString::fromLatin1("FullFilePath")]          = m_imagePath;
    metadata[QString::fromLatin1("OriginalFileName")]      = f.fileName();
    metadata[QString::fromLatin1("OriginalFileExtension")] = QString::fromLatin1(".") + f.suffix();
    metadata[QString::fromLatin1("PerceivedType")]         = QString::fromLatin1("image"); //what are the other values here? video?
    metadata[QString::fromLatin1("OriginalWidth")]         = QString::number(m_image.width());
    metadata[QString::fromLatin1("OriginalHeight")]        = QString::number(m_image.height());
    metadata[QString::fromLatin1("LengthMS")]              = QLatin1Char('0');
    metadata[QString::fromLatin1("FileSize")]              = QString::number(f.size());

    //TODO extract these from exif
    metadata[QString::fromLatin1("Title")]                 = QString::fromLatin1("");
    metadata[QString::fromLatin1("KeywordSet")]            = QString::fromLatin1("");
    metadata[QString::fromLatin1("PeopleRegionSet")]       = QString::fromLatin1("");

    qsrand((uint)QTime::currentTime().msec());
    QString id = QString::number(qrand());
    QString ret(QString::fromLatin1("  <objectInfo>\n    <Item id=\""));
    ret.append(id).append(QString::fromLatin1("\">\n"));

    foreach(QString key, metadata.keys())
    {
        ret.append(QString::fromLatin1("      <")).append(key);
        QString value = metadata[key];

        if (value.length() == 0)
        {
            ret.append(QString::fromLatin1(" />\n"));
        }
        else
        {
            ret.append(QString::fromLatin1(">"));
            ret.append(value);
            ret.append(QString::fromLatin1("</"));
            ret.append(key);
            ret.append(QString::fromLatin1(">\n"));
        }
    }

    ret.append(QString::fromLatin1("    </Item>\n  </objectInfo>\n"));

    return ret;
}

QString AddPhotoCommand::contentType() const
{
    return m_form->contentType();
}

QByteArray AddPhotoCommand::encode() const
{
    if (m_image.isNull())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << m_imagePath << " could not be read, no data will be sent.";
        return QByteArray();
    }

    PreparedImage prepared                      = s_prepareImageForUpload(m_tmpDir,
                                                                          m_image,
                                                                          m_imagePath,
                                                                          m_desiredDimension,
                                                                          THUMB_SIZE,
                                                                          m_jpgQuality);

    //add the rest of the parameters to be encoded as xml
    QImage scaled(prepared.scaledImagePath);
    parameters()[QString::fromLatin1("width")]  = QString::number(scaled.width());
    parameters()[QString::fromLatin1("height")] = QString::number(scaled.height());
    QString xml                                 = getXml();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Really sending:\n" << xml;

    //now we can create the form with all the info
    m_form->reset();

    m_form->addPair(QString::fromLatin1("data"), xml);

    m_form->addFile(QString::fromLatin1("thumb"), prepared.thumbPath);
    m_form->addFile(QString::fromLatin1("photo"), prepared.scaledImagePath);

    QFile::remove(prepared.thumbPath);
    QFile::remove(prepared.scaledImagePath);

    m_form->finish();

    QByteArray ret = m_form->formData();

    return ret;
}

} // namespace Digikam

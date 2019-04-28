/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * Copyright (C) 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

using namespace Digikam;

namespace DigikamGenericRajcePlugin
{

const unsigned THUMB_SIZE = 100;

struct PreparedImage
{
    QString scaledImagePath;
    QString thumbPath;
};

PreparedImage s_prepareImageForUpload(const QString& saveDir,
                                      const QImage& img,
                                      const QString& imagePath,
                                      unsigned maxDimension,
                                      unsigned thumbDimension,
                                      int      jpgQuality)
{
    PreparedImage ret;

    if (img.isNull())
        return ret;

    QImage image(img);

    // get temporary file name
    QString baseName    = saveDir  + QFileInfo(imagePath).baseName().trimmed();
    ret.scaledImagePath = baseName + QLatin1String(".jpg");
    ret.thumbPath       = baseName + QLatin1String(".thumb.jpg");

    if (maxDimension > 0 && ((unsigned) image.width() > maxDimension ||
        (unsigned) image.height() > maxDimension))
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
        meta.setItemDimensions(image.size());
        meta.setItemOrientation(MetaEngine::ORIENTATION_NORMAL);
        meta.setMetadataWritingMode((int)DMetadata::WRITE_TO_FILE_ONLY);
        meta.save(ret.scaledImagePath, true);
    }

    return ret;
}

// -----------------------------------------------------------------------

class Q_DECL_HIDDEN RajceCommand::Private
{
public:

    explicit Private()
    {
        commandType = Logout;
    }

    QString                name;
    RajceCommandType       commandType;
    QMap<QString, QString> parameters;
};

RajceCommand::RajceCommand(const QString& name, RajceCommandType type)
    : d(new Private)
{
    d->name        = name;
    d->commandType = type;
}

RajceCommand::~RajceCommand()
{
    delete d;
}

QMap<QString, QString>& RajceCommand::parameters() const
{
    return const_cast<QMap<QString, QString> &>(d->parameters);
}

QString RajceCommand::getXml() const
{
    QString ret(QLatin1String("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"));

    ret.append(QLatin1String("<request>\n"));
    ret.append(QLatin1String("  <command>")).append(d->name).append(QLatin1String("</command>\n"));
    ret.append(QLatin1String("  <parameters>\n"));

    foreach (QString key, d->parameters.keys())
    {
        ret.append(QLatin1String("    <")).append(key).append(QLatin1String(">"));
        ret.append(d->parameters[key]);
        ret.append(QLatin1String("</")).append(key).append(QLatin1String(">\n"));
    }

    ret.append(QLatin1String("</parameters>\n"));
    ret.append(additionalXml());
    ret.append(QLatin1String("\n</request>\n"));

    return ret;
}

bool RajceCommand::parseErrorFromQuery(QXmlQuery& query, RajceSession& state)
{
    QString results;

    query.setQuery(QLatin1String("/response/string(errorCode)"));
    query.evaluateTo(&results);

    if (results.trimmed().length() > 0)
    {
        state.lastErrorCode()    = results.toUInt();
        query.setQuery(QLatin1String("/response/string(result)"));
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

    state.lastCommand() = d->commandType;

    if (parseErrorFromQuery(q, state))
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
    return QLatin1String("application/x-www-form-urlencoded");
}

RajceCommandType RajceCommand::commandType() const
{
    return d->commandType;
}

// -----------------------------------------------------------------------

OpenAlbumCommand::OpenAlbumCommand(unsigned albumId, const RajceSession& state)
    : RajceCommand(QLatin1String("openAlbum"), OpenAlbum)
{
    parameters()[QLatin1String("token")]   = state.sessionToken();
    parameters()[QLatin1String("albumID")] = QString::number(albumId);
}

void OpenAlbumCommand::parseResponse(QXmlQuery& q, RajceSession& state)
{
    state.openAlbumToken() = QString();

    QString result;

    q.setQuery(QLatin1String("/response/data(albumToken)"));
    q.evaluateTo(&result);

    state.openAlbumToken() = result.trimmed();
}

void OpenAlbumCommand::cleanUpOnError(RajceSession& state)
{
    state.openAlbumToken() = QString();
}

// -----------------------------------------------------------------------

LoginCommand::LoginCommand(const QString& username, const QString& password)
    : RajceCommand(QLatin1String("login"), Login)
{
    parameters()[QLatin1String("login")]    = username;
    parameters()[QLatin1String("password")] = QLatin1String(
        QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5).toHex());
}

void LoginCommand::parseResponse(QXmlQuery& q, RajceSession& state)
{
    QString results;

    q.setQuery(QLatin1String("/response/string(maxWidth)"));
    q.evaluateTo(&results);
    state.maxWidth()     = results.toUInt();

    q.setQuery(QLatin1String("/response/string(maxHeight)"));
    q.evaluateTo(&results);
    state.maxHeight()    = results.toUInt();

    q.setQuery(QLatin1String("/response/string(quality)"));
    q.evaluateTo(&results);
    state.imageQuality() = results.toUInt();

    q.setQuery(QLatin1String("/response/string(nick)"));
    q.evaluateTo(&results);
    state.nickname()     = results.trimmed();

    q.setQuery(QLatin1String("data(/response/sessionToken)"));
    q.evaluateTo(&results);
    state.sessionToken() = results.trimmed();

    state.username()     = parameters()[QLatin1String("login")];
}

void LoginCommand::cleanUpOnError(RajceSession& state)
{
    state.openAlbumToken() = QLatin1String("");
    state.nickname()       = QLatin1String("");
    state.username()       = QLatin1String("");
    state.imageQuality()   = 0;
    state.maxHeight()      = 0;
    state.maxWidth()       = 0;
    state.sessionToken()   = QLatin1String("");
    state.albums().clear();
}

// -----------------------------------------------------------------------

CreateAlbumCommand::CreateAlbumCommand(const QString& name,
                                       const QString& description,
                                       bool visible,
                                       const RajceSession& state)
    : RajceCommand(QLatin1String("createAlbum"), CreateAlbum)
{
    parameters()[QLatin1String("token")]            = state.sessionToken();
    parameters()[QLatin1String("albumName")]        = name;
    parameters()[QLatin1String("albumDescription")] = description;
    parameters()[QLatin1String("albumVisible")]     = visible ? QLatin1String("1") : QLatin1String("0");
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
    : RajceCommand(QLatin1String("closeAlbum"), CloseAlbum)
{
    parameters()[QLatin1String("token")]      = state.sessionToken();
    parameters()[QLatin1String("albumToken")] = state.openAlbumToken();
}

// -----------------------------------------------------------------------

AlbumListCommand::AlbumListCommand(const RajceSession& state)
    : RajceCommand(QLatin1String("getAlbumList"), ListAlbums)
{
    parameters()[QLatin1String("token")] = state.sessionToken();
}

void AlbumListCommand::parseResponse(QXmlQuery& q, RajceSession& state)
{
    state.albums().clear();

    QXmlResultItems results;

    q.setQuery(QLatin1String("/response/albums/album"));
    q.evaluateTo(&results);

    QXmlItem item(results.next());

    while(!item.isNull())
    {
        q.setFocus(item);
        RajceAlbum album;
        QString detail;

        q.setQuery(QLatin1String("data(./@id)"));
        q.evaluateTo(&detail);
        album.id          = detail.toUInt();

        q.setQuery(QLatin1String("data(./albumName)"));
        q.evaluateTo(&detail);
        album.name        = detail.trimmed();

        q.setQuery(QLatin1String("data(./description)"));
        q.evaluateTo(&detail);
        album.description = detail.trimmed();

        q.setQuery(QLatin1String("data(./url)"));
        q.evaluateTo(&detail);
        album.url         = detail.trimmed();

        q.setQuery(QLatin1String("data(./thumbUrl)"));
        q.evaluateTo(&detail);
        album.thumbUrl    = detail.trimmed();

        q.setQuery(QLatin1String("data(./createDate)"));
        q.evaluateTo(&detail);
        album.createDate  = QDateTime::fromString(detail.trimmed(), QLatin1String("yyyy-MM-dd hh:mm:ss"));

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Create date: " << detail.trimmed() << " = "
                                         << QDateTime::fromString(detail.trimmed(), QLatin1String("yyyy-MM-dd hh:mm:ss"));

        q.setQuery(QLatin1String("data(./updateDate)"));
        q.evaluateTo(&detail);
        album.updateDate  = QDateTime::fromString(detail.trimmed(), QLatin1String("yyyy-MM-dd hh:mm:ss"));

        q.evaluateTo(&detail);
        album.isHidden    = detail.toUInt() != 0;

        q.setQuery(QLatin1String("data(./secure)"));
        q.evaluateTo(&detail);
        album.isSecure    = detail.toUInt() != 0;

        q.setQuery(QLatin1String("data(./startDateInterval)"));
        q.evaluateTo(&detail);

        if (detail.trimmed().length() > 0)
        {
            album.validFrom = QDateTime::fromString(detail, QLatin1String("yyyy-MM-dd hh:mm:ss"));
        }

        q.setQuery(QLatin1String("data(./endDateInterval)"));
        q.evaluateTo(&detail);

        if (detail.trimmed().length() > 0)
        {
            album.validTo = QDateTime::fromString(detail, QLatin1String("yyyy-MM-dd hh:mm:ss"));
        }

        q.setQuery(QLatin1String("data(./thumbUrlBest)"));
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

class Q_DECL_HIDDEN AddPhotoCommand::Private
{
public:

    explicit Private()
    {
        jpgQuality       = 90;
        desiredDimension = 0;
        maxDimension     = 0;
        form             = nullptr;
    }

    int          jpgQuality;

    unsigned     desiredDimension;
    unsigned     maxDimension;

    QString      tmpDir;
    QString      imagePath;

    QImage       image;

    RajceMPForm* form;
};

AddPhotoCommand::AddPhotoCommand(const QString& tmpDir,
                                 const QString& path,
                                 unsigned dimension,
                                 int      jpgQuality,
                                 const RajceSession& state)
    : RajceCommand(QLatin1String("addPhoto"), AddPhoto),
      d(new Private)
{
    d->jpgQuality       = jpgQuality;
    d->desiredDimension = dimension;
    d->tmpDir           = tmpDir;
    d->imagePath        = path;
    d->image            = PreviewLoadThread::loadHighQualitySynchronously(path).copyQImage();

    if (d->image.isNull())
    {
        d->image.load(path);
    }

    if (d->image.isNull())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Could not read in an image from "
                                         << path << ". Adding the photo will not work.";
        return;
    }

    d->maxDimension                                 = (state.maxHeight() > state.maxWidth()) ? state.maxWidth()
                                                                                             : state.maxHeight();
    parameters()[QLatin1String("token")]      = state.sessionToken();
    parameters()[QLatin1String("albumToken")] = state.openAlbumToken();
    d->form                                         = new RajceMPForm;
}

AddPhotoCommand::~AddPhotoCommand()
{
    delete d->form;
    delete d;
}

void AddPhotoCommand::cleanUpOnError(RajceSession&)
{
}

void AddPhotoCommand::parseResponse(QXmlQuery&, RajceSession&)
{
}

QString AddPhotoCommand::additionalXml() const
{
    if (d->image.isNull())
    {
        return QString();
    }

    QMap<QString, QString> metadata;
    QFileInfo f(d->imagePath);

    metadata[QLatin1String("FullFilePath")]          = d->imagePath;
    metadata[QLatin1String("OriginalFileName")]      = f.fileName();
    metadata[QLatin1String("OriginalFileExtension")] = QLatin1Char('.') + f.suffix();
    metadata[QLatin1String("PerceivedType")]         = QLatin1String("image"); //what are the other values here? video?
    metadata[QLatin1String("OriginalWidth")]         = QString::number(d->image.width());
    metadata[QLatin1String("OriginalHeight")]        = QString::number(d->image.height());
    metadata[QLatin1String("LengthMS")]              = QLatin1Char('0');
    metadata[QLatin1String("FileSize")]              = QString::number(f.size());

    //TODO extract these from exif
    metadata[QLatin1String("Title")]                 = QLatin1String("");
    metadata[QLatin1String("KeywordSet")]            = QLatin1String("");
    metadata[QLatin1String("PeopleRegionSet")]       = QLatin1String("");

    qsrand((uint)QTime::currentTime().msec());
    QString id = QString::number(qrand());
    QString ret(QLatin1String("  <objectInfo>\n    <Item id=\""));
    ret.append(id).append(QLatin1String("\">\n"));

    foreach(const QString& key, metadata.keys())
    {
        ret.append(QLatin1String("      <")).append(key);
        QString value = metadata[key];

        if (value.length() == 0)
        {
            ret.append(QLatin1String(" />\n"));
        }
        else
        {
            ret.append(QLatin1String(">"));
            ret.append(value);
            ret.append(QLatin1String("</"));
            ret.append(key);
            ret.append(QLatin1String(">\n"));
        }
    }

    ret.append(QLatin1String("    </Item>\n  </objectInfo>\n"));

    return ret;
}

QString AddPhotoCommand::contentType() const
{
    return d->form->contentType();
}

QByteArray AddPhotoCommand::encode() const
{
    if (d->image.isNull())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << d->imagePath << " could not be read, no data will be sent.";
        return QByteArray();
    }

    PreparedImage prepared                      = s_prepareImageForUpload(d->tmpDir,
                                                                          d->image,
                                                                          d->imagePath,
                                                                          d->desiredDimension,
                                                                          THUMB_SIZE,
                                                                          d->jpgQuality);

    //add the rest of the parameters to be encoded as xml
    QImage scaled(prepared.scaledImagePath);
    parameters()[QLatin1String("width")]  = QString::number(scaled.width());
    parameters()[QLatin1String("height")] = QString::number(scaled.height());
    QString xml                                 = getXml();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Really sending:\n" << xml;

    //now we can create the form with all the info
    d->form->reset();

    d->form->addPair(QLatin1String("data"),  xml);

    d->form->addFile(QLatin1String("thumb"), prepared.thumbPath);
    d->form->addFile(QLatin1String("photo"), prepared.scaledImagePath);

    QFile::remove(prepared.thumbPath);
    QFile::remove(prepared.scaledImagePath);

    d->form->finish();

    QByteArray ret = d->form->formData();

    return ret;
}

} // namespace DigikamGenericRajcePlugin

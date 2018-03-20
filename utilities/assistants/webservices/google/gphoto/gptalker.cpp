/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-16-07
 * Description : a tool to export items to Google web services
 *
 * Copyright (C) 2007-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Luka Renko <lure at kubuntu dot org>
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

#include "gptalker.h"

// Qt includes

#include <QMimeDatabase>
#include <QTextDocument>
#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QStringList>
#include <QUrl>
#include <QPointer>
#include <QtAlgorithms>
#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QUrlQuery>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "wstoolutils.h"
#include "digikam_version.h"
#include "gswindow.h"
#include "gpmpform.h"
#include "digikam_debug.h"
#include "previewloadthread.h"
#include "dmetadata.h"

namespace Digikam
{

static bool gphotoLessThan(const GSFolder& p1, const GSFolder& p2)
{
    return (p1.title.toLower() < p2.title.toLower());
}

class GPTalker::Private
{
public:

    enum State
    {
        GP_LOGOUT     = -1,
        GP_LISTALBUMS = 0,
        GP_LISTPHOTOS,
        GP_ADDPHOTO,
        GP_UPDATEPHOTO,
        GP_GETPHOTO,
        GP_CREATEALBUM
    };

public:

    explicit Private()
    {
        state   = GP_LOGOUT;
        netMngr = 0;
    }

public:

    QString                loginName;
    QString                username;
    QString                password;
    QString                userEmailId;
    State                  state;

    QNetworkAccessManager* netMngr;
};

GPTalker::GPTalker(QWidget* const parent)
    : GSTalkerBase(parent, QString::fromLatin1("https://picasaweb.google.com/data/")),
      d(new Private)
{
    m_reply   = 0;
    d->netMngr = new QNetworkAccessManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    connect(this, SIGNAL(signalError(QString)),
            this, SLOT(slotError(QString)));
}

GPTalker::~GPTalker()
{
    if (m_reply)
        m_reply->abort();

    delete d;
}

/**
 * Google Photo's Album listing request/response
 * First a request is sent to the url below and then we might(?) get a redirect URL
 * We then need to send the GET request to the Redirect url.
 * This uses the authenticated album list fetching to get all the albums included the unlisted-albums
 * which is not returned for an unauthorised request as done without the Authorization header.
 */
void GPTalker::listAlbums()
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    QUrl url(QString::fromLatin1("https://picasaweb.google.com/data/feed/api/user/default"));

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));

    if (!m_accessToken.isEmpty())
    {
        netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1());
    }

    m_reply = d->netMngr->get(netRequest);

    d->state = Private::GP_LISTALBUMS;
    m_buffer.resize(0);
    emit signalBusy(true);
}

void GPTalker::listPhotos(const QString& albumId, const QString& imgmax)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    QUrl url(QString::fromLatin1("https://picasaweb.google.com/data/feed/api/user/default/albumid/") + albumId);

    QUrlQuery q(url);
    q.addQueryItem(QString::fromLatin1("thumbsize"), QString::fromLatin1("200"));

    if (!imgmax.isNull())
    {
        q.addQueryItem(QString::fromLatin1("imgmax"), imgmax);
    }

    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    if (!m_accessToken.isEmpty())
    {
        netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1());
    }

    m_reply = d->netMngr->get(netRequest);

    d->state = Private::GP_LISTPHOTOS;
    m_buffer.resize(0);
    emit signalBusy(true);
}

QString GPTalker::token() const
{
    return m_accessToken;
}

void GPTalker::createAlbum(const GSFolder& album)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    //Create the Body in atom-xml
    QDomDocument docMeta;
    QDomProcessingInstruction instr = docMeta.createProcessingInstruction(
        QString::fromLatin1("xml"), QString::fromLatin1("version='1.0' encoding='UTF-8'"));
    docMeta.appendChild(instr);
    QDomElement entryElem           = docMeta.createElement(QString::fromLatin1("entry"));
    docMeta.appendChild(entryElem);
    entryElem.setAttribute(QString::fromLatin1("xmlns"), QString::fromLatin1("http://www.w3.org/2005/Atom"));
    QDomElement titleElem           = docMeta.createElement(QString::fromLatin1("title"));
    entryElem.appendChild(titleElem);
    QDomText titleText              = docMeta.createTextNode(album.title);
    titleElem.appendChild(titleText);
    QDomElement summaryElem         = docMeta.createElement(QString::fromLatin1("summary"));
    entryElem.appendChild(summaryElem);
    QDomText summaryText            = docMeta.createTextNode(album.description);
    summaryElem.appendChild(summaryText);
    QDomElement locationElem        = docMeta.createElementNS(
        QString::fromLatin1("http://schemas.google.com/photos/2007"),
        QString::fromLatin1("gphoto:location"));
    entryElem.appendChild(locationElem);
    QDomText locationText           = docMeta.createTextNode(album.location);
    locationElem.appendChild(locationText);
    QDomElement accessElem          = docMeta.createElementNS(
        QString::fromLatin1("http://schemas.google.com/photos/2007"),
        QString::fromLatin1("gphoto:access"));
    entryElem.appendChild(accessElem);
    QDomText accessText             = docMeta.createTextNode(album.access);
    accessElem.appendChild(accessText);
    QDomElement commentElem         = docMeta.createElementNS(
        QString::fromLatin1("http://schemas.google.com/photos/2007"),
        QString::fromLatin1("gphoto:commentingEnabled"));
    entryElem.appendChild(commentElem);
    QDomText commentText            = docMeta.createTextNode(
        album.canComment ? QString::fromLatin1("true")
                         : QString::fromLatin1("false"));
    commentElem.appendChild(commentText);
    QDomElement timestampElem       = docMeta.createElementNS(
        QString::fromLatin1("http://schemas.google.com/photos/2007"),
        QString::fromLatin1("gphoto:timestamp"));
    entryElem.appendChild(timestampElem);
    QDomText timestampText          = docMeta.createTextNode(album.timestamp);
    timestampElem.appendChild(timestampText);
    QDomElement categoryElem        = docMeta.createElement(QString::fromLatin1("category"));
    entryElem.appendChild(categoryElem);
    categoryElem.setAttribute(
        QString::fromLatin1("scheme"),
        QString::fromLatin1("http://schemas.google.com/g/2005#kind"));
    categoryElem.setAttribute(
        QString::fromLatin1("term"),
        QString::fromLatin1("http://schemas.google.com/photos/2007#album"));
    QDomElement mediaGroupElem      = docMeta.createElementNS(
        QString::fromLatin1("http://search.yahoo.com/mrss/"),
        QString::fromLatin1("media:group"));
    entryElem.appendChild(mediaGroupElem);
    QDomElement mediaKeywordsElem   = docMeta.createElementNS(
        QString::fromLatin1("http://search.yahoo.com/mrss/"),
        QString::fromLatin1("media:keywords"));
    mediaGroupElem.appendChild(mediaKeywordsElem);
    QDomText mediaKeywordsText      = docMeta.createTextNode(album.tags.join(QString::fromLatin1(",")));
    mediaKeywordsElem.appendChild(mediaKeywordsText);

    QByteArray buffer;
    buffer.append(docMeta.toString().toUtf8());

    QUrl url(QString::fromLatin1("https://picasaweb.google.com/data/feed/api/user/default"));

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/atom+xml"));
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1());

    m_reply = d->netMngr->post(netRequest, buffer);

    d->state = Private::GP_CREATEALBUM;
    m_buffer.resize(0);
    emit signalBusy(true);
}

bool GPTalker::addPhoto(const QString& photoPath,
                        GSPhoto& info,
                        const QString& albumId,
                        bool rescale,
                        int maxDim,
                        int imageQuality)
{
   if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    QUrl url(QString::fromLatin1("https://picasaweb.google.com/data/feed/api/user/default/albumid/") + albumId);
    GPMPForm form;
    QString path = photoPath;

    QMimeDatabase mimeDB;

    if (!mimeDB.mimeTypeForFile(path).name().startsWith(QLatin1String("video/")))
    {
        QImage image = PreviewLoadThread::loadHighQualitySynchronously(photoPath).copyQImage();

        if (image.isNull())
        {
            image.load(photoPath);
        }

        if (image.isNull())
        {
            return false;
        }

        path                  = WSToolUtils::makeTemporaryDir("google")
                                             .filePath(QFileInfo(photoPath)
                                             .baseName().trimmed() + 
                                             QLatin1String(".jpg"));
        int imgQualityToApply = 100;

        if (rescale)
        {
            if (image.width() > maxDim || image.height() > maxDim)
                image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);

            imgQualityToApply = imageQuality;
        }

        image.save(path, "JPEG", imgQualityToApply);

        DMetadata meta;

        if (meta.load(photoPath))
        {
            meta.setImageDimensions(image.size());
            meta.setImageOrientation(MetaEngine::ORIENTATION_NORMAL);
            meta.setImageProgramId(QString::fromLatin1("digiKam"), digiKamVersion());
            meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
            meta.save(path);
        }
    }

    // Create the Body in atom-xml

    QDomDocument docMeta;
    QDomProcessingInstruction instr = docMeta.createProcessingInstruction(QString::fromLatin1("xml"), QString::fromLatin1("version='1.0' encoding='UTF-8'"));
    docMeta.appendChild(instr);
    QDomElement entryElem           = docMeta.createElement(QString::fromLatin1("entry"));
    docMeta.appendChild(entryElem);
    entryElem.setAttribute(QString::fromLatin1("xmlns"), QString::fromLatin1("http://www.w3.org/2005/Atom"));
    QDomElement titleElem           = docMeta.createElement(QString::fromLatin1("title"));
    entryElem.appendChild(titleElem);
    QDomText titleText              = docMeta.createTextNode(QFileInfo(path).fileName()); // NOTE: Do not use info.title as arg here to set titleText because we change the format of image as .jpg before uploading.
    titleElem.appendChild(titleText);
    QDomElement summaryElem         = docMeta.createElement(QString::fromLatin1("summary"));
    entryElem.appendChild(summaryElem);
    QDomText summaryText            = docMeta.createTextNode(info.description);
    summaryElem.appendChild(summaryText);
    QDomElement categoryElem        = docMeta.createElement(QString::fromLatin1("category"));
    entryElem.appendChild(categoryElem);
    categoryElem.setAttribute(
        QString::fromLatin1("scheme"),
        QString::fromLatin1("http://schemas.google.com/g/2005#kind"));
    categoryElem.setAttribute(
        QString::fromLatin1("term"),
        QString::fromLatin1("http://schemas.google.com/photos/2007#photo"));
    QDomElement mediaGroupElem      = docMeta.createElementNS(
        QString::fromLatin1("http://search.yahoo.com/mrss/"),
        QString::fromLatin1("media:group"));
    entryElem.appendChild(mediaGroupElem);
    QDomElement mediaKeywordsElem   = docMeta.createElementNS(
        QString::fromLatin1("http://search.yahoo.com/mrss/"),
        QString::fromLatin1("media:keywords"));
    mediaGroupElem.appendChild(mediaKeywordsElem);
    QDomText mediaKeywordsText      = docMeta.createTextNode(info.tags.join(QString::fromLatin1(",")));
    mediaKeywordsElem.appendChild(mediaKeywordsText);

    if (!info.gpsLat.isEmpty() && !info.gpsLon.isEmpty())
    {
        QDomElement whereElem = docMeta.createElementNS(
            QString::fromLatin1("http://www.georss.org/georss"),
            QString::fromLatin1("georss:where"));
        entryElem.appendChild(whereElem);
        QDomElement pointElem = docMeta.createElementNS(
            QString::fromLatin1("http://www.opengis.net/gml"),
            QString::fromLatin1("gml:Point"));
        whereElem.appendChild(pointElem);
        QDomElement gpsElem   = docMeta.createElementNS(
            QString::fromLatin1("http://www.opengis.net/gml"),
            QString::fromLatin1("gml:pos"));
        pointElem.appendChild(gpsElem);
        QDomText gpsVal       = docMeta.createTextNode(info.gpsLat + QLatin1Char(' ') + info.gpsLon);
        gpsElem.appendChild(gpsVal);
    }

    form.addPair(QString::fromLatin1("descr"), docMeta.toString(), QString::fromLatin1("application/atom+xml"));

    if (!form.addFile(QString::fromLatin1("photo"), path))
        return false;

    form.finish();

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1() + "\nMIME-version: 1.0");

    m_reply = d->netMngr->post(netRequest, form.formData());

    d->state = Private::GP_ADDPHOTO;
    m_buffer.resize(0);
    emit signalBusy(true);
    return true;
}

bool GPTalker::updatePhoto(const QString& photoPath, GSPhoto& info/*, const QString& albumId*/,
                                  bool rescale, int maxDim, int imageQuality)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    GPMPForm form;
    QString path = photoPath;

    QMimeDatabase mimeDB;

    if (!mimeDB.mimeTypeForFile(path).name().startsWith(QLatin1String("video/")))
    {
        QImage image = PreviewLoadThread::loadHighQualitySynchronously(photoPath).copyQImage();

        if (image.isNull())
        {
            image.load(photoPath);
        }

        if (image.isNull())
        {
            return false;
        }

        path                  = WSToolUtils::makeTemporaryDir("google")
                                             .filePath(QFileInfo(photoPath)
                                             .baseName().trimmed() +
                                             QLatin1String(".jpg"));
        int imgQualityToApply = 100;

        if (rescale)
        {
            if (image.width() > maxDim || image.height() > maxDim)
                image = image.scaled(maxDim,maxDim, Qt::KeepAspectRatio,Qt::SmoothTransformation);

            imgQualityToApply = imageQuality;
        }

        image.save(path, "JPEG", imgQualityToApply);

        DMetadata meta;

        if (meta.load(photoPath))
        {
            meta.setImageDimensions(image.size());
            meta.setImageOrientation(MetaEngine::ORIENTATION_NORMAL);
            meta.setImageProgramId(QString::fromLatin1("digiKam"), digiKamVersion());
            meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
            meta.save(path);
        }
    }

    //Create the Body in atom-xml
    QDomDocument docMeta;
    QDomProcessingInstruction instr = docMeta.createProcessingInstruction(
        QString::fromLatin1("xml"),
        QString::fromLatin1("version='1.0' encoding='UTF-8'"));
    docMeta.appendChild(instr);
    QDomElement entryElem           = docMeta.createElement(QString::fromLatin1("entry"));
    docMeta.appendChild(entryElem);
    entryElem.setAttribute(
        QString::fromLatin1("xmlns"),
        QString::fromLatin1("http://www.w3.org/2005/Atom"));
    QDomElement titleElem           = docMeta.createElement(QString::fromLatin1("title"));
    entryElem.appendChild(titleElem);
    QDomText titleText              = docMeta.createTextNode(QFileInfo(path).fileName());
    titleElem.appendChild(titleText);
    QDomElement summaryElem         = docMeta.createElement(QString::fromLatin1("summary"));
    entryElem.appendChild(summaryElem);
    QDomText summaryText            = docMeta.createTextNode(info.description);
    summaryElem.appendChild(summaryText);
    QDomElement categoryElem        = docMeta.createElement(QString::fromLatin1("category"));
    entryElem.appendChild(categoryElem);
    categoryElem.setAttribute(
        QString::fromLatin1("scheme"),
        QString::fromLatin1("http://schemas.google.com/g/2005#kind"));
    categoryElem.setAttribute(
        QString::fromLatin1("term"),
        QString::fromLatin1("http://schemas.google.com/photos/2007#photo"));
    QDomElement mediaGroupElem      = docMeta.createElementNS(
        QString::fromLatin1("http://search.yahoo.com/mrss/"),
        QString::fromLatin1("media:group"));
    entryElem.appendChild(mediaGroupElem);
    QDomElement mediaKeywordsElem   = docMeta.createElementNS(
        QString::fromLatin1("http://search.yahoo.com/mrss/"),
        QString::fromLatin1("media:keywords"));
    mediaGroupElem.appendChild(mediaKeywordsElem);
    QDomText mediaKeywordsText      = docMeta.createTextNode(info.tags.join(QString::fromLatin1(",")));
    mediaKeywordsElem.appendChild(mediaKeywordsText);

    if (!info.gpsLat.isEmpty() && !info.gpsLon.isEmpty())
    {
        QDomElement whereElem = docMeta.createElementNS(
            QString::fromLatin1("http://www.georss.org/georss"),
            QString::fromLatin1("georss:where"));
        entryElem.appendChild(whereElem);
        QDomElement pointElem = docMeta.createElementNS(
            QString::fromLatin1("http://www.opengis.net/gml"),
            QString::fromLatin1("gml:Point"));
        whereElem.appendChild(pointElem);
        QDomElement gpsElem   = docMeta.createElementNS(
            QString::fromLatin1("http://www.opengis.net/gml"),
            QString::fromLatin1("gml:pos"));
        pointElem.appendChild(gpsElem);
        QDomText gpsVal       = docMeta.createTextNode(info.gpsLat + QLatin1Char(' ') + info.gpsLon);
        gpsElem.appendChild(gpsVal);
    }

    form.addPair(QString::fromLatin1("descr"), docMeta.toString(), QString::fromLatin1("application/atom+xml"));

    if (!form.addFile(QString::fromLatin1("photo"), path))
        return false;

    form.finish();

    QNetworkRequest netRequest(info.editUrl);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1() + "\nIf-Match: *");

    m_reply = d->netMngr->put(netRequest, form.formData());

    d->state = Private::GP_UPDATEPHOTO;
    m_buffer.resize(0);
    emit signalBusy(true);
    return true;
}

void GPTalker::getPhoto(const QString& imgPath)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);

    QUrl url(imgPath);
    m_reply = d->netMngr->get(QNetworkRequest(url));

    d->state = Private::GP_GETPHOTO;
    m_buffer.resize(0);
}

QString GPTalker::getUserName() const
{
    return d->username;
}

QString GPTalker::getUserEmailId() const
{
    return d->userEmailId;
}

QString GPTalker::getLoginName() const
{
    return d->loginName;
}

void GPTalker::cancel()
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(false);
}

void GPTalker::slotError(const QString & error)
{
    QString transError;
    int     errorNo = 0;

    if (!error.isEmpty())
        errorNo = error.toInt();

    switch (errorNo)
    {
        case 2:
            transError=i18n("No photo specified");
            break;
        case 3:
            transError=i18n("General upload failure");
            break;
        case 4:
            transError=i18n("File-size was zero");
            break;
        case 5:
            transError=i18n("File-type was not recognized");
            break;
        case 6:
            transError=i18n("User exceeded upload limit");
            break;
        case 96:
            transError=i18n("Invalid signature");
            break;
        case 97:
            transError=i18n("Missing signature");
            break;
        case 98:
            transError=i18n("Login failed / Invalid auth token");
            break;
        case 100:
            transError=i18n("Invalid API Key");
            break;
        case 105:
            transError=i18n("Service currently unavailable");
            break;
        case 108:
            transError=i18n("Invalid Frob");
            break;
        case 111:
            transError=i18n("Format \"xxx\" not found");
            break;
        case 112:
            transError=i18n("Method \"xxx\" not found");
            break;
        case 114:
            transError=i18n("Invalid SOAP envelope");
            break;
        case 115:
            transError=i18n("Invalid XML-RPC Method Call");
            break;
        case 116:
            transError=i18n("The POST method is now required for all setters.");
            break;
        default:
            transError=i18n("Unknown error");
    };

    QMessageBox::critical(QApplication::activeWindow(), i18nc("@title:window", "Error"),
                          i18n("Error occurred: %1\nUnable to proceed further.",transError + error));
}

void GPTalker::slotFinished(QNetworkReply* reply)
{
    emit signalBusy(false);

    if (reply != m_reply)
    {
        return;
    }

    m_reply = 0;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (d->state == Private::GP_ADDPHOTO)
        {
            emit signalAddPhotoDone(reply->error(), reply->errorString(), QString::fromLatin1("-1"));
        }
        else
        {
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18n("Error"), reply->errorString());
        }

        reply->deleteLater();
        return;
    }

    m_buffer.append(reply->readAll());

    switch (d->state)
    {
        case (Private::GP_LOGOUT):
            break;
        case (Private::GP_CREATEALBUM):
            parseResponseCreateAlbum(m_buffer);
            break;
        case (Private::GP_LISTALBUMS):
            parseResponseListAlbums(m_buffer);
            break;
        case (Private::GP_LISTPHOTOS):
            parseResponseListPhotos(m_buffer);
            break;
        case (Private::GP_ADDPHOTO):
            parseResponseAddPhoto(m_buffer);
            break;
        case (Private::GP_UPDATEPHOTO):
            emit signalAddPhotoDone(1, QString::fromLatin1(""), QString::fromLatin1(""));
            break;
        case (Private::GP_GETPHOTO):
            // all we get is data of the image
            emit signalGetPhotoDone(1, QString(), m_buffer);
            break;
    }

    reply->deleteLater();
}

void GPTalker::parseResponseListAlbums(const QByteArray& data)
{
    QDomDocument doc(QString::fromLatin1("feed"));
    QString      err;
    int          line;
    int          columns;

    if ( !doc.setContent( data, false, &err, &line, &columns ) )
    {
        qCCritical(DIGIKAM_WEBSERVICES_LOG) << "error is "<< err << " at line " << line << " at columns " << columns;
        emit signalListAlbumsDone(0, i18n("Failed to fetch photo-set list"), QList<GSFolder>());
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode node       = docElem.firstChild();
    QDomElement e;

    QList<GSFolder> albumList;

    while (!node.isNull())
    {
        if (node.isElement() && node.nodeName() == QString::fromLatin1("gphoto:nickname"))
        {
            d->loginName = node.toElement().text();
        }

        if (node.isElement() && node.nodeName() == QString::fromLatin1("gphoto:user"))
        {
            d->username = node.toElement().text();
        }

        if (node.isElement() && node.nodeName() == QString::fromLatin1("entry"))
        {
            e                    = node.toElement();
            QDomNode details     = e.firstChild();
            QDomNode detailsNode = details;
            GSFolder fps;

            while (!detailsNode.isNull())
            {
                if (detailsNode.isElement())
                {
                    if (detailsNode.nodeName() == QString::fromLatin1("gphoto:id"))
                    {
                        fps.id = detailsNode.toElement().text();
                    }

                    if (detailsNode.nodeName() == QString::fromLatin1("title"))
                    {
                        fps.title = detailsNode.toElement().text();
                    }

                    if (detailsNode.nodeName() == QString::fromLatin1("gphoto:access"))
                    {
                        fps.access = detailsNode.toElement().text();
                    }
                }

                detailsNode = detailsNode.nextSibling();
            }

            albumList.append(fps);
        }

        node = node.nextSibling();
    }

    std::sort(albumList.begin(), albumList.end(), gphotoLessThan);
    emit signalListAlbumsDone(1, QString::fromLatin1(""), albumList);
}

void GPTalker::parseResponseListPhotos(const QByteArray& data)
{
    QDomDocument doc(QString::fromLatin1("feed"));

    if ( !doc.setContent( data ) )
    {
        emit signalListPhotosDone(0, i18n("Failed to fetch photo-set list"), QList<GSPhoto>());
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode node       = docElem.firstChild();

    QList<GSPhoto> photoList;

    while (!node.isNull())
    {
        if (node.isElement() && node.nodeName() == QString::fromLatin1("entry"))
        {
            QDomNode details     = node.firstChild();
            QDomNode detailsNode = details;
            GSPhoto fps;

            while (!detailsNode.isNull())
            {
                if (detailsNode.isElement())
                {
                    QDomElement detailsElem = detailsNode.toElement();

                    if (detailsNode.nodeName() == QString::fromLatin1("gphoto:id"))
                    {
                        fps.id = detailsElem.text();
                    }

                    if (detailsNode.nodeName() == QString::fromLatin1("title"))
                    {
                        //fps.title = detailsElem.text();
                    }

                    if (detailsNode.nodeName() == QString::fromLatin1("summary"))
                    {
                        fps.description = detailsElem.text();
                    }

                    if (detailsNode.nodeName() == QString::fromLatin1("gphoto:access"))
                    {
                        fps.access = detailsElem.text();
                    }

                    if (detailsNode.nodeName() == QString::fromLatin1("link") &&
                        detailsElem.attribute(QString::fromLatin1("rel")) == QString::fromLatin1("edit-media"))
                    {
                        fps.editUrl = QUrl(detailsElem.attribute(QString::fromLatin1("href")));
                    }

                    if (detailsNode.nodeName() == QString::fromLatin1("georss:where"))
                    {
                        QDomNode geoPointNode = detailsElem.namedItem(QString::fromLatin1("gml:Point"));

                        if (!geoPointNode.isNull() && geoPointNode.isElement())
                        {
                            QDomNode geoPosNode = geoPointNode.toElement().namedItem(
                                QString::fromLatin1("gml:pos"));

                            if (!geoPosNode.isNull() && geoPosNode.isElement())
                            {
                                QStringList value = geoPosNode.toElement().text().split(QLatin1Char(' '));

                                if (value.size() == 2)
                                {
                                    fps.gpsLat = value[0];
                                    fps.gpsLon = value[1];
                                }
                            }
                        }
                    }

                    if (detailsNode.nodeName() == QString::fromLatin1("media:group"))
                    {
                        QDomNode thumbNode = detailsElem.namedItem(QString::fromLatin1("media:thumbnail"));

                        if (!thumbNode.isNull() && thumbNode.isElement())
                        {
                            fps.thumbURL = QUrl(thumbNode.toElement().attribute(
                                QString::fromLatin1("url"), QString::fromLatin1("")));
                        }

                        QDomNode keywordNode = detailsElem.namedItem(QString::fromLatin1("media:keywords"));

                        if (!keywordNode.isNull() && keywordNode.isElement())
                        {
                            fps.tags = keywordNode.toElement().text().split(QLatin1Char(','));
                        }

                        QDomNodeList contentsList = detailsElem.elementsByTagName(QString::fromLatin1("media:content"));

                        for (int i = 0; i < contentsList.size(); ++i)
                        {
                            QDomElement contentElem = contentsList.at(i).toElement();

                            if (!contentElem.isNull())
                            {
                                if ((contentElem.attribute(QString::fromLatin1("medium")) == QString::fromLatin1("image")) &&
                                    fps.originalURL.isEmpty())
                                {
                                    fps.originalURL = QUrl(contentElem.attribute(QString::fromLatin1("url")));
                                    fps.title       = fps.originalURL.fileName();
                                    fps.mimeType    = contentElem.attribute(QString::fromLatin1("type"));
                                }

                                if ((contentElem.attribute(QString::fromLatin1("medium")) == QString::fromLatin1("video")) &&
                                    (contentElem.attribute(QString::fromLatin1("type")) == QString::fromLatin1("video/mpeg4")))
                                {
                                    fps.originalURL = QUrl(contentElem.attribute(QString::fromLatin1("url")));
                                    fps.title       = fps.originalURL.fileName();
                                    fps.mimeType    = contentElem.attribute(QString::fromLatin1("type"));
                                }
                            }
                        }
                    }
                }

                detailsNode = detailsNode.nextSibling();
            }

            photoList.append(fps);
        }

        node = node.nextSibling();
    }

    emit signalListPhotosDone(1, QString::fromLatin1(""), photoList);
}

void GPTalker::parseResponseCreateAlbum(const QByteArray& data)
{
    bool success = false;

    QDomDocument doc(QString::fromLatin1("AddPhoto Response"));

    if ( !doc.setContent( data ) )
    {
        signalCreateAlbumDone(0, i18n("Failed to create album"), QString::fromLatin1("-1"));
        return;
    }

    // parse the new album name
    QDomElement docElem = doc.documentElement();
    QString albumId(QString::fromLatin1(""));

    if (docElem.nodeName() == QString::fromLatin1("entry"))
    {
        success       = true;
        QDomNode node = docElem.firstChild(); //this should mean <entry>

        while ( !node.isNull() )
        {
            if ( node.isElement())
            {
                if (node.nodeName() == QString::fromLatin1("gphoto:id"))
                {
                    albumId = node.toElement().text();
                }
             }

             node = node.nextSibling();
        }
    }

    if (success == true)
    {
        signalCreateAlbumDone(1, QString::fromLatin1(""), albumId);
    }
    else
    {
        signalCreateAlbumDone(0, i18n("Failed to create album"), QString::fromLatin1("-1"));
    }
}

void GPTalker::parseResponseAddPhoto(const QByteArray& data)
{
    QDomDocument doc(QString::fromLatin1("AddPhoto Response"));

    if ( !doc.setContent( data ) )
    {
        emit signalAddPhotoDone(0, i18n("Failed to upload photo"), QString::fromLatin1("-1"));
        return;
    }

    // parse the new album name
    QDomElement docElem = doc.documentElement();
    QString photoId(QString::fromLatin1(""));

    if (docElem.nodeName() == QString::fromLatin1("entry"))
    {
        QDomNode node = docElem.firstChild(); //this should mean <entry>

        while (!node.isNull())
        {
            if (node.isElement())
            {
                if (node.nodeName() == QString::fromLatin1("gphoto:id"))
                {
                    photoId = node.toElement().text();
                }
            }

            node = node.nextSibling();
        }
    }

    emit signalAddPhotoDone(1, QString::fromLatin1(""), photoId);
}

} // namespace Digikam

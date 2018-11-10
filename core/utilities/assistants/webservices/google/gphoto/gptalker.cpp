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
 * Copyright (C) 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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
#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QStringList>
#include <QUrl>
#include <QtAlgorithms>
#include <QApplication>
#include <QDir>
#include <QMessageBox>

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

#define NB_MAX_ITEM_UPLOAD 50

namespace Digikam
{

static bool gphotoLessThan(const GSFolder& p1, const GSFolder& p2)
{
    return (p1.title.toLower() < p2.title.toLower());
}

class Q_DECL_HIDDEN GPTalker::Private
{
public:

    enum State
    {
        GP_LOGOUT     = -1,
        GP_LISTALBUMS = 0,
        GP_GETUSER,
        GP_LISTPHOTOS,
        GP_ADDPHOTO,
        GP_UPDATEPHOTO,
        GP_UPLOADPHOTO,
        GP_GETPHOTO,
        GP_CREATEALBUM
    };

public:

    explicit Private()
    {
        state           = GP_LOGOUT;
        netMngr         = 0;

        userInfoUrl     = QLatin1String("https://www.googleapis.com/plus/v1/people/me");

        apiVersion      = QLatin1String("v1");
        apiUrl          = QString::fromLatin1("https://photoslibrary.googleapis.com/%1/%2").arg(apiVersion);

        albumIdToUpload = QLatin1String("-1");
        previousImageId = QLatin1String("-1");
    }

public:

    QString                userInfoUrl;

    QString                apiUrl;
    QString                apiVersion;

    State                  state;

    QString                descriptionToUpload;
    QString                albumIdToUpload;
    QString                previousImageId;
    QStringList            uploadTokenList;

    QNetworkAccessManager* netMngr;
};

GPTalker::GPTalker(QWidget* const parent)
    : GSTalkerBase(parent,
                   QStringList() // to get user login (temporary until gphoto supports it officially)
                                 << QLatin1String("https://www.googleapis.com/auth/plus.login")
                                 // to add and download photo in the library
                                 << QLatin1String("https://www.googleapis.com/auth/photoslibrary")
                                 // to download photo created by digiKam on GPhoto
                                 << QLatin1String("https://www.googleapis.com/auth/photoslibrary.readonly.appcreateddata")
                                 // for shared albums
                                 << QLatin1String("https://www.googleapis.com/auth/photoslibrary.sharing"),
                   QLatin1String("GooglePhotos")),
      d(new Private)
{
    m_reply    = 0;
    d->netMngr = new QNetworkAccessManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    connect(this, SIGNAL(signalError(QString)),
            this, SLOT(slotError(QString)));

    connect(this, SIGNAL(signalReadyToUpload()),
            this, SLOT(slotUploadPhoto()));
}

GPTalker::~GPTalker()
{
    if (m_reply)
    {
        m_reply->abort();
    }

    WSToolUtils::removeTemporaryDir("google");

    delete d;
}

QStringList GPTalker::getUploadTokenList()
{
    return d->uploadTokenList;
}

/**
 * (Trung): Comments below are not valid anymore with google photos api
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

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "list albums";

    QUrl url(d->apiUrl.arg("albums"));

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url for list albums " << url;

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1());

    m_reply = d->netMngr->get(netRequest);

    d->state = Private::GP_LISTALBUMS;
    m_buffer.resize(0);
    emit signalBusy(true);
}

/**
 * We get user profile from Google Plus API
 * This is a temporary solution until Google Photo support API for user profile
 */
void GPTalker::getLoggedInUser()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "getLoggedInUser";

    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    QUrl url(d->userInfoUrl);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url for list albums " << url;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "m_accessToken " << m_accessToken;

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1());

    m_reply = d->netMngr->get(netRequest);

    d->state = Private::GP_GETUSER;
    m_buffer.resize(0);

    emit signalBusy(true);
}

void GPTalker::listPhotos(const QString& albumId, const QString& /*imgmax*/)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    QUrl url(d->apiUrl.arg("mediaItems:search"));

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toUtf8());

    QByteArray data;
    data += "{\"pageSize\": \"100\",";
    data += "\"albumId\":\"";
    data += albumId.toUtf8();
    data += "\"}";
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "data to list photos : " << QString(data);

    m_reply = d->netMngr->post(netRequest, data);

    d->state = Private::GP_LISTPHOTOS;
    m_buffer.resize(0);
    emit signalBusy(true);
}

void GPTalker::createAlbum(const GSFolder& album)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    // Create body in json
    QByteArray data;
    data += "{\"album\":";
    data += "{\"title\":\"";
    data += album.title.toLatin1();
    data += "\"}}";
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << QString(data);

    QUrl url(d->apiUrl.arg("albums"));

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1());

    m_reply = d->netMngr->post(netRequest, data);

    d->state = Private::GP_CREATEALBUM;
    m_buffer.resize(0);
    emit signalBusy(true);
}

/**
 * First a request is sent to the url below and then we will get an upload token
 * Upload token then will be sent with url in GPTlaker::uploadPhoto to create real photos on user account
 */
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

    QUrl url(d->apiUrl.arg("uploads"));

    // Save album ID and description to upload
    d->descriptionToUpload = info.description;
    d->albumIdToUpload     = albumId;

    QString path = photoPath;

    QMimeDatabase mimeDB;

    if (mimeDB.mimeTypeForFile(path).name().startsWith(QLatin1String("image/")))
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

        path = WSToolUtils::makeTemporaryDir("google").filePath(QFileInfo(photoPath)
                                             .baseName().trimmed() + QLatin1String(".jpg"));
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
            meta.setItemDimensions(image.size());
            meta.setItemOrientation(MetaEngine::ORIENTATION_NORMAL);
            meta.setMetadataWritingMode((int)DMetadata::WRITE_TO_FILE_ONLY);
            meta.save(path, true);
        }
    }

    // Create the body for temporary upload
    QFile imageFile(path);

    if (!imageFile.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QByteArray data = imageFile.readAll();
    imageFile.close();

    QString imageName = QUrl::fromLocalFile(path).fileName();

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/octet-stream"));
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1());
    netRequest.setRawHeader("X-Goog-Upload-File-Name", imageName.toLatin1());
    netRequest.setRawHeader("X-Goog-Upload-Protocol", "raw");

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << imageName;

    m_reply = d->netMngr->post(netRequest, data);

    d->state = Private::GP_ADDPHOTO;
    m_buffer.resize(0);
    emit signalBusy(true);
    return true;
}

bool GPTalker::updatePhoto(const QString& photoPath, GSPhoto& info, /*const QString& albumId,*/
                           bool rescale, int maxDim, int imageQuality)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);

    GPMPForm form;
    QString path = photoPath;

    QMimeDatabase mimeDB;

    if (mimeDB.mimeTypeForFile(path).name().startsWith(QLatin1String("image/")))
    {
        QImage image = PreviewLoadThread::loadHighQualitySynchronously(photoPath).copyQImage();

        if (image.isNull())
        {
            image.load(photoPath);
        }

        if (image.isNull())
        {
            emit signalBusy(false);
            return false;
        }

        path = WSToolUtils::makeTemporaryDir("google").filePath(QFileInfo(photoPath)
                                             .baseName().trimmed() + QLatin1String(".jpg"));
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
            meta.setItemDimensions(image.size());
            meta.setItemOrientation(MetaEngine::ORIENTATION_NORMAL);
            meta.setMetadataWritingMode((int)DMetadata::WRITE_TO_FILE_ONLY);
            meta.save(path, true);
        }
    }

    //Create the Body in atom-xml
    QDomDocument docMeta;
    QDomProcessingInstruction instr = docMeta.createProcessingInstruction(
        QLatin1String("xml"),
        QLatin1String("version='1.0' encoding='UTF-8'"));
    docMeta.appendChild(instr);
    QDomElement entryElem           = docMeta.createElement(QLatin1String("entry"));
    docMeta.appendChild(entryElem);
    entryElem.setAttribute(
        QLatin1String("xmlns"),
        QLatin1String("http://www.w3.org/2005/Atom"));
    QDomElement titleElem           = docMeta.createElement(QLatin1String("title"));
    entryElem.appendChild(titleElem);
    QDomText titleText              = docMeta.createTextNode(QFileInfo(path).fileName());
    titleElem.appendChild(titleText);
    QDomElement summaryElem         = docMeta.createElement(QLatin1String("summary"));
    entryElem.appendChild(summaryElem);
    QDomText summaryText            = docMeta.createTextNode(info.description);
    summaryElem.appendChild(summaryText);
    QDomElement categoryElem        = docMeta.createElement(QLatin1String("category"));
    entryElem.appendChild(categoryElem);
    categoryElem.setAttribute(
        QLatin1String("scheme"),
        QLatin1String("http://schemas.google.com/g/2005#kind"));
    categoryElem.setAttribute(
        QLatin1String("term"),
        QLatin1String("http://schemas.google.com/photos/2007#photo"));
    QDomElement mediaGroupElem      = docMeta.createElementNS(
        QLatin1String("http://search.yahoo.com/mrss/"),
        QLatin1String("media:group"));
    entryElem.appendChild(mediaGroupElem);
    QDomElement mediaKeywordsElem   = docMeta.createElementNS(
        QLatin1String("http://search.yahoo.com/mrss/"),
        QLatin1String("media:keywords"));
    mediaGroupElem.appendChild(mediaKeywordsElem);
    QDomText mediaKeywordsText      = docMeta.createTextNode(info.tags.join(QLatin1Char(',')));
    mediaKeywordsElem.appendChild(mediaKeywordsText);

    if (!info.gpsLat.isEmpty() && !info.gpsLon.isEmpty())
    {
        QDomElement whereElem = docMeta.createElementNS(
            QLatin1String("http://www.georss.org/georss"),
            QLatin1String("georss:where"));
        entryElem.appendChild(whereElem);
        QDomElement pointElem = docMeta.createElementNS(
            QLatin1String("http://www.opengis.net/gml"),
            QLatin1String("gml:Point"));
        whereElem.appendChild(pointElem);
        QDomElement gpsElem   = docMeta.createElementNS(
            QLatin1String("http://www.opengis.net/gml"),
            QLatin1String("gml:pos"));
        pointElem.appendChild(gpsElem);
        QDomText gpsVal       = docMeta.createTextNode(info.gpsLat + QLatin1Char(' ') + info.gpsLon);
        gpsElem.appendChild(gpsVal);
    }

    form.addPair(QLatin1String("descr"), docMeta.toString(), QLatin1String("application/atom+xml"));

    if (!form.addFile(QLatin1String("photo"), path))
    {
        emit signalBusy(false);
        return false;
    }

    form.finish();

    QNetworkRequest netRequest(info.editUrl);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1() + "\nIf-Match: *");

    m_reply = d->netMngr->put(netRequest, form.formData());

    d->state = Private::GP_UPDATEPHOTO;
    m_buffer.resize(0);

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
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "link to get photo " << url.url();

    m_reply = d->netMngr->get(QNetworkRequest(url));

    d->state = Private::GP_GETPHOTO;
    m_buffer.resize(0);
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

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "reply error : " << reply->error() << " - " << reply->errorString();

    if (reply->error() != QNetworkReply::NoError)
    {
        if (d->state == Private::GP_ADDPHOTO)
        {
            emit signalAddPhotoDone(reply->error(), reply->errorString());
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
        case (Private::GP_GETUSER):
            parseResponseGetLoggedInUser(m_buffer);
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
            emit signalAddPhotoDone(1, QLatin1String(""));
            break;
        case (Private::GP_UPLOADPHOTO):
            parseResponseUploadPhoto(m_buffer);
            break;
        case (Private::GP_GETPHOTO):

            qCDebug(DIGIKAM_WEBSERVICES_LOG) << QString(m_buffer);
            // all we get is data of the image
            emit signalGetPhotoDone(1, QString(), m_buffer);
            break;
    }

    reply->deleteLater();
}

void GPTalker::slotUploadPhoto()
{
    /* Keep track of number of items will be uploaded, because
     * Google Photo API upload maximum NB_MAX_ITEM_UPLOAD items in at a time
     */
    int nbItemsUpload = 0;

    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    QUrl url(d->apiUrl.arg("mediaItems:batchCreate"));

    QByteArray data;
    data += '{';

    if (d->albumIdToUpload != QLatin1String("-1"))
    {
        data += "\"albumId\": \"";
        data += d->albumIdToUpload.toLatin1();
        data += "\",";
    }

    data += "\"newMediaItems\": [";

    if (d->uploadTokenList.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "token list is empty";
    }

    while (!d->uploadTokenList.isEmpty() && nbItemsUpload < NB_MAX_ITEM_UPLOAD)
    {
        const QString& uploadToken = d->uploadTokenList.takeFirst();
        data += "{\"description\": \"";
        data += d->descriptionToUpload.toUtf8();
        data += "\",";
        data += "\"simpleMediaItem\": {";
        data += "\"uploadToken\": \"";
        data += uploadToken;
        data += "\"}}";

        if (d->uploadTokenList.length() > 0)
        {
            data += ',';
        }

        nbItemsUpload ++;
    }

    if (d->previousImageId == QLatin1String("-1"))
    {
        data += ']';
    }
    else
    {
        data += "],\"albumPosition\": {";
        data += "\"position\": \"AFTER_MEDIA_ITEM\",";
        data += "\"relativeMediaItemId\": \"";
        data += d->previousImageId.toLatin1();
        data += "\"}\r\n";
    }

    data += "}\r\n";
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << QString(data);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1());

    m_reply = d->netMngr->post(netRequest, data);

    d->state = Private::GP_UPLOADPHOTO;
    m_buffer.resize(0);
    emit signalBusy(true);
}

void GPTalker::parseResponseListAlbums(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseListAlbums";

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        emit signalListAlbumsDone(0, QString::fromLatin1("Code: %1 - %2").arg(err.error)
                                                                         .arg(err.errorString()),
                                  QList<GSFolder>());
        return;
    }

    QJsonObject jsonObject  = doc.object();
    QJsonArray jsonArray    = jsonObject[QLatin1String("albums")].toArray();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "json array " << doc;

    QList<GSFolder> albumList;

    /**
     * Google-photos allows user to post photos on their main page (not in any albums)
     * so this folder is created for that purpose
     */
    GSFolder mainPage;
    albumList.append(mainPage);

    foreach (const QJsonValue& value, jsonArray)
    {
        QJsonObject obj = value.toObject();

        GSFolder album;
        album.id            = obj[QLatin1String("id")].toString();
        album.title         = obj[QLatin1String("title")].toString();
        album.url           = obj[QLatin1String("productUrl")].toString();
        album.isWriteable   = obj[QLatin1String("isWriteable")].toBool();

        albumList.append(album);
    }

    std::sort(albumList.begin(), albumList.end(), gphotoLessThan);
    emit signalListAlbumsDone(1, QLatin1String(""), albumList);
}

void GPTalker::parseResponseListPhotos(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseListPhotos";

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        emit signalListPhotosDone(0, i18n("Failed to fetch photo-set list"), QList<GSPhoto>());
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "error code: " << err.error << ", msg: " << err.errorString();
        return;
    }

    QJsonObject jsonObject  = doc.object();
    QJsonArray jsonArray    = jsonObject[QLatin1String("mediaItems")].toArray();
    QList<GSPhoto> photoList;

    foreach (const QJsonValue& value, jsonArray)
    {
        QJsonObject obj = value.toObject();

        GSPhoto photo;

        photo.baseUrl       = obj[QLatin1String("baseUrl")].toString();
        photo.description   = obj[QLatin1String("description")].toString();
        photo.id            = obj[QLatin1String("id")].toString();
        photo.mimeType      = obj[QLatin1String("mimeType")].toString();
        photo.location      = obj[QLatin1String("Location")].toString(); // Not yet available in v1 but will be in the future

        QJsonObject metadata = obj[QLatin1String("mediaMetadata")].toObject();

        photo.creationTime  = metadata[QLatin1String("creationTime")].toString();
        photo.width         = metadata[QLatin1String("width")].toString();
        photo.height        = metadata[QLatin1String("height")].toString();

        photo.originalURL   = QUrl(photo.baseUrl + QString::fromLatin1("=w%1-h%2").arg(photo.width)
                                                                                  .arg(photo.height));
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << photo.originalURL.url();

        photoList.append(photo);
    }

    emit signalListPhotosDone(1, QLatin1String(""), photoList);
}

void GPTalker::parseResponseCreateAlbum(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseCreateAlbums";

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        emit signalCreateAlbumDone(0, QString::fromLatin1("Code: %1 - %2").arg(err.error)
                                                                          .arg(err.errorString()),
                                   QString());
        return;
    }

    QJsonObject jsonObject  = doc.object();
    QString albumId         = jsonObject[QLatin1String("id")].toString();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "album Id " << doc;

    emit signalCreateAlbumDone(1, QLatin1String(""), albumId);
}

void GPTalker::parseResponseAddPhoto(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseAddPhoto";
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "response " << QString(data);

    d->uploadTokenList << QString(data);
    emit signalAddPhotoDone(1, QLatin1String(""));
}

void GPTalker::parseResponseGetLoggedInUser(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseGetLoggedInUser";

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        return;
    }

    QJsonObject jsonObject = doc.object();
    QString userName       = jsonObject[QLatin1String("displayName")].toString();

    emit signalSetUserName(userName);

    listAlbums();
}

//TODO: Parse and return photoID
void GPTalker::parseResponseUploadPhoto(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseUploadPhoto";

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "doc " << doc;

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        emit signalUploadPhotoDone(0, err.errorString(), QStringList());
        return;
    }

    QJsonObject jsonObject  = doc.object();
    QJsonArray jsonArray    = jsonObject[QLatin1String("newMediaItemResults")].toArray();

    QStringList listPhotoId;

    foreach (const QJsonValue& value, jsonArray)
    {
        QJsonObject obj = value.toObject();

        QJsonObject mediaItem = obj[QLatin1String("mediaItem")].toObject();
        listPhotoId << mediaItem[QLatin1String("id")].toString();
    }

    d->previousImageId = listPhotoId.last();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "list photo Id " << listPhotoId.join(", ");

    emit signalBusy(false);
    emit signalUploadPhotoDone(1, QLatin1String(""), listPhotoId);
}

} // namespace Digikam

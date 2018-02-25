/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-07
 * Description : a tool to export images to Flickr web service
 *
 * Copyright (C) 2005-2009 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2017      by Maik Qualmann <metzpinguin at gmail dot com>
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

#include "flickrtalker.h"

// Qt includes

#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QMap>
#include <QStringList>
#include <QProgressDialog>
#include <QApplication>
#include <QDesktopServices>
#include <QMessageBox>

// Local includes

#include "dmetadata.h"
#include "wstoolutils.h"
#include "flickrmpform.h"
#include "flickritem.h"
#include "flickrwindow.h"
#include "digikam_debug.h"
#include "digikam_version.h"
#include "previewloadthread.h"

namespace Digikam
{

class FlickrTalker::Private
{
public:

    explicit Private()
    {
        parent          = 0;
        netMngr         = 0;
        reply           = 0;
        settings        = 0;
        state           = FE_LOGOUT;
        iface           = 0;
        o1              = 0;
        store           = 0;
        requestor       = 0;
    }

    QWidget*               parent;
    QByteArray             buffer;

    QString                serviceName;
    QString                apiUrl;
    QString                authUrl;
    QString                tokenUrl;
    QString                accessUrl;
    QString                uploadUrl;
    QString                apikey;
    QString                secret;
    QString                maxSize;
    QString                username;
    QString                userId;
    QString                lastTmpFile;

    QNetworkAccessManager* netMngr;
    QNetworkReply*         reply;

    QSettings*             settings;

    State                  state;

    DInfoInterface*        iface;

    O1*                    o1;
    O0SettingsStore*       store;
    O1Requestor*           requestor;
};

FlickrTalker::FlickrTalker(QWidget* const parent,
                           const QString& serviceName,
                           DInfoInterface* const iface)
    : d(new Private)
{
    d->parent         = parent;
    d->serviceName    = serviceName;
    d->iface          = iface;
    m_photoSetsList   = 0;
    m_authProgressDlg = 0;

    if (d->serviceName == QLatin1String("23"))
    {
        d->apiUrl    = QLatin1String("http://www.23hq.com/services/rest/");
        d->authUrl   = QLatin1String("http://www.23hq.com/services/auth/");
        d->uploadUrl = QLatin1String("http://www.23hq.com/services/upload/");

        // bshanks: do 23 and flickr really share API keys? or does 23 not need
        // one?
        d->apikey    = QLatin1String("49d585bafa0758cb5c58ab67198bf632");
        d->secret    = QLatin1String("34b39925e6273ffd");
    }
    else
    {
        d->apiUrl    = QLatin1String("https://www.flickr.com/services/rest/");
        d->authUrl   = QLatin1String("https://www.flickr.com/services/oauth/authorize?perms=write");
        d->tokenUrl  = QLatin1String("https://www.flickr.com/services/oauth/request_token");
        d->accessUrl = QLatin1String("https://www.flickr.com/services/oauth/access_token");
        d->uploadUrl = QLatin1String("https://up.flickr.com/services/upload/");

        d->apikey    = QLatin1String("49d585bafa0758cb5c58ab67198bf632");
        d->secret    = QLatin1String("34b39925e6273ffd");
    }

    d->netMngr = new QNetworkAccessManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    /* Initialize selected photo set as empty. */
    m_selectedPhotoSet = FPhotoSet();
    /* Initialize photo sets list. */
    m_photoSetsList    = new QLinkedList<FPhotoSet>();

    d->o1 = new O1(this);
    d->o1->setClientId(d->apikey);
    d->o1->setClientSecret(d->secret);
    d->o1->setAuthorizeUrl(QUrl(d->authUrl));
    d->o1->setAccessTokenUrl(QUrl(d->accessUrl));
    d->o1->setRequestTokenUrl(QUrl(d->tokenUrl));

    d->settings = WSToolUtils::getOauthSettings(this);
    d->store    = new O0SettingsStore(d->settings, QLatin1String(O2_ENCRYPTION_KEY), this);
    d->store->setGroupKey(d->serviceName);
    d->o1->setStore(d->store);

    connect(d->o1, SIGNAL(linkingFailed()),
            this, SLOT(slotLinkingFailed()));

    connect(d->o1, SIGNAL(linkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));

    connect(d->o1, SIGNAL(openBrowser(QUrl)),
            this, SLOT(slotOpenBrowser(QUrl)));

    d->requestor = new O1Requestor(d->netMngr, d->o1, this);
}

FlickrTalker::~FlickrTalker()
{
    if (d->reply)
    {
        d->reply->abort();
    }

    delete m_photoSetsList;

    WSToolUtils::removeTemporaryDir(d->serviceName.toLatin1().constData());

    delete d;
}

void FlickrTalker::link(const QString& userName)
{
    emit signalBusy(true);

    if (userName.isEmpty())
    {
        d->store->setGroupKey(d->serviceName);
    }
    else
    {
        d->store->setGroupKey(d->serviceName + userName);
    }

    d->o1->link();
}

void FlickrTalker::unLink()
{
    d->o1->unlink();
}

void FlickrTalker::removeUserName(const QString& userName)
{
    if (userName.startsWith(d->serviceName))
    {
        d->settings->beginGroup(userName);
        d->settings->remove(QString());
        d->settings->endGroup();
    }
}

void FlickrTalker::slotLinkingFailed()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Flickr fail";
    d->username = QString();
    emit signalBusy(false);
}

void FlickrTalker::slotLinkingSucceeded()
{
    if (!d->o1->linked())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UNLINK to Flickr ok";
        d->username = QString();
        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Flickr ok";

    d->username = d->o1->extraTokens()[QLatin1String("username")].toString();
    d->userId   = d->o1->extraTokens()[QLatin1String("user_nsid")].toString();

    if (d->store->groupKey() == d->serviceName)
    {
        d->settings->beginGroup(d->serviceName);
        QStringList keys = d->settings->allKeys();
        d->settings->endGroup();

        foreach(const QString& key, keys)
        {
            d->settings->beginGroup(d->serviceName);
            QVariant value = d->settings->value(key);
            d->settings->endGroup();
            d->settings->beginGroup(d->serviceName + d->username);
            d->settings->setValue(key, value);
            d->settings->endGroup();
        }

        d->store->setGroupKey(d->serviceName + d->username);
        removeUserName(d->serviceName);
    }

    emit signalLinkingSucceeded();
}

void FlickrTalker::slotOpenBrowser(const QUrl& url)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Open Browser...";
    QDesktopServices::openUrl(url);
}

QString FlickrTalker::getMaxAllowedFileSize()
{
    return d->maxSize;
}

void FlickrTalker::maxAllowedFileSize()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    if (!d->o1->linked())
        return;

    QUrl url(d->apiUrl);
    QNetworkRequest netRequest(url);
    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_XFORM));

    reqParams << O0RequestParameter("method", "flickr.people.getLimits");

    QByteArray postData = O1::createQueryParameters(reqParams);

    d->reply = d->requestor->post(netRequest, reqParams, postData);

    d->state = FE_GETMAXSIZE;
    m_authProgressDlg->setLabelText(i18n("Getting the maximum allowed file size."));
    m_authProgressDlg->setMaximum(4);
    m_authProgressDlg->setValue(1);
    d->buffer.resize(0);
    emit signalBusy(true);
}

void FlickrTalker::listPhotoSets()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    if (!d->o1->linked())
        return;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "List photoset invoked";

    QUrl url(d->apiUrl);
    QNetworkRequest netRequest(url);
    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_XFORM));

    reqParams << O0RequestParameter("method", "flickr.photosets.getList");

    QByteArray postData = O1::createQueryParameters(reqParams);

    d->reply = d->requestor->post(netRequest, reqParams, postData);

    d->state = FE_LISTPHOTOSETS;
    d->buffer.resize(0);
    emit signalBusy(true);
}

void FlickrTalker::getPhotoProperty(const QString& method, const QStringList& argList)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    if (!d->o1->linked())
        return;

    QUrl url(d->apiUrl);
    QNetworkRequest netRequest(url);
    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_XFORM));

    reqParams << O0RequestParameter("method", method.toLatin1());

    for (QStringList::const_iterator it = argList.constBegin(); it != argList.constEnd(); ++it)
    {
        QStringList str = (*it).split(QLatin1Char('='), QString::SkipEmptyParts);
        reqParams << O0RequestParameter(str[0].toLatin1(), str[1].toLatin1());
    }

    QByteArray postData = O1::createQueryParameters(reqParams);

    d->reply = d->requestor->post(netRequest, reqParams, postData);

    d->state = FE_GETPHOTOPROPERTY;
    d->buffer.resize(0);
    emit signalBusy(true);
}

void FlickrTalker::listPhotos(const QString& /*albumName*/)
{
    // TODO
}

void FlickrTalker::createPhotoSet(const QString& /*albumName*/, const QString& albumTitle,
                                  const QString& albumDescription, const QString& primaryPhotoId)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    if (!d->o1->linked())
        return;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Create photoset invoked";

    QUrl url(d->apiUrl);
    QNetworkRequest netRequest(url);
    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_XFORM));

    reqParams << O0RequestParameter("method", "flickr.photosets.create");
    reqParams << O0RequestParameter("title", albumTitle.toLatin1());
    reqParams << O0RequestParameter("description", albumDescription.toLatin1());
    reqParams << O0RequestParameter("primary_photo_id", primaryPhotoId.toLatin1());

    QByteArray postData = O1::createQueryParameters(reqParams);

    d->reply = d->requestor->post(netRequest, reqParams, postData);

    d->state = FE_CREATEPHOTOSET;
    d->buffer.resize(0);
    emit signalBusy(true);
}

void FlickrTalker::addPhotoToPhotoSet(const QString& photoId,
                                      const QString& photoSetId)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    if (!d->o1->linked())
        return;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "AddPhotoToPhotoSet invoked";

    /* If the photoset id starts with the special string "UNDEFINED_", it means
     * it doesn't exist yet on Flickr and needs to be created. Note that it's
     * not necessary to subsequently add the photo to the photo set, as this
     * is done in the set creation call to Flickr. */
    if (photoSetId.startsWith(QLatin1String("UNDEFINED_")))
    {
        createPhotoSet(QLatin1String(""), m_selectedPhotoSet.title, m_selectedPhotoSet.description, photoId);
    }
    else
    {
        QUrl url(d->apiUrl);
        QNetworkRequest netRequest(url);
        QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

        netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_XFORM));

        reqParams << O0RequestParameter("method", "flickr.photosets.addPhoto");
        reqParams << O0RequestParameter("photoset_id", photoSetId.toLatin1());
        reqParams << O0RequestParameter("photo_id", photoId.toLatin1());

        QByteArray postData = O1::createQueryParameters(reqParams);

        d->reply = d->requestor->post(netRequest, reqParams, postData);

        d->state = FE_ADDPHOTOTOPHOTOSET;
        d->buffer.resize(0);
        emit signalBusy(true);
    }
}

bool FlickrTalker::addPhoto(const QString& photoPath, const FPhotoInfo& info,
                            bool original, bool rescale, int maxDim, int imageQuality)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    if (!d->o1->linked())
        return false;

    QUrl url(d->uploadUrl);
    QNetworkRequest netRequest(url);
    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    QString path = photoPath;
    FlickrMPForm  form;

    QString ispublic = (info.is_public == 1) ? QLatin1String("1") : QLatin1String("0");
    form.addPair(QLatin1String("is_public"), ispublic, QLatin1String("text/plain"));
    reqParams << O0RequestParameter("is_public", ispublic.toLatin1());

    QString isfamily = (info.is_family == 1) ? QLatin1String("1") : QLatin1String("0");
    form.addPair(QLatin1String("is_family"), isfamily, QLatin1String("text/plain"));
    reqParams << O0RequestParameter("is_family", isfamily.toLatin1());

    QString isfriend = (info.is_friend == 1) ? QLatin1String("1") : QLatin1String("0");
    form.addPair(QLatin1String("is_friend"), isfriend, QLatin1String("text/plain"));
    reqParams << O0RequestParameter("is_friend", isfriend.toLatin1());

    QString safetyLevel = QString::number(static_cast<int>(info.safety_level));
    form.addPair(QLatin1String("safety_level"), safetyLevel, QLatin1String("text/plain"));
    reqParams << O0RequestParameter("safety_level", safetyLevel.toLatin1());

    QString contentType = QString::number(static_cast<int>(info.content_type));
    form.addPair(QLatin1String("content_type"), contentType, QLatin1String("text/plain"));
    reqParams << O0RequestParameter("content_type", contentType.toLatin1());

    QString tags = QLatin1String("\"") + info.tags.join(QLatin1String("\" \"")) + QLatin1String("\"");

    if (tags.length() > 0)
    {
        form.addPair(QLatin1String("tags"), tags, QLatin1String("text/plain"));
        reqParams << O0RequestParameter("tags", tags.toUtf8());
    }

    if (!info.title.isEmpty())
    {
        form.addPair(QLatin1String("title"), info.title, QLatin1String("text/plain"));
        reqParams << O0RequestParameter("title", info.title.toUtf8());
    }

    if (!info.description.isEmpty())
    {
        form.addPair(QLatin1String("description"), info.description, QLatin1String("text/plain"));
        reqParams << O0RequestParameter("description", info.description.toUtf8());
    }

    if (!original)
    {
        QImage image = PreviewLoadThread::loadHighQualitySynchronously(photoPath).copyQImage();

        if (image.isNull())
        {
            image.load(photoPath);
        }

        if (!image.isNull())
        {
            if (!d->lastTmpFile.isEmpty())
            {
                QFile::remove(d->lastTmpFile);
            }

            path = WSToolUtils::makeTemporaryDir(d->serviceName.toLatin1().constData()).filePath(QFileInfo(photoPath)
                                                                         .baseName().trimmed() + QLatin1String(".jpg"));

            if (rescale)
            {
                if (image.width() > maxDim || image.height() > maxDim)
                    image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }

            image.save(path, "JPEG", imageQuality);
            d->lastTmpFile = path;

            // Restore all metadata.

            DMetadata meta;

            if (meta.load(photoPath))
            {
                meta.setImageDimensions(image.size());
                meta.setImageOrientation(MetaEngine::ORIENTATION_NORMAL);

                // NOTE: see bug #153207: Flickr use IPTC keywords to create Tags in web interface
                //       As IPTC do not support UTF-8, we need to remove it.
                //       This function call remove all Application2 Tags.
                meta.removeIptcTags(QStringList() << QLatin1String("Application2"));

                // NOTE: see bug # 384260: Flickr use Xmp.dc.subject to create Tags
                //       in web interface, we need to remove it.
                //       This function call remove all Dublin Core Tags.
                meta.removeXmpTags(QStringList() << QLatin1String("dc"));

                meta.setImageProgramId(QLatin1String("digiKam"), digiKamVersion());
                meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
                meta.save(path);
            }
            else
            {
                qCWarning(DIGIKAM_WEBSERVICES_LOG) << "Flickr::Image doesn't have metadata";
            }

            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Resizing and saving to temp file: " << path;
        }
    }

    QFileInfo tempFileInfo(path);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "QUrl path is " << QUrl::fromLocalFile(path) << "Image size (in bytes) is "<< tempFileInfo.size();

    if (tempFileInfo.size() > (getMaxAllowedFileSize().toLongLong()))
    {
        emit signalAddPhotoFailed(i18n("File Size exceeds maximum allowed file size."));
        return false;
    }

    if (!form.addFile(QLatin1String("photo"), path))
    {
        return false;
    }

    form.finish();

    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());

    d->reply = d->requestor->post(netRequest, reqParams, form.formData());

    d->state = FE_ADDPHOTO;
    d->buffer.resize(0);
    emit signalBusy(true);
    return true;
}

QString FlickrTalker::getUserName() const
{
    return d->username;
}

QString FlickrTalker::getUserId() const
{
    return d->userId;
}

void FlickrTalker::cancel()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    if (m_authProgressDlg && !m_authProgressDlg->isHidden())
    {
        m_authProgressDlg->hide();
    }
}

void FlickrTalker::slotError(const QString& error)
{
    QString transError;
    int errorNo = error.toInt();

    switch (errorNo)
    {
        case 2:
            transError = i18n("No photo specified");
            break;

        case 3:
            transError = i18n("General upload failure");
            break;

        case 4:
            transError = i18n("Filesize was zero");
            break;

        case 5:
            transError = i18n("Filetype was not recognized");
            break;

        case 6:
            transError = i18n("User exceeded upload limit");
            break;

        case 96:
            transError = i18n("Invalid signature");
            break;

        case 97:
            transError = i18n("Missing signature");
            break;

        case 98:
            transError = i18n("Login Failed / Invalid auth token");
            break;

        case 100:
            transError = i18n("Invalid API Key");
            break;

        case 105:
            transError = i18n("Service currently unavailable");
            break;

        case 108:
            transError = i18n("Invalid Frob");
            break;

        case 111:
            transError = i18n("Format \"xxx\" not found");
            break;

        case 112:
            transError = i18n("Method \"xxx\" not found");
            break;

        case 114:
            transError = i18n("Invalid SOAP envelope");
            break;

        case 115:
            transError = i18n("Invalid XML-RPC Method Call");
            break;

        case 116:
            transError = i18n("The POST method is now required for all setters");
            break;

        default:
            transError = i18n("Unknown error");
            break;
    };

    QMessageBox::critical(QApplication::activeWindow(),
                          i18n("Error"),
                          i18n("Error Occurred: %1\nCannot proceed any further.", transError));
}

void FlickrTalker::slotFinished(QNetworkReply* reply)
{
    emit signalBusy(false);

    if (reply != d->reply)
    {
        return;
    }

    d->reply = 0;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (d->state == FE_ADDPHOTO)
        {
            emit signalAddPhotoFailed(reply->errorString());
        }
        else
        {
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18n("Error"), reply->errorString());
        }

        reply->deleteLater();
        return;
    }

    d->buffer.append(reply->readAll());

    switch (d->state)
    {
        case (FE_LOGIN):
            //parseResponseLogin(d->buffer);
            break;

        case (FE_LISTPHOTOSETS):
            parseResponseListPhotoSets(d->buffer);
            break;

        case (FE_LISTPHOTOS):
            parseResponseListPhotos(d->buffer);
            break;

        case (FE_GETPHOTOPROPERTY):
            parseResponsePhotoProperty(d->buffer);
            break;

        case (FE_ADDPHOTO):
            parseResponseAddPhoto(d->buffer);
            break;

        case (FE_ADDPHOTOTOPHOTOSET):
            parseResponseAddPhotoToPhotoSet(d->buffer);
            break;

        case (FE_CREATEPHOTOSET):
            parseResponseCreatePhotoSet(d->buffer);
            break;

        case (FE_GETMAXSIZE):
            parseResponseMaxSize(d->buffer);
            break;

        default:  // FR_LOGOUT
            break;
    }

    reply->deleteLater();
}

void FlickrTalker::parseResponseMaxSize(const QByteArray& data)
{
    QString errorString;
    QDomDocument doc(QLatin1String("mydocument"));

    if (!doc.setContent(data))
    {
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode node       = docElem.firstChild();

    QDomElement e;

    while (!node.isNull())
    {
        if (node.isElement() && node.nodeName() == QLatin1String("person"))
        {
            e                = node.toElement();
            QDomNode details = e.firstChild();

            while (!details.isNull())
            {
                if (details.isElement())
                {
                    e = details.toElement();

                    if (details.nodeName() == QLatin1String("photos"))
                    {
                        QDomAttr a = e.attributeNode(QLatin1String("maxupload"));
                        d->maxSize = a.value();
                        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Max upload size is"<<d->maxSize;
                    }
                }

                details = details.nextSibling();
            }
        }

        if (node.isElement() && node.nodeName() == QLatin1String("err"))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Checking Error in response";
            errorString = node.toElement().attribute(QLatin1String("code"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error code=" << errorString;
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Msg=" << node.toElement().attribute(QLatin1String("msg"));
        }

        node = node.nextSibling();
    }

    m_authProgressDlg->hide();
}

void FlickrTalker::parseResponseCreatePhotoSet(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Parse response create photoset received " << data;

    //bool success = false;

    QDomDocument doc(QLatin1String("getListPhotoSets"));

    if (!doc.setContent(data))
    {
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode    node    = docElem.firstChild();
    QDomElement e;

    while (!node.isNull())
    {
        if (node.isElement() && node.nodeName() == QLatin1String("photoset"))
        {
            // Parse the id from the response.
            QString new_id = node.toElement().attribute(QLatin1String("id"));

            // Set the new id in the photo sets list.
            QLinkedList<FPhotoSet>::iterator it = m_photoSetsList->begin();

            while (it != m_photoSetsList->end())
            {
                if (it->id == m_selectedPhotoSet.id)
                {
                    it->id = new_id;
                    break;
                }

                ++it;
            }

            // Set the new id in the selected photo set.
            m_selectedPhotoSet.id = new_id;

            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "PhotoSet created successfully with id" << new_id;
            emit signalAddPhotoSetSucceeded();
        }

        if (node.isElement() && node.nodeName() == QLatin1String("err"))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Checking Error in response";
            QString code = node.toElement().attribute(QLatin1String("code"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error code=" << code;
            QString msg = node.toElement().attribute(QLatin1String("msg"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Msg=" << msg;
            QMessageBox::critical(QApplication::activeWindow(), i18n("Error"), i18n("PhotoSet creation failed: ") + msg);
        }

        node = node.nextSibling();
    }
}

void FlickrTalker::parseResponseListPhotoSets(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseListPhotosets" << data;
    bool success = false;
    QDomDocument doc(QLatin1String("getListPhotoSets"));

    if (!doc.setContent(data))
    {
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode    node    = docElem.firstChild();
    QDomElement e;

    QString photoSet_id, photoSet_title, photoSet_description;
    m_photoSetsList->clear();

    while (!node.isNull())
    {
        if (node.isElement() && node.nodeName() == QLatin1String("photosets"))
        {
            e                    = node.toElement();
            QDomNode details     = e.firstChild();
            FPhotoSet fps;
            QDomNode detailsNode = details;

            while (!detailsNode.isNull())
            {
                if (detailsNode.isElement())
                {
                    e = detailsNode.toElement();

                    if (detailsNode.nodeName() == QLatin1String("photoset"))
                    {
                        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "id=" << e.attribute(QLatin1String("id"));
                        photoSet_id              = e.attribute(QLatin1String("id"));     // this is what is obtained from data.
                        fps.id                   = photoSet_id;
                        QDomNode photoSetDetails = detailsNode.firstChild();
                        QDomElement e_detail;

                        while (!photoSetDetails.isNull())
                        {
                            e_detail = photoSetDetails.toElement();

                            if (photoSetDetails.nodeName() == QLatin1String("title"))
                            {
                                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Title=" << e_detail.text();
                                photoSet_title = e_detail.text();
                                fps.title      = photoSet_title;
                            }
                            else if (photoSetDetails.nodeName() == QLatin1String("description"))
                            {
                                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Description =" << e_detail.text();
                                photoSet_description = e_detail.text();
                                fps.description      = photoSet_description;
                            }

                            photoSetDetails = photoSetDetails.nextSibling();
                        }

                        m_photoSetsList->append(fps);
                    }
                }

                detailsNode = detailsNode.nextSibling();
            }

            details = details.nextSibling();
            success = true;
        }

        if (node.isElement() && node.nodeName() == QLatin1String("err"))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Checking Error in response";
            QString code = node.toElement().attribute(QLatin1String("code"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error code=" << code;
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Msg=" << node.toElement().attribute(QLatin1String("msg"));
            emit signalError(code);
        }

        node = node.nextSibling();
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "GetPhotoList finished";

    if (!success)
    {
        emit signalListPhotoSetsFailed(i18n("Failed to fetch list of photo sets."));
    }
    else
    {
        emit signalListPhotoSetsSucceeded();
        maxAllowedFileSize();
    }
}

void FlickrTalker::parseResponseListPhotos(const QByteArray& data)
{
    QDomDocument doc(QLatin1String("getPhotosList"));

    if (!doc.setContent(data))
    {
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode node       = docElem.firstChild();
    //QDomElement e;
    //TODO
}

void FlickrTalker::parseResponseCreateAlbum(const QByteArray& data)
{
    QDomDocument doc(QLatin1String("getCreateAlbum"));

    if (!doc.setContent(data))
    {
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode node       = docElem.firstChild();

    //TODO
}

void FlickrTalker::parseResponseAddPhoto(const QByteArray& data)
{
    bool    success = false;
    QString line;
    QDomDocument doc(QLatin1String("AddPhoto Response"));

    if (!doc.setContent(data))
    {
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode node       = docElem.firstChild();
    QDomElement e;
    QString photoId;

    while (!node.isNull())
    {
        if (node.isElement() && node.nodeName() == QLatin1String("photoid"))
        {
            e                = node.toElement();           // try to convert the node to an element.
            QDomNode details = e.firstChild();
            photoId          = e.text();
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Photoid= " << photoId;
            success          = true;
        }

        if (node.isElement() && node.nodeName() == QLatin1String("err"))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Checking Error in response";
            QString code = node.toElement().attribute(QLatin1String("code"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error code=" << code;
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Msg=" << node.toElement().attribute(QLatin1String("msg"));
            emit signalError(code);
        }

        node = node.nextSibling();
    }

    if (!success)
    {
        emit signalAddPhotoFailed(i18n("Failed to upload photo"));
    }
    else
    {
        QString photoSetId = m_selectedPhotoSet.id;

        if (photoSetId == QLatin1String("-1"))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "PhotoSet Id not set, not adding the photo to any photoset";
            emit signalAddPhotoSucceeded();
        }
        else
        {
            addPhotoToPhotoSet(photoId, photoSetId);
        }
    }
}

void FlickrTalker::parseResponsePhotoProperty(const QByteArray& data)
{
    bool         success = false;
    QString      line;
    QDomDocument doc(QLatin1String("Photos Properties"));

    if (!doc.setContent(data))
    {
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode    node    = docElem.firstChild();
    QDomElement e;

    while (!node.isNull())
    {
        if (node.isElement() && node.nodeName() == QLatin1String("photoid"))
        {
            e                = node.toElement();                 // try to convert the node to an element.
            QDomNode details = e.firstChild();
            success          = true;
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Photoid=" << e.text();
        }

        if (node.isElement() && node.nodeName() == QLatin1String("err"))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Checking Error in response";
            QString code = node.toElement().attribute(QLatin1String("code"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error code=" << code;
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Msg=" << node.toElement().attribute(QLatin1String("msg"));
            emit signalError(code);
        }

        node = node.nextSibling();
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "GetToken finished";

    if (!success)
    {
        emit signalAddPhotoFailed(i18n("Failed to query photo information"));
    }
    else
    {
        emit signalAddPhotoSucceeded();
    }
}

void FlickrTalker::parseResponseAddPhotoToPhotoSet(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseListPhotosets" << data;
    emit signalAddPhotoSucceeded();
}

} // namespace Digikam

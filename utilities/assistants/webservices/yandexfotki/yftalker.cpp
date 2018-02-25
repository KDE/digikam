/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-14
 * Description : a tool to export items to YandexFotki web service
 *
 * Copyright (C) 2010      by Roman Tsisyk <roman at tsisyk dot com>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "yftalker.h"

// Qt includes

#include <QTextDocument>
#include <QByteArray>
#include <QDomDocument>
#include <QDomNode>
#include <QPointer>
#include <QFile>
#include <QFileInfo>
#include <QPointer>
#include <QNetworkReply>
#include <QNetworkAccessManager>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "yfauth.h"
#include "yfalbum.h"

namespace Digikam
{

class YFTalker::Private
{
public:

    explicit Private()
    {
        state     = STATE_UNAUTHENTICATED;
        lastPhoto = 0;
        netMngr   = 0;
        reply     = 0;
    }

    // API-related fields
    QString                 sessionKey;
    QString                 sessionId;
    QString                 token;
    QString                 login;
    QString                 password;
    QString                 apiAlbumsUrl;
    QString                 apiPhotosUrl;
    QString                 apiTagsUrl;

    // FSM data
    State                   state;
    // temporary data
    YFPhoto*                lastPhoto;
    QString                 lastPhotosUrl;

    // for albums pagination
    //in listAlbums()
    QList<YandexFotkiAlbum> albums;

    QString                 albumsNextUrl;

    QList<YFPhoto>          photos;
    QString                 photosNextUrl;

    QNetworkAccessManager*  netMngr;

    QNetworkReply*          reply;

    // Data buffer
    QByteArray              buffer;

    // constants
    // use QString insted of QUrl, we need .arg
    static const QString    SESSION_URL;
    static const QString    TOKEN_URL;
    static const QString    SERVICE_URL;
    static const QString    AUTH_REALM;
    static const QString    ACCESS_STRINGS[];
};

/*
 * static API constants
 */
const QString YFTalker::Private::SESSION_URL          = QString::fromLatin1("http://auth.mobile.yandex.ru/yamrsa/key/");
const QString YFTalker::Private::AUTH_REALM           = QString::fromLatin1("fotki.yandex.ru");
const QString YFTalker::Private::TOKEN_URL            = QString::fromLatin1("http://auth.mobile.yandex.ru/yamrsa/token/");
const QString YFTalker::Private::SERVICE_URL          = QString::fromLatin1("http://api-fotki.yandex.ru/api/users/%1/");

const QString YFTalker::Private::ACCESS_STRINGS[]     =
{
    QString::fromLatin1("public"),
    QString::fromLatin1("friends"),
    QString::fromLatin1("private")
};

const QString YFTalker::USERPAGE_URL         = QString::fromLatin1("http://fotki.yandex.ru/users/%1/");
const QString YFTalker::USERPAGE_DEFAULT_URL = QString::fromLatin1("http://fotki.yandex.ru/");

// ------------------------------------------------------------

YFTalker::YFTalker(QObject* const parent)
    : QObject(parent),
      d(new Private)
{
    d->netMngr = new QNetworkAccessManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));
}

YFTalker::~YFTalker()
{
    reset();
    delete d;
}

YFTalker::State YFTalker::state() const
{
    return d->state;
}

const QString& YFTalker::sessionKey() const
{
    return d->sessionKey;
}

const QString& YFTalker::sessionId() const
{
    return d->sessionId;
}

const QString& YFTalker::token() const
{
    return d->token;
}

const QString& YFTalker::login() const
{
    return d->login;
}

void YFTalker::setLogin(const QString& login)
{
    d->login = login;
}

const QString& YFTalker::password() const
{
    return d->password;
}

void YFTalker::setPassword(const QString& password)
{
    d->password = password;
}

bool YFTalker::isAuthenticated() const
{
    return (d->state & STATE_AUTHENTICATED) != 0;
}

bool YFTalker::isErrorState() const
{
    return (d->state & STATE_ERROR) != 0;
}

const QList<YandexFotkiAlbum>& YFTalker::albums() const
{
    return d->albums;
}

const QList<YFPhoto>& YFTalker::photos() const
{
    return d->photos;
}

void YFTalker::getService()
{
    d->state = STATE_GETSERVICE;

    QUrl url(d->SERVICE_URL.arg(d->login));

    d->reply = d->netMngr->get(QNetworkRequest(url));

    d->buffer.resize(0);
}

/*
void YFTalker::checkToken()
{
    // try to get somthing with our token, if it is invalid catch 401
    d->state = STATE_CHECKTOKEN;

    QUrl url(d->apiAlbumsUrl);
    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Authorization", QString::fromLatin1("FimpToken realm=\"%1\", token=\"%2\"")
                                             .arg(AUTH_REALM).arg(d->token).toLatin1());

    d->reply = d->netMngr->get(netRequest);

    // Error:    STATE_CHECKTOKEN_INVALID
    // Function: slotParseResponseCheckToken()

    d->buffer.resize(0);
}
*/

void YFTalker::getSession()
{
    if (d->state != STATE_GETSERVICE_DONE)
        return;

    d->state = STATE_GETSESSION;

    QUrl url(d->SESSION_URL);

    d->reply = d->netMngr->get(QNetworkRequest(url));

    d->buffer.resize(0);
}

void YFTalker::getToken()
{
    if (d->state != STATE_GETSESSION_DONE)
        return;

    const QString credentials = YFAuth::makeCredentials(d->sessionKey,
                                d->login, d->password);

    // prepare params
    QStringList paramList;

    paramList.append(QLatin1String("request_id=") + d->sessionId);

    paramList.append(QLatin1String("credentials=") + QString::fromUtf8(QUrl::toPercentEncoding(credentials)));

    QString params = paramList.join(QString::fromLatin1("&"));

    d->state = STATE_GETTOKEN;

    QUrl url(d->TOKEN_URL);
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    d->reply = d->netMngr->post(netRequest, params.toUtf8());

    d->buffer.resize(0);
}

void YFTalker::listAlbums()
{
    if (isErrorState() || !isAuthenticated())
        return;

    d->albumsNextUrl = d->apiAlbumsUrl;
    d->albums.clear();
    listAlbumsNext();
}

void YFTalker::listAlbumsNext()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "listAlbumsNext";

    d->state = STATE_LISTALBUMS;

    QUrl url(d->albumsNextUrl);
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/atom+xml; charset=utf-8; type=feed"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("FimpToken realm=\"%1\", token=\"%2\"")
                                             .arg(d->AUTH_REALM).arg(d->token).toLatin1());

    d->reply = d->netMngr->get(netRequest);

    d->buffer.resize(0);
}

void YFTalker::listPhotos(const YandexFotkiAlbum& album)
{
    if (isErrorState() || !isAuthenticated())
        return;

    d->photosNextUrl = album.m_apiPhotosUrl;
    d->photos.clear();
    listPhotosNext();
}

// protected member
void YFTalker::listPhotosNext()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "listPhotosNext";

    d->state = STATE_LISTPHOTOS;

    QUrl url(d->photosNextUrl);
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/atom+xml; charset=utf-8; type=feed"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("FimpToken realm=\"%1\", token=\"%2\"")
                                             .arg(d->AUTH_REALM).arg(d->token).toLatin1());

    d->reply = d->netMngr->get(netRequest);

    d->buffer.resize(0);
}

void YFTalker::updatePhoto(YFPhoto& photo, const YandexFotkiAlbum& album)
{
    if (isErrorState() || !isAuthenticated())
        return;

    // sanity check
    if (photo.title().isEmpty())
    {
        photo.setTitle(QFileInfo(photo.localUrl()).baseName().trimmed());
    }

    // move photo to another album (if changed)
    photo.m_apiAlbumUrl = album.m_apiSelfUrl;
    // FIXME: hack
    d->lastPhotosUrl     = album.m_apiPhotosUrl;

    if (!photo.remoteUrl().isNull())
    {
        // TODO: updating image file haven't yet supported by API
        // so, just update info
        return updatePhotoInfo(photo);
    }
    else
    {
        // for new images also upload file
        updatePhotoFile(photo);
    }
}

void YFTalker::updatePhotoFile(YFPhoto& photo)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "updatePhotoFile" << photo;

    QFile imageFile(photo.localUrl());

    if (!imageFile.open(QIODevice::ReadOnly))
    {
        setErrorState(STATE_UPDATEPHOTO_FILE_ERROR);
        return;
    }

    d->state     = STATE_UPDATEPHOTO_FILE;
    d->lastPhoto = &photo;

    QUrl url(d->lastPhotosUrl);
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("image/jpeg"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("FimpToken realm=\"%1\", token=\"%2\"")
                                             .arg(d->AUTH_REALM).arg(d->token).toLatin1());
    netRequest.setRawHeader("Slug", QUrl::toPercentEncoding(photo.title()) + ".jpg");

    d->reply = d->netMngr->post(netRequest, imageFile.readAll());

    d->buffer.resize(0);
}

void YFTalker::updatePhotoInfo(YFPhoto& photo)
{
    QDomDocument doc;
    QDomProcessingInstruction instr = doc.createProcessingInstruction(
        QString::fromLatin1("xml"),
        QString::fromLatin1("version='1.0' encoding='UTF-8'"));

    doc.appendChild(instr);
    QDomElement entryElem = doc.createElement(QString::fromLatin1("entry"));
    entryElem.setAttribute(QString::fromLatin1("xmlns"), QString::fromLatin1("http://www.w3.org/2005/Atom"));
    entryElem.setAttribute(QString::fromLatin1("xmlns:f"), QString::fromLatin1("yandex:fotki"));
    doc.appendChild(entryElem);

    QDomElement urn = doc.createElement(QString::fromLatin1("urn"));
    urn.appendChild(doc.createTextNode(photo.urn()));
    entryElem.appendChild(urn);

    QDomElement title = doc.createElement(QString::fromLatin1("title"));
    title.appendChild(doc.createTextNode(photo.title()));
    entryElem.appendChild(title);

    QDomElement linkAlbum = doc.createElement(QString::fromLatin1("link"));
    linkAlbum.setAttribute(QString::fromLatin1("href"), photo.m_apiAlbumUrl);
    linkAlbum.setAttribute(QString::fromLatin1("rel"), QString::fromLatin1("album"));
    entryElem.appendChild(linkAlbum);

    QDomElement summary   = doc.createElement(QString::fromLatin1("summary"));
    summary.appendChild(doc.createTextNode(photo.summary()));
    entryElem.appendChild(summary);

    QDomElement adult     = doc.createElement(QString::fromLatin1("f:xxx"));
    adult.setAttribute(QString::fromLatin1("value"), photo.isAdult() ? QString::fromLatin1("true") : QString::fromLatin1("false"));
    entryElem.appendChild(adult);

    QDomElement hideOriginal = doc.createElement(QString::fromLatin1("f:hide_original"));
    hideOriginal.setAttribute(QString::fromLatin1("value"),
                              photo.isHideOriginal() ? QString::fromLatin1("true") : QString::fromLatin1("false"));
    entryElem.appendChild(hideOriginal);

    QDomElement disableComments = doc.createElement(QString::fromLatin1("f:disable_comments"));
    disableComments.setAttribute(QString::fromLatin1("value"),
                                 photo.isDisableComments() ? QString::fromLatin1("true") : QString::fromLatin1("false"));
    entryElem.appendChild(disableComments);

    QDomElement access = doc.createElement(QString::fromLatin1("f:access"));
    access.setAttribute(QString::fromLatin1("value"), d->ACCESS_STRINGS[photo.access()]);
    entryElem.appendChild(access);

    // FIXME: undocumented API
    foreach(const QString& t, photo.tags)
    {
        QDomElement tag = doc.createElement(QString::fromLatin1("category"));
        tag.setAttribute(QString::fromLatin1("scheme"), d->apiTagsUrl);
        tag.setAttribute(QString::fromLatin1("term"), t);
        entryElem.appendChild(tag);
    }

    QByteArray buffer = doc.toString(1).toUtf8(); // with idents

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Prepared data: " << buffer;

    d->lastPhoto = &photo;
    d->state     = STATE_UPDATEPHOTO_INFO;

    QUrl url(photo.m_apiEditUrl);
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                         QLatin1String("application/atom+xml; charset=utf-8; type=entry"));
    netRequest.setRawHeader("Authorization",
                            QString::fromLatin1("FimpToken realm=\"%1\", token=\"%2\"")
                                                .arg(d->AUTH_REALM).arg(d->token).toLatin1());

    d->reply = d->netMngr->put(netRequest, buffer);

    d->buffer.resize(0);
}

void YFTalker::updateAlbum(YandexFotkiAlbum& album)
{
    if (isErrorState() || !isAuthenticated())
        return;

    if (album.urn().isEmpty())
    {
        // new album
        return updateAlbumCreate(album);
    }
    else
    {
        qCCritical(DIGIKAM_WEBSERVICES_LOG) << "Updating albums is not yet supported";
    }
}

void YFTalker::updateAlbumCreate(YandexFotkiAlbum& album)
{
    QDomDocument doc;
    QDomProcessingInstruction instr = doc.createProcessingInstruction(
        QString::fromLatin1("xml"),
        QString::fromLatin1("version='1.0' encoding='UTF-8'"));

    doc.appendChild(instr);
    QDomElement entryElem = doc.createElement(QString::fromLatin1("entry"));
    entryElem.setAttribute(QString::fromLatin1("xmlns"), QString::fromLatin1("http://www.w3.org/2005/Atom"));
    entryElem.setAttribute(QString::fromLatin1("xmlns:f"), QString::fromLatin1("yandex:fotki"));
    doc.appendChild(entryElem);

    QDomElement title = doc.createElement(QString::fromLatin1("title"));
    title.appendChild(doc.createTextNode(album.title()));
    entryElem.appendChild(title);

    QDomElement summary = doc.createElement(QString::fromLatin1("summary"));
    summary.appendChild(doc.createTextNode(album.summary()));
    entryElem.appendChild(summary);

    QDomElement password = doc.createElement(QString::fromLatin1("f:password"));
    password.appendChild(doc.createTextNode(album.m_password));
    entryElem.appendChild(password);

    const QByteArray postData = doc.toString(1).toUtf8(); // with idents
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Prepared data: " << postData;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Url"             << d->apiAlbumsUrl;

    d->state = STATE_UPDATEALBUM;

    QUrl url(d->apiAlbumsUrl);
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/atom+xml; charset=utf-8; type=entry"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("FimpToken realm=\"%1\", token=\"%2\"")
                                             .arg(d->AUTH_REALM).arg(d->token).toLatin1());

    d->reply = d->netMngr->post(netRequest, postData);

    d->buffer.resize(0);
}

void YFTalker::reset()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    d->token.clear();
    d->state = STATE_UNAUTHENTICATED;
}

void YFTalker::cancel()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    if (isAuthenticated())
    {
        d->state = STATE_AUTHENTICATED;
    }
    else
    {
        d->token.clear();
        d->state = STATE_UNAUTHENTICATED;
    }
}

void YFTalker::setErrorState(State state)
{
    d->state = state;
    emit signalError();
}

void YFTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != d->reply)
    {
        return;
    }

    d->reply = 0;

    if (reply->error() != QNetworkReply::NoError)
    {
        int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Transfer Error" << code << reply->errorString();

        if (code == 401 || code == 403 || code == 404) // auth required, 404 user not found
        {
            setErrorState(STATE_INVALID_CREDENTIALS);
        }
        else if (d->state == STATE_GETSERVICE)
        {
            setErrorState(STATE_GETSERVICE_ERROR);
        }
        else if (d->state == STATE_GETSESSION)
        {
            setErrorState(STATE_GETSESSION_ERROR);
        }
        else if (d->state == STATE_GETTOKEN)
        {
            setErrorState(STATE_GETTOKEN_ERROR);
        }
        else if (d->state == STATE_LISTALBUMS)
        {
            setErrorState(STATE_LISTALBUMS_ERROR);
        }
        else if (d->state == STATE_LISTPHOTOS)
        {
            setErrorState(STATE_LISTPHOTOS_ERROR);
        }
        else if (d->state == STATE_UPDATEPHOTO_FILE)
        {
            setErrorState(STATE_UPDATEPHOTO_FILE_ERROR);
        }
        else if (d->state == STATE_UPDATEPHOTO_INFO)
        {
            setErrorState(STATE_UPDATEPHOTO_INFO_ERROR);
        }
        else if (d->state == STATE_UPDATEALBUM)
        {
            setErrorState(STATE_UPDATEALBUM_ERROR);
        }

        reply->deleteLater();
        return;
    }

    d->buffer.append(reply->readAll());

    switch(d->state)
    {
        case (STATE_GETSERVICE):
            slotParseResponseGetService();
            break;
        case (STATE_GETSESSION):
            slotParseResponseGetSession();
            break;
        case (STATE_GETTOKEN):
            slotParseResponseGetToken();
            break;
        case (STATE_LISTALBUMS):
            slotParseResponseListAlbums();
            break;
        case (STATE_LISTPHOTOS):
            slotParseResponseListPhotos();
            break;
        case (STATE_UPDATEPHOTO_FILE):
            slotParseResponseUpdatePhotoFile();
            break;
        case (STATE_UPDATEPHOTO_INFO):
            slotParseResponseUpdatePhotoInfo();
            break;
        case (STATE_UPDATEALBUM):
            slotParseResponseUpdateAlbum();
            break;
        default:
            break;
    }

    reply->deleteLater();
}

void YFTalker::slotParseResponseGetService()
{
    QDomDocument doc(QString::fromLatin1("service"));

    if (!doc.setContent(d->buffer))
    {
        qCCritical(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML: parse error" << d->buffer;
        return setErrorState(STATE_GETSERVICE_ERROR);
    }

    const QDomElement rootElem = doc.documentElement();

    QDomElement workspaceElem = rootElem.firstChildElement(QString::fromLatin1("app:workspace"));

    // FIXME: workaround for Yandex xml namespaces bugs
    QString prefix = QString::fromLatin1("app:");

    if (workspaceElem.isNull())
    {
        workspaceElem = rootElem.firstChildElement(QString::fromLatin1("workspace"));
        prefix        = QString();
        qCCritical(DIGIKAM_WEBSERVICES_LOG) << "Service document without namespaces found";
    }

    if (workspaceElem.isNull())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML data: workspace element";
        return setErrorState(STATE_GETSERVICE_ERROR);
    }

    QString apiAlbumsUrl;
    QString apiPhotosUrl;
    QString apiTagsUrl;

    QDomElement collectionElem = workspaceElem.firstChildElement(prefix + QString::fromLatin1("collection"));

    for ( ; !collectionElem.isNull() ;
          collectionElem = collectionElem.nextSiblingElement(prefix + QString::fromLatin1("collection")))
    {
        const QDomElement acceptElem = collectionElem.firstChildElement(prefix + QString::fromLatin1("accept"));

        if (acceptElem.isNull()) // invalid section, ignore
        {
            continue;
        }

        // FIXME: id attribute is undocumented
        if (collectionElem.attribute(QString::fromLatin1("id")) == QString::fromLatin1("album-list"))
        {
            apiAlbumsUrl = collectionElem.attribute(QString::fromLatin1("href"));
        }
        else if (collectionElem.attribute(QString::fromLatin1("id")) == QString::fromLatin1("photo-list"))
        {
            apiPhotosUrl = collectionElem.attribute(QString::fromLatin1("href"));
        }
        else if (collectionElem.attribute(QString::fromLatin1("id")) == QString::fromLatin1("tag-list"))
        {
            apiTagsUrl = collectionElem.attribute(QString::fromLatin1("href"));
        }
        // else skip unknown section
    }

    if (apiAlbumsUrl.isNull() || apiPhotosUrl.isNull())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML data: service URLs";
        return setErrorState(STATE_GETSERVICE_ERROR);
    }

    d->apiAlbumsUrl = apiAlbumsUrl;
    d->apiPhotosUrl = apiPhotosUrl;
    d->apiTagsUrl   = apiTagsUrl;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "ServiceUrls:";
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Albums" << d->apiAlbumsUrl;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Photos" << d->apiPhotosUrl;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Tags"   << d->apiTagsUrl;

    d->state = STATE_GETSERVICE_DONE;
    emit signalGetServiceDone();
}

/*
void YFTalker::slotParseResponseCheckToken()
{
    // token still valid, skip getSession and getToken
    d->state = STATE_GETTOKEN_DONE;
    emit signalGetTokenDone();
}
*/

void YFTalker::slotParseResponseGetSession()
{
    QDomDocument doc(QString::fromLatin1("session"));

    if (!doc.setContent(d->buffer))
    {
        return setErrorState(STATE_GETSESSION_ERROR);
    }

    const QDomElement rootElem = doc.documentElement();

    const QDomElement keyElem =  rootElem.firstChildElement(QString::fromLatin1("key"));

    const QDomElement requestIdElem =  rootElem.firstChildElement(QString::fromLatin1("request_id"));

    if (keyElem.isNull() || keyElem.nodeType() != QDomNode::ElementNode ||
        requestIdElem.isNull() || requestIdElem.nodeType() != QDomNode::ElementNode)
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML" << d->buffer;
        return setErrorState(STATE_GETSESSION_ERROR);
    }

    d->sessionKey = keyElem.text();
    d->sessionId  = requestIdElem.text();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Session started" << d->sessionKey << d->sessionId;

    d->state      = STATE_GETSESSION_DONE;
    emit signalGetSessionDone();
}

void YFTalker::slotParseResponseGetToken()
{
    QDomDocument doc(QString::fromLatin1("response"));

    if (!doc.setContent(d->buffer))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML: parse error" << d->buffer;
        return setErrorState(STATE_GETTOKEN_ERROR);
    }

    const QDomElement rootElem  = doc.documentElement();
    const QDomElement tokenElem = rootElem.firstChildElement(QString::fromLatin1("token"));

    if (tokenElem.isNull() || tokenElem.nodeType() != QDomNode::ElementNode)
    {
        const QDomElement errorElem = rootElem.firstChildElement(QString::fromLatin1("error"));

        if (errorElem.isNull() || errorElem.nodeType() != QDomNode::ElementNode)
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Auth unknown error";
            return setErrorState(STATE_GETTOKEN_ERROR);
        }

/*
        // checked by HTTP error code in prepareJobResult
        const QString errorCode = errorElem.attribute("code", "0");
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << QString("Auth error: %1, code=%2").arg(errorElem.text()).arg(errorCode);

        if (errorCode == "2")
        {
            // Invalid credentials
            return setErrorState(STATE_GETTOKEN_INVALID_CREDENTIALS);
        }
*/

        return;
    }

    d->token = tokenElem.text();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Token got" << d->token;
    d->state = STATE_GETTOKEN_DONE;
    emit signalGetTokenDone();
}


void YFTalker::slotParseResponseListAlbums()
{
    QDomDocument doc(QString::fromLatin1("feed"));

    if (!doc.setContent(d->buffer))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML: parse error";
        return setErrorState(STATE_LISTALBUMS_ERROR);
    }

    bool errorOccurred         = false;
    const QDomElement rootElem = doc.documentElement();

    // find next page link
    d->albumsNextUrl.clear();
    QDomElement linkElem       = rootElem.firstChildElement(QString::fromLatin1("link"));

    for ( ; !linkElem.isNull() ;
          linkElem = linkElem.nextSiblingElement(QString::fromLatin1("link")))
    {
        if (linkElem.attribute(QString::fromLatin1("rel")) == QString::fromLatin1("next") &&
            !linkElem.attribute(QString::fromLatin1("href")).isNull())
        {
            d->albumsNextUrl = linkElem.attribute(QString::fromLatin1("href"));
            break;
        }
    }

    QDomElement entryElem = rootElem.firstChildElement(QString::fromLatin1("entry"));

    for ( ; !entryElem.isNull() ;
          entryElem = entryElem.nextSiblingElement(QString::fromLatin1("entry")))
    {
        const QDomElement urn       = entryElem.firstChildElement(QString::fromLatin1("id"));
        const QDomElement author    = entryElem.firstChildElement(QString::fromLatin1("author"));
        const QDomElement title     = entryElem.firstChildElement(QString::fromLatin1("title"));
        const QDomElement summary   = entryElem.firstChildElement(QString::fromLatin1("summary"));
        const QDomElement published = entryElem.firstChildElement(QString::fromLatin1("published"));
        const QDomElement edited    = entryElem.firstChildElement(QString::fromLatin1("app:edited"));
        const QDomElement updated   = entryElem.firstChildElement(QString::fromLatin1("updated"));
        const QDomElement prot      = entryElem.firstChildElement(QString::fromLatin1("protected"));

        QDomElement linkSelf;
        QDomElement linkEdit;
        QDomElement linkPhotos;

        QDomElement linkElem = entryElem.firstChildElement(QString::fromLatin1("link"));

        for ( ; !linkElem.isNull() ;
              linkElem = linkElem.nextSiblingElement(QString::fromLatin1("link")))
        {
            if (linkElem.attribute(QString::fromLatin1("rel")) == QString::fromLatin1("self"))
                linkSelf = linkElem;
            else if (linkElem.attribute(QString::fromLatin1("rel")) == QString::fromLatin1("edit"))
                linkEdit = linkElem;
            else if (linkElem.attribute(QString::fromLatin1("rel")) == QString::fromLatin1("photos"))
                linkPhotos = linkElem;
            // else skip <link>
        }

        if (urn.isNull() || title.isNull() ||
            linkSelf.isNull() || linkEdit.isNull() || linkPhotos.isNull())
        {
            errorOccurred = true;
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML data: invalid entry on line" << entryElem.lineNumber();
            // simple skip this record, no addtional messages to user
            continue;
        }

        QString password;

        if (!prot.isNull() && prot.attribute(QString::fromLatin1("value"), QString::fromLatin1("false")) == QString::fromLatin1("true"))
        {
            password = QString::fromLatin1(""); // set not null value
        }

        d->albums.append(YandexFotkiAlbum(
                            urn.text(),
                            author.text(),
                            title.text(),
                            summary.text(),
                            linkEdit.attribute(QString::fromLatin1("href")),
                            linkSelf.attribute(QString::fromLatin1("href")),
                            linkPhotos.attribute(QString::fromLatin1("href")),
                            QDateTime::fromString(published.text(), QString::fromLatin1("yyyy-MM-ddTHH:mm:ssZ")),
                            QDateTime::fromString(edited.text(), QString::fromLatin1("yyyy-MM-ddTHH:mm:ssZ")),
                            QDateTime::fromString(updated.text(), QString::fromLatin1("yyyy-MM-ddTHH:mm:ssZ")),
                            password
                        ));

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Found album:" << d->albums.last();
    }

    // TODO: pagination like listPhotos

    // if an error has occurred and we didn't find anything => notify user
    if (errorOccurred && d->albums.empty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "No result and errors have occurred";
        return setErrorState(STATE_LISTALBUMS_ERROR);
    }

    // we have next page
    if (!d->albumsNextUrl.isNull())
    {
        return listAlbumsNext();
    }
    else
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "List albums done: " << d->albums.size();
        d->state = STATE_LISTALBUMS_DONE;
        emit signalListAlbumsDone(d->albums);
    }
}

bool YFTalker::slotParsePhotoXml(const QDomElement& entryElem, YFPhoto& photo)
{

    const QDomElement urn             = entryElem.firstChildElement(QString::fromLatin1("id"));
    const QDomElement author          = entryElem.firstChildElement(QString::fromLatin1("author"));
    const QDomElement title           = entryElem.firstChildElement(QString::fromLatin1("title"));
    const QDomElement summary         = entryElem.firstChildElement(QString::fromLatin1("summary"));
    const QDomElement published       = entryElem.firstChildElement(QString::fromLatin1("published"));
    const QDomElement edited          = entryElem.firstChildElement(QString::fromLatin1("app:edited"));
    const QDomElement updated         = entryElem.firstChildElement(QString::fromLatin1("updated"));
    const QDomElement created         = entryElem.firstChildElement(QString::fromLatin1("f:created"));
    const QDomElement accessAttr      = entryElem.firstChildElement(QString::fromLatin1("f:access"));
    const QDomElement hideOriginal    = entryElem.firstChildElement(QString::fromLatin1("f:hide_original"));
    const QDomElement disableComments = entryElem.firstChildElement(QString::fromLatin1("f:disable_comments"));
    const QDomElement adult           = entryElem.firstChildElement(QString::fromLatin1("f:xxx"));
    const QDomElement content         = entryElem.firstChildElement(QString::fromLatin1("content"));

    QDomElement linkSelf;
    QDomElement linkEdit;
    QDomElement linkMedia;
    QDomElement linkAlbum;

    QDomElement linkElem = entryElem.firstChildElement(QString::fromLatin1("link"));

    for ( ; !linkElem.isNull() ;
          linkElem = linkElem.nextSiblingElement(QString::fromLatin1("link")))
    {

        if (linkElem.attribute(QString::fromLatin1("rel")) == QString::fromLatin1("self"))
            linkSelf = linkElem;
        else if (linkElem.attribute(QString::fromLatin1("rel")) == QString::fromLatin1("edit"))
            linkEdit = linkElem;
        else if (linkElem.attribute(QString::fromLatin1("rel")) == QString::fromLatin1("edit-media"))
            linkMedia = linkElem;
        else if (linkElem.attribute(QString::fromLatin1("rel")) == QString::fromLatin1("album"))
            linkAlbum = linkElem;
        // else skip <link>
    }

    // XML sanity checks
    if (urn.isNull()       || title.isNull()              ||
        linkSelf.isNull()  || linkEdit.isNull()           ||
        linkMedia.isNull() || linkAlbum.isNull()          ||
        !content.hasAttribute(QString::fromLatin1("src")) ||
        !accessAttr.hasAttribute(QString::fromLatin1("value")))
    {

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML data, error on line" << entryElem.lineNumber();
        // simple skip this record, no addtional messages to user
        return false;
    }

    const QString accessString = accessAttr.attribute(QString::fromLatin1("value"));

    YFPhoto::Access access;

    if (accessString == d->ACCESS_STRINGS[YFPhoto::ACCESS_PRIVATE])
        access = YFPhoto::ACCESS_PRIVATE;
    else if (accessString == d->ACCESS_STRINGS[YFPhoto::ACCESS_FRIENDS])
        access = YFPhoto::ACCESS_FRIENDS;
    else if (accessString == d->ACCESS_STRINGS[YFPhoto::ACCESS_PUBLIC])
        access = YFPhoto::ACCESS_PUBLIC;
    else
    {
        qCCritical(DIGIKAM_WEBSERVICES_LOG) << "Unknown photo access level: " << accessString;
        access = YFPhoto::ACCESS_PUBLIC;
    }

    photo.m_urn           = urn.text();
    photo.m_author        = author.text();

    photo.setTitle(title.text());
    photo.setSummary(summary.text());
    photo.m_apiEditUrl    = linkEdit.attribute(QString::fromLatin1("href"));
    photo.m_apiSelfUrl    = linkSelf.attribute(QString::fromLatin1("href"));
    photo.m_apiMediaUrl   = linkMedia.attribute(QString::fromLatin1("href"));
    photo.m_apiAlbumUrl   = linkAlbum.attribute(QString::fromLatin1("href"));
    photo.m_publishedDate = QDateTime::fromString(published.text(), QString::fromLatin1("yyyy-MM-ddTHH:mm:ssZ"));
    photo.m_editedDate    = QDateTime::fromString(edited.text(), QString::fromLatin1("yyyy-MM-ddTHH:mm:ssZ"));
    photo.m_updatedDate   = QDateTime::fromString(updated.text(), QString::fromLatin1("yyyy-MM-ddTHH:mm:ssZ"));
    photo.m_createdDate   = QDateTime::fromString(created.text(), QString::fromLatin1("yyyy-MM-ddTHH:mm:ss"));

    photo.setAccess(access);
    photo.setHideOriginal(hideOriginal.attribute(
        QString::fromLatin1("value"), QString::fromLatin1("false")) == QString::fromLatin1("true"));
    photo.setDisableComments(disableComments.attribute(
        QString::fromLatin1("value"), QString::fromLatin1("false")) == QString::fromLatin1("true"));
    photo.setAdult(adult.attribute(
        QString::fromLatin1("value"), QString::fromLatin1("false")) == QString::fromLatin1("true"));

    photo.m_remoteUrl = content.attribute(QString::fromLatin1("src"));

    /*
     * FIXME: tags part of the API is not documented by Yandex
     */

    // reload all tags from the response
    photo.tags.clear();
    QDomElement category = entryElem.firstChildElement(QString::fromLatin1("category"));

    for ( ; !category.isNull() ;
         category = category.nextSiblingElement(QString::fromLatin1("category")))
    {
        if (category.hasAttribute(QString::fromLatin1("term")) &&
            category.hasAttribute(QString::fromLatin1("scheme")) &&
            // FIXME: I have no idea how to make its better, usable API is needed
            category.attribute(QString::fromLatin1("scheme")) == d->apiTagsUrl)
        {
            photo.tags.append(category.attribute(QString::fromLatin1("term")));
        }
    }

    return true;
}

void YFTalker::slotParseResponseListPhotos()
{
    QDomDocument doc(QString::fromLatin1("feed"));

    if (!doc.setContent(d->buffer))
    {
        qCCritical(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML, parse error: " << d->buffer;
        return setErrorState(STATE_LISTPHOTOS_ERROR);
    }

    int initialSize            = d->photos.size();
    bool errorOccurred         = false;
    const QDomElement rootElem = doc.documentElement();

    // find next page link
    d->photosNextUrl.clear();
    QDomElement linkElem       = rootElem.firstChildElement(QString::fromLatin1("link"));

    for ( ; !linkElem.isNull() ;
          linkElem = linkElem.nextSiblingElement(QString::fromLatin1("link")))
    {
        if (linkElem.attribute(QString::fromLatin1("rel")) == QString::fromLatin1("next") &&
            !linkElem.attribute(QString::fromLatin1("href")).isNull())
        {
            d->photosNextUrl = linkElem.attribute(QString::fromLatin1("href"));
            break;
        }
    }

    QDomElement entryElem = rootElem.firstChildElement(QString::fromLatin1("entry"));

    for ( ; !entryElem.isNull() ;
          entryElem = entryElem.nextSiblingElement(QString::fromLatin1("entry")))
    {
        YFPhoto photo;

        if (slotParsePhotoXml(entryElem, photo))
        {
            d->photos.append(photo);
        }
        else
        {
            // set error mark and conintinue
            errorOccurred = true;
        }
    }

    // if an error has occurred and we didn't find anything => notify user
    if (errorOccurred && initialSize == d->photos.size())
    {
        qCCritical(DIGIKAM_WEBSERVICES_LOG) << "No photos found, some XML errors have occurred";
        return setErrorState(STATE_LISTPHOTOS_ERROR);
    }

    // we have next page
    if (!d->photosNextUrl.isNull())
    {
        return listPhotosNext();
    }
    else
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "List photos done: " << d->photos.size();
        d->state = STATE_LISTPHOTOS_DONE;
        emit signalListPhotosDone(d->photos);
    }
}

void YFTalker::slotParseResponseUpdatePhotoFile()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Uploaded photo document" << d->buffer;
    QDomDocument doc(QString::fromLatin1("entry"));

    if (!doc.setContent(d->buffer))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML, parse error" << d->buffer;
        return setErrorState(STATE_UPDATEPHOTO_INFO_ERROR);
    }

    YFPhoto& photo              = *d->lastPhoto;
    YFPhoto tmpPhoto;
    const QDomElement entryElem = doc.documentElement();

    if (!slotParsePhotoXml(entryElem, tmpPhoto))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML, entry not found" << d->buffer;
        return setErrorState(STATE_UPDATEPHOTO_INFO_ERROR);
    }

    photo.m_urn         = tmpPhoto.m_urn;
    photo.m_apiEditUrl  = tmpPhoto.m_apiEditUrl;
    photo.m_apiSelfUrl  = tmpPhoto.m_apiSelfUrl;
    photo.m_apiMediaUrl = tmpPhoto.m_apiMediaUrl;
    photo.m_remoteUrl   = tmpPhoto.m_remoteUrl;
    photo.m_remoteUrl   = tmpPhoto.m_remoteUrl;
    photo.m_author      = tmpPhoto.m_author;

    // update info
    updatePhotoInfo(photo);
}

void YFTalker::slotParseResponseUpdatePhotoInfo()
{
    YFPhoto& photo = *d->lastPhoto;

/*
    // reload all information
    QDomDocument doc("entry");

    if (!doc.setContent(d->buffer))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML: parse error" << d->buffer;
        return setErrorState(STATE_UPDATEPHOTO_INFO_ERROR);
    }

    const QDomElement entryElem = doc.documentElement();

    if (!slotParsePhotoXml(entryElem, photo))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Can't reload photo after uploading";
        return setErrorState(STATE_UPDATEPHOTO_INFO_ERROR);
    }
*/

    d->state     = STATE_UPDATEPHOTO_DONE;
    d->lastPhoto = 0;
    emit signalUpdatePhotoDone(photo);
}

void YFTalker::slotParseResponseUpdateAlbum()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Updated album" << d->buffer;

    d->state     = STATE_UPDATEALBUM_DONE;
    d->lastPhoto = 0;

    emit signalUpdateAlbumDone();
}

} // namespace Digikam

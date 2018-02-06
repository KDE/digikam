/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-14
 * Description : a tool to export items to YandexFotki web service
 *
 * Copyright (C) 2010 by Roman Tsisyk <roman at tsisyk dot com>
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
#include <QDomElement>
#include <QDomNode>
#include <QPointer>
#include <QFile>
#include <QFileInfo>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "yfauth.h"
#include "yfalbum.h"

namespace Digikam
{
/*
 * static API constants
 */
const QString YFTalker::SESSION_URL          = QString::fromLatin1("http://auth.mobile.yandex.ru/yamrsa/key/");
const QString YFTalker::AUTH_REALM           = QString::fromLatin1("fotki.yandex.ru");
const QString YFTalker::TOKEN_URL            = QString::fromLatin1("http://auth.mobile.yandex.ru/yamrsa/token/");
const QString YFTalker::SERVICE_URL          = QString::fromLatin1("http://api-fotki.yandex.ru/api/users/%1/");
const QString YFTalker::USERPAGE_URL         = QString::fromLatin1("http://fotki.yandex.ru/users/%1/");
const QString YFTalker::USERPAGE_DEFAULT_URL = QString::fromLatin1("http://fotki.yandex.ru/");
const QString YFTalker::ACCESS_STRINGS[]     = {
    QString::fromLatin1("public"),
    QString::fromLatin1("friends"),
    QString::fromLatin1("private") };

YFTalker::YFTalker(QObject* const parent)
    : QObject(parent),
      m_state(STATE_UNAUTHENTICATED),
      m_lastPhoto(0),
      m_netMngr(0),
      m_reply(0)
{
    m_netMngr = new QNetworkAccessManager(this);

    connect(m_netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));
}

YFTalker::~YFTalker()
{
    reset();
}

void YFTalker::getService()
{
    m_state = STATE_GETSERVICE;

    QUrl url(SERVICE_URL.arg(m_login));

    m_reply = m_netMngr->get(QNetworkRequest(url));

    m_buffer.resize(0);
}

/*
void YFTalker::checkToken()
{
    // try to get somthing with our token, if it is invalid catch 401
    m_state = STATE_CHECKTOKEN;

    QUrl url(m_apiAlbumsUrl);
    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Authorization", QString::fromLatin1("FimpToken realm=\"%1\", token=\"%2\"")
                                             .arg(AUTH_REALM).arg(m_token).toLatin1());

    m_reply = m_netMngr->get(netRequest);

    // Error:    STATE_CHECKTOKEN_INVALID
    // Function: parseResponseCheckToken()

    m_buffer.resize(0);
}
*/

void YFTalker::getSession()
{
    if (m_state != STATE_GETSERVICE_DONE)
        return;

    m_state = STATE_GETSESSION;

    QUrl url(SESSION_URL);

    m_reply = m_netMngr->get(QNetworkRequest(url));

    m_buffer.resize(0);
}

void YFTalker::getToken()
{
    if (m_state != STATE_GETSESSION_DONE)
        return;

    const QString credentials = YFAuth::makeCredentials(m_sessionKey,
                                m_login, m_password);

    // prepare params
    QStringList paramList;

    paramList.append(QLatin1String("request_id=") + m_sessionId);

    paramList.append(QLatin1String("credentials=") + QString::fromUtf8(QUrl::toPercentEncoding(credentials)));

    QString params = paramList.join(QString::fromLatin1("&"));

    m_state = STATE_GETTOKEN;

    QUrl url(TOKEN_URL);
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    m_reply = m_netMngr->post(netRequest, params.toUtf8());

    m_buffer.resize(0);
}

void YFTalker::listAlbums()
{
    if (isErrorState() || !isAuthenticated())
        return;

    m_albumsNextUrl = m_apiAlbumsUrl;
    m_albums.clear();
    listAlbumsNext();
}

void YFTalker::listAlbumsNext()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "listAlbumsNext";

    m_state = STATE_LISTALBUMS;

    QUrl url(m_albumsNextUrl);
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/atom+xml; charset=utf-8; type=feed"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("FimpToken realm=\"%1\", token=\"%2\"")
                                             .arg(AUTH_REALM).arg(m_token).toLatin1());

    m_reply = m_netMngr->get(netRequest);

    m_buffer.resize(0);
}

void YFTalker::listPhotos(const YandexFotkiAlbum& album)
{
    if (isErrorState() || !isAuthenticated())
        return;

    m_photosNextUrl = album.m_apiPhotosUrl;
    m_photos.clear();
    listPhotosNext();
}

// protected member
void YFTalker::listPhotosNext()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "listPhotosNext";

    m_state = STATE_LISTPHOTOS;

    QUrl url(m_photosNextUrl);
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/atom+xml; charset=utf-8; type=feed"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("FimpToken realm=\"%1\", token=\"%2\"")
                                             .arg(AUTH_REALM).arg(m_token).toLatin1());

    m_reply = m_netMngr->get(netRequest);

    m_buffer.resize(0);
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
    m_lastPhotosUrl = album.m_apiPhotosUrl;

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

    m_state     = STATE_UPDATEPHOTO_FILE;
    m_lastPhoto = &photo;

    QUrl url(m_lastPhotosUrl);
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("image/jpeg"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("FimpToken realm=\"%1\", token=\"%2\"")
                                             .arg(AUTH_REALM).arg(m_token).toLatin1());
    netRequest.setRawHeader("Slug", QUrl::toPercentEncoding(photo.title()) + ".jpg");

    m_reply = m_netMngr->post(netRequest, imageFile.readAll());

    m_buffer.resize(0);
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

    QDomElement summary = doc.createElement(QString::fromLatin1("summary"));
    summary.appendChild(doc.createTextNode(photo.summary()));
    entryElem.appendChild(summary);

    QDomElement adult = doc.createElement(QString::fromLatin1("f:xxx"));
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
    access.setAttribute(QString::fromLatin1("value"), ACCESS_STRINGS[photo.access()]);
    entryElem.appendChild(access);

    // FIXME: undocumented API
    foreach(const QString& t, photo.tags)
    {
        QDomElement tag = doc.createElement(QString::fromLatin1("category"));
        tag.setAttribute(QString::fromLatin1("scheme"), m_apiTagsUrl);
        tag.setAttribute(QString::fromLatin1("term"), t);
        entryElem.appendChild(tag);
    }

    QByteArray buffer = doc.toString(1).toUtf8(); // with idents

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Prepared data: " << buffer;
    m_lastPhoto = &photo;

    m_state = STATE_UPDATEPHOTO_INFO;

    QUrl url(photo.m_apiEditUrl);
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/atom+xml; charset=utf-8; type=entry"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("FimpToken realm=\"%1\", token=\"%2\"")
                                             .arg(AUTH_REALM).arg(m_token).toLatin1());

    m_reply = m_netMngr->put(netRequest, buffer);

    m_buffer.resize(0);
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
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Url" << m_apiAlbumsUrl;

    m_state = STATE_UPDATEALBUM;

    QUrl url(m_apiAlbumsUrl);
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/atom+xml; charset=utf-8; type=entry"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("FimpToken realm=\"%1\", token=\"%2\"")
                                             .arg(AUTH_REALM).arg(m_token).toLatin1());

    m_reply = m_netMngr->post(netRequest, postData);

    m_buffer.resize(0);
}

void YFTalker::reset()
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    m_token.clear();
    m_state = STATE_UNAUTHENTICATED;
}

void YFTalker::cancel()
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    if (isAuthenticated())
    {
        m_state = STATE_AUTHENTICATED;
    }
    else
    {
        m_token.clear();
        m_state = STATE_UNAUTHENTICATED;
    }
}

void YFTalker::setErrorState(State state)
{
    m_state = state;
    emit signalError();
}

void YFTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != m_reply)
    {
        return;
    }

    m_reply = 0;

    if (reply->error() != QNetworkReply::NoError)
    {
        int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Transfer Error" << code << reply->errorString();

        if (code == 401 || code == 403 || code == 404) // auth required, 404 user not found
        {
            setErrorState(STATE_INVALID_CREDENTIALS);
        }
        else if (m_state == STATE_GETSERVICE)
        {
            setErrorState(STATE_GETSERVICE_ERROR);
        }
        else if (m_state == STATE_GETSESSION)
        {
            setErrorState(STATE_GETSESSION_ERROR);
        }
        else if (m_state == STATE_GETTOKEN)
        {
            setErrorState(STATE_GETTOKEN_ERROR);
        }
        else if (m_state == STATE_LISTALBUMS)
        {
            setErrorState(STATE_LISTALBUMS_ERROR);
        }
        else if (m_state == STATE_LISTPHOTOS)
        {
            setErrorState(STATE_LISTPHOTOS_ERROR);
        }
        else if (m_state == STATE_UPDATEPHOTO_FILE)
        {
            setErrorState(STATE_UPDATEPHOTO_FILE_ERROR);
        }
        else if (m_state == STATE_UPDATEPHOTO_INFO)
        {
            setErrorState(STATE_UPDATEPHOTO_INFO_ERROR);
        }
        else if (m_state == STATE_UPDATEALBUM)
        {
            setErrorState(STATE_UPDATEALBUM_ERROR);
        }

        reply->deleteLater();
        return;
    }

    m_buffer.append(reply->readAll());

    switch(m_state)
    {
        case (STATE_GETSERVICE):
            parseResponseGetService();
            break;
        case (STATE_GETSESSION):
            parseResponseGetSession();
            break;
        case (STATE_GETTOKEN):
            parseResponseGetToken();
            break;
        case (STATE_LISTALBUMS):
            parseResponseListAlbums();
            break;
        case (STATE_LISTPHOTOS):
            parseResponseListPhotos();
            break;
        case (STATE_UPDATEPHOTO_FILE):
            parseResponseUpdatePhotoFile();
            break;
        case (STATE_UPDATEPHOTO_INFO):
            parseResponseUpdatePhotoInfo();
            break;
        case (STATE_UPDATEALBUM):
            parseResponseUpdateAlbum();
            break;
        default:
            break;
    }

    reply->deleteLater();
}

void YFTalker::parseResponseGetService()
{
    QDomDocument doc(QString::fromLatin1("service"));

    if (!doc.setContent(m_buffer))
    {
        qCCritical(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML: parse error" << m_buffer;
        return setErrorState(STATE_GETSERVICE_ERROR);
    }

    const QDomElement rootElem = doc.documentElement();

    QDomElement workspaceElem = rootElem.firstChildElement(QString::fromLatin1("app:workspace"));

    // FIXME: workaround for Yandex xml namespaces bugs
    QString prefix = QString::fromLatin1("app:");

    if (workspaceElem.isNull())
    {
        workspaceElem = rootElem.firstChildElement(QString::fromLatin1("workspace"));
        prefix = QString();
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

    for ( ; !collectionElem.isNull();
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
        } // else skip unknown section
    }

    if (apiAlbumsUrl.isNull() || apiPhotosUrl.isNull())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML data: service URLs";
        return setErrorState(STATE_GETSERVICE_ERROR);
    }

    m_apiAlbumsUrl = apiAlbumsUrl;
    m_apiPhotosUrl = apiPhotosUrl;
    m_apiTagsUrl = apiTagsUrl;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "ServiceUrls:";
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Albums" << m_apiAlbumsUrl;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Photos" << m_apiPhotosUrl;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Tags" << m_apiTagsUrl;

    m_state = STATE_GETSERVICE_DONE;
    emit signalGetServiceDone();
}

/*
void YFTalker::parseResponseCheckToken()
{
    // token still valid, skip getSession and getToken
    m_state = STATE_GETTOKEN_DONE;
    emit signalGetTokenDone();
}
*/

void YFTalker::parseResponseGetSession()
{
    QDomDocument doc(QString::fromLatin1("session"));

    if (!doc.setContent(m_buffer))
    {
        return setErrorState(STATE_GETSESSION_ERROR);
    }

    const QDomElement rootElem = doc.documentElement();

    const QDomElement keyElem =  rootElem.firstChildElement(QString::fromLatin1("key"));

    const QDomElement requestIdElem =  rootElem.firstChildElement(QString::fromLatin1("request_id"));

    if (keyElem.isNull() || keyElem.nodeType() != QDomNode::ElementNode ||
        requestIdElem.isNull() || requestIdElem.nodeType() != QDomNode::ElementNode)
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML" << m_buffer;
        return setErrorState(STATE_GETSESSION_ERROR);
    }

    m_sessionKey = keyElem.text();
    m_sessionId  = requestIdElem.text();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Session started" << m_sessionKey << m_sessionId;

    m_state = STATE_GETSESSION_DONE;
    emit signalGetSessionDone();
}

void YFTalker::parseResponseGetToken()
{
    QDomDocument doc(QString::fromLatin1("response"));

    if (!doc.setContent(m_buffer))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML: parse error" << m_buffer;
        return setErrorState(STATE_GETTOKEN_ERROR);
    }

    const QDomElement rootElem  = doc.documentElement();
    const QDomElement tokenElem =  rootElem.firstChildElement(QString::fromLatin1("token"));

    if (tokenElem.isNull() || tokenElem.nodeType() != QDomNode::ElementNode)
    {
        const QDomElement errorElem =  rootElem.firstChildElement(QString::fromLatin1("error"));

        if (errorElem.isNull() || errorElem.nodeType() != QDomNode::ElementNode)
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Auth unknown error";
            return setErrorState(STATE_GETTOKEN_ERROR);
        }

        /*
          // checked by HTTP error code in prepareJobResult
        const QString errorCode = errorElem.attribute("code", "0");
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << QString("Auth error: %1, code=%2").arg(errorElem.text()).arg(errorCode);

        if (errorCode == "2")  { // Invalid credentials
            return setErrorState(STATE_GETTOKEN_INVALID_CREDENTIALS);
        }
        */

        return;
    }

    m_token = tokenElem.text();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Token got" << m_token;
    m_state = STATE_GETTOKEN_DONE;
    emit signalGetTokenDone();
}


void YFTalker::parseResponseListAlbums()
{
    QDomDocument doc(QString::fromLatin1("feed"));

    if (!doc.setContent(m_buffer))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML: parse error";
        return setErrorState(STATE_LISTALBUMS_ERROR);
    }

    bool errorOccurred         = false;
    const QDomElement rootElem = doc.documentElement();

    // find next page link
    m_albumsNextUrl.clear();
    QDomElement linkElem = rootElem.firstChildElement(QString::fromLatin1("link"));

    for ( ; !linkElem.isNull();
          linkElem = linkElem.nextSiblingElement(QString::fromLatin1("link")))
    {
        if (linkElem.attribute(QString::fromLatin1("rel")) == QString::fromLatin1("next") &&
            !linkElem.attribute(QString::fromLatin1("href")).isNull())
        {
            m_albumsNextUrl = linkElem.attribute(QString::fromLatin1("href"));
            break;
        }
    }

    QDomElement entryElem = rootElem.firstChildElement(QString::fromLatin1("entry"));

    for ( ; !entryElem.isNull();
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

        for ( ; !linkElem.isNull();
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

        m_albums.append(YandexFotkiAlbum(
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

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Found album:" << m_albums.last();
    }

    // TODO: pagination like listPhotos

    // if an error has occurred and we didn't find anything => notify user
    if (errorOccurred && m_albums.empty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "No result and errors have occurred";
        return setErrorState(STATE_LISTALBUMS_ERROR);
    }

    // we have next page
    if (!m_albumsNextUrl.isNull())
    {
        return listAlbumsNext();
    }
    else
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "List albums done: " << m_albums.size();
        m_state = STATE_LISTALBUMS_DONE;
        emit signalListAlbumsDone(m_albums);
    }
}

bool YFTalker::parsePhotoXml(const QDomElement& entryElem, YFPhoto& photo)
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

    for ( ; !linkElem.isNull();
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
    if (urn.isNull() || title.isNull() ||
        linkSelf.isNull() || linkEdit.isNull() ||
        linkMedia.isNull() || linkAlbum.isNull() ||
        !content.hasAttribute(QString::fromLatin1("src")) ||
        !accessAttr.hasAttribute(QString::fromLatin1("value")))
    {

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML data, error on line" << entryElem.lineNumber();
        // simple skip this record, no addtional messages to user
        return false;
    }

    const QString accessString = accessAttr.attribute(QString::fromLatin1("value"));

    YFPhoto::Access access;

    if (accessString == ACCESS_STRINGS[YFPhoto::ACCESS_PRIVATE])
        access = YFPhoto::ACCESS_PRIVATE;
    else if (accessString == ACCESS_STRINGS[YFPhoto::ACCESS_FRIENDS])
        access = YFPhoto::ACCESS_FRIENDS;
    else if (accessString == ACCESS_STRINGS[YFPhoto::ACCESS_PUBLIC])
        access = YFPhoto::ACCESS_PUBLIC;
    else
    {
        qCCritical(DIGIKAM_WEBSERVICES_LOG) << "Unknown photo access level: " << accessString;
        access = YFPhoto::ACCESS_PUBLIC;
    }

    photo.m_urn    = urn.text();
    photo.m_author = author.text();

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

    for ( ; !category.isNull();
         category = category.nextSiblingElement(QString::fromLatin1("category")))
    {
        if (category.hasAttribute(QString::fromLatin1("term")) &&
            category.hasAttribute(QString::fromLatin1("scheme")) &&
            // FIXME: I have no idea how to make its better, usable API is needed
            category.attribute(QString::fromLatin1("scheme")) == m_apiTagsUrl)
        {
            photo.tags.append(category.attribute(QString::fromLatin1("term")));
        }
    }

    return true;
}

void YFTalker::parseResponseListPhotos()
{
    QDomDocument doc(QString::fromLatin1("feed"));

    if (!doc.setContent(m_buffer))
    {
        qCCritical(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML, parse error: " << m_buffer;
        return setErrorState(STATE_LISTPHOTOS_ERROR);
    }

    int initialSize    = m_photos.size();
    bool errorOccurred = false;

    const QDomElement rootElem = doc.documentElement();

    // find next page link
    m_photosNextUrl.clear();
    QDomElement linkElem = rootElem.firstChildElement(QString::fromLatin1("link"));

    for ( ; !linkElem.isNull();
          linkElem = linkElem.nextSiblingElement(QString::fromLatin1("link")))
    {
        if (linkElem.attribute(QString::fromLatin1("rel")) == QString::fromLatin1("next") &&
            !linkElem.attribute(QString::fromLatin1("href")).isNull())
        {
            m_photosNextUrl = linkElem.attribute(QString::fromLatin1("href"));
            break;
        }
    }

    QDomElement entryElem = rootElem.firstChildElement(QString::fromLatin1("entry"));
    for ( ; !entryElem.isNull();
          entryElem = entryElem.nextSiblingElement(QString::fromLatin1("entry")))
    {
        YFPhoto photo;

        if (parsePhotoXml(entryElem, photo))
        {
            m_photos.append(photo);
        }
        else
        {
            // set error mark and conintinue
            errorOccurred = true;
        }
    }

    // if an error has occurred and we didn't find anything => notify user
    if (errorOccurred && initialSize == m_photos.size())
    {
        qCCritical(DIGIKAM_WEBSERVICES_LOG) << "No photos found, some XML errors have occurred";
        return setErrorState(STATE_LISTPHOTOS_ERROR);
    }

    // we have next page
    if (!m_photosNextUrl.isNull())
    {
        return listPhotosNext();
    }
    else
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "List photos done: " << m_photos.size();
        m_state = STATE_LISTPHOTOS_DONE;
        emit signalListPhotosDone(m_photos);
    }
}

void YFTalker::parseResponseUpdatePhotoFile()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Uploaded photo document" << m_buffer;
    QDomDocument doc(QString::fromLatin1("entry"));

    if (!doc.setContent(m_buffer))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML, parse error" << m_buffer;
        return setErrorState(STATE_UPDATEPHOTO_INFO_ERROR);
    }

    YFPhoto& photo = *m_lastPhoto;

    YFPhoto tmpPhoto;
    const QDomElement entryElem = doc.documentElement();

    if (!parsePhotoXml(entryElem, tmpPhoto))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML, entry not found" << m_buffer;
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

void YFTalker::parseResponseUpdatePhotoInfo()
{
    YFPhoto& photo = *m_lastPhoto;

    /*
    // reload all information
    QDomDocument doc("entry");
    if ( !doc.setContent( m_buffer ) )
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Invalid XML: parse error" << m_buffer;
        return setErrorState(STATE_UPDATEPHOTO_INFO_ERROR);
    }

    const QDomElement entryElem = doc.documentElement();
    if(!parsePhotoXml(entryElem, photo))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Can't reload photo after uploading";
        return setErrorState(STATE_UPDATEPHOTO_INFO_ERROR);
    }*/

    m_state     = STATE_UPDATEPHOTO_DONE;
    m_lastPhoto = 0;
    emit signalUpdatePhotoDone(photo);
}

void YFTalker::parseResponseUpdateAlbum()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Updated album" << m_buffer;

    m_state     = STATE_UPDATEALBUM_DONE;
    m_lastPhoto = 0;

    emit signalUpdateAlbumDone();
}

} // namespace Digikam

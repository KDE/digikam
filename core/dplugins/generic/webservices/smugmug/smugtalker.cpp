/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-01
 * Description : a tool to export images to Smugmug web service
 *
 * Copyright (C) 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * Copyright (C) 2008-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "smugtalker.h"

// Qt includes

#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QTextDocument>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopServices>
#include <QCryptographicHash>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QNetworkAccessManager>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "smugmpform.h"
#include "smugitem.h"

// OAuth2 library includes

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wextra-semi"
#endif

#include "wstoolutils.h"
#include "o0settingsstore.h"
#include "o1requestor.h"
#include "o0globals.h"

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

using namespace Digikam;

namespace DigikamGenericSmugPlugin
{

class Q_DECL_HIDDEN SmugTalker::Private
{

public:

    enum State
    {
        SMUG_LOGIN = 0,
        SMUG_LOGOUT,
        SMUG_LISTALBUMS,
        SMUG_LISTPHOTOS,
        SMUG_LISTALBUMTEMPLATES,
        SMUG_CREATEALBUM,
        SMUG_ADDPHOTO,
        SMUG_GETPHOTO
        /*
         * SMUG_LISTCATEGORIES,
         * SMUG_LISTSUBCATEGORIES,
         */
    };

public:

    explicit Private()
    {
        parent          = nullptr;

        userAgent       = QString::fromLatin1("digiKam/%1 (digikamdeveloper@gmail.com)").arg(digiKamVersion());

        apiVersion      = QLatin1String("v2");
        apiURL          = QLatin1String("https://api.smugmug.com%1");
        uploadUrl       = QLatin1String("https://upload.smugmug.com/");
        requestTokenUrl = QLatin1String("https://api.smugmug.com/services/oauth/1.0a/getRequestToken");
        authUrl         = QLatin1String("https://api.smugmug.com/services/oauth/1.0a/authorize");
        accessTokenUrl  = QLatin1String("https://api.smugmug.com/services/oauth/1.0a/getAccessToken");

        apikey          = QLatin1String("xKp43CXF8MHgjhgGdgdgfgc7cWjqQcck");
        clientSecret    = QLatin1String("3CKcLcWx64Rm8HVRwX3bf4HCtJpnGrwnk9xSn4DK8wRhGLVsRBBFktD95W4HTRHD");
        iface           = nullptr;
        netMngr         = nullptr;
        reply           = nullptr;
        state           = SMUG_LOGOUT;

        requestor       = nullptr;
        o1              = nullptr;
        settings        = nullptr;
    }

public:

    QWidget*               parent;

    QString                userAgent;

    QString                apiURL;
    QString                uploadUrl;
    QString                requestTokenUrl;
    QString                authUrl;
    QString                accessTokenUrl;

    QString                apiVersion;
    QString                apikey;
    QString                clientSecret;
    QString                sessionID;

    SmugUser               user;
    DInfoInterface*        iface;

    QNetworkAccessManager* netMngr;

    QNetworkReply*         reply;

    State                  state;

    QSettings*             settings;
    O1Requestor*           requestor;
    O1SmugMug*             o1;
};

SmugTalker::SmugTalker(DInfoInterface* const iface, QWidget* const parent)
    : d(new Private)
{
    d->parent     = parent;
    d->iface      = iface;
    d->netMngr    = new QNetworkAccessManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    // Init
    d->o1 = new O1SmugMug(this, d->netMngr);

    // Config for authentication flow
    d->o1->setRequestTokenUrl(QUrl(d->requestTokenUrl));
    d->o1->setAuthorizeUrl(QUrl(d->authUrl));
    d->o1->setAccessTokenUrl(QUrl(d->accessTokenUrl));
    d->o1->setLocalPort(8000);

    // Application credentials
    d->o1->setClientId(d->apikey);
    d->o1->setClientSecret(d->clientSecret);

    // Set userAgent to work around error :
    // O1::onTokenRequestError: 201 "Error transferring requestTokenUrl() - server replied: Forbidden" "Bad bot"
    d->o1->setUserAgent(d->userAgent.toUtf8());

    // Setting to store oauth config
    d->settings                  = WSToolUtils::getOauthSettings(this);
    O0SettingsStore* const store = new O0SettingsStore(d->settings, QLatin1String(O2_ENCRYPTION_KEY), this);
    store->setGroupKey(QLatin1String("Smugmug"));
    d->o1->setStore(store);

    // Connect signaux slots
    connect(d->o1, SIGNAL(linkingFailed()),
            this, SLOT(slotLinkingFailed()));

    connect(this, SIGNAL(signalLinkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));

    connect(d->o1, SIGNAL(linkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));

    connect(d->o1, SIGNAL(openBrowser(QUrl)),
            this, SLOT(slotOpenBrowser(QUrl)));

    d->requestor = new O1Requestor(d->netMngr, d->o1, this);
}

SmugTalker::~SmugTalker()
{
    /**
     * We shouldn't logout without user's consent
     *
     * if (loggedIn())
     * {
     *    logout();
     *
     *    while (d->reply && d->reply->isRunning())
     *    {
     *        qApp->processEvents();
     *    }
     * }
     */

    if (d->reply)
        d->reply->abort();

    delete d;
}

//TODO: Porting to O2
void SmugTalker::link()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Smug ";
    d->o1->link();
}

void SmugTalker::unlink()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UNLINK to Smug ";
    d->o1->unlink();
}

void SmugTalker::removeUserAccount(const QString& /*userName*/)
{
/*
    if (userName.startsWith(d->serviceName))
    {
        d->settings->beginGroup(userName);
        d->settings->remove(QString());
        d->settings->endGroup();
    }
*/
}

bool SmugTalker::loggedIn() const
{
    return d->o1->linked();
}

void SmugTalker::slotLinkingFailed()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Smug fail";
    emit signalBusy(false);
    getLoginedUser();
}

void SmugTalker::slotLinkingSucceeded()
{
    if (!d->o1->linked())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UNLINK to Smug ok";

        // Remove user account
        removeUserAccount(d->user.nickName);

        // Clear user field
        d->user.clear();

        // Set state to SMUG_LOGOUT
        d->state = Private::SMUG_LOGOUT;

        emit signalBusy(false);
        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Smug ok";

    getLoginedUser();
}

void SmugTalker::slotOpenBrowser(const QUrl& url)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Open Browser...";
    QDesktopServices::openUrl(url);
}

void SmugTalker::slotCloseBrowser()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Close Browser...";
}

SmugUser SmugTalker::getUser() const
{
    return d->user;
}

/**
 * (Trung)
 * There are some characters not valid for album title (e.g "_")
 * and for url (e.g "-") that are not mentioned on the API page,
 * so if found, that has to be treated here
 */
QString SmugTalker::createAlbumName(const QString& word)
{
    QString w(word);

    // First we remove space at beginning and end
    w = w.trimmed();

    // We replace all character "_" with space
    w = w.replace(QLatin1Char('_'), QLatin1Char(' '));

    // Then we replace first letter with its uppercase
    w.replace(0, 1, w[0].toUpper());

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << w;

    return w;
}

QString SmugTalker::createAlbumUrl(const QString& name)
{
    QString n(name);

    // First we create a valid name
    n = createAlbumName(n);

    // Then we replace space with "-"
    QStringList words = n.split(QLatin1Char(' '));
    n = words.join(QLatin1Char('-'));

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url name : " << n;

    return n;
}

void SmugTalker::cancel()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    emit signalBusy(false);
}

void SmugTalker::login()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    emit signalBusy(true);
    emit signalLoginProgress(1, 4, i18n("Logging in to SmugMug service..."));

    // Build authentication url
    O1SmugMug::AuthorizationUrlBuilder builder;
    builder.setAccess(O1SmugMug::AccessFull);
    builder.setPermissions(O1SmugMug::PermissionsModify);
    d->o1->initAuthorizationUrl(builder);

    if (!d->o1->linked())
    {
        link();
    }
    else
    {
        emit signalLinkingSucceeded();
    }
}

void SmugTalker::getLoginedUser()
{
    QUrl url(d->apiURL.arg(QLatin1String("/api/v2!authuser")));
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url = " << url.url();

    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Accept", "application/json");
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader,   d->userAgent);

    d->reply = d->requestor->get(netRequest, reqParams);

    d->state = Private::SMUG_LOGIN;
}

void SmugTalker::logout()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    emit signalBusy(true);

    unlink();
}

void SmugTalker::listAlbums(const QString& /*nickName*/)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    emit signalBusy(true);

    QUrl url(d->apiURL.arg(QString::fromLatin1("%1!albums").arg(d->user.userUri)));
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url = " << url.url();

    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Accept", "application/json");
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader,   d->userAgent);

    d->reply = d->requestor->get(netRequest, reqParams);

    d->state = Private::SMUG_LISTALBUMS;
}

void SmugTalker::listPhotos(const qint64 /*albumID*/,
                            const QString& albumKey,
                            const QString& /*albumPassword*/,
                            const QString& /*sitePassword*/)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    emit signalBusy(true);

    QUrl url(d->apiURL.arg(QString::fromLatin1("/api/v2/album/%1!images").arg(albumKey)));
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "list photo " << url.url();

    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Accept", "application/json");
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader,   d->userAgent);

    d->reply = d->requestor->get(netRequest, reqParams);

    d->state = Private::SMUG_LISTPHOTOS;
}

void SmugTalker::listAlbumTmpl()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    emit signalBusy(true);

    QUrl url(d->apiURL.arg(QString::fromLatin1("%1!albumtemplates").arg(d->user.userUri)));
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url to listAlbumTmpl " << url.url();

    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Accept", "application/json");
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader,   d->userAgent);

    d->reply = d->requestor->get(netRequest, reqParams);

    d->state = Private::SMUG_LISTALBUMTEMPLATES;
}

/**
 * Categories deprecated in API v2
 *
void SmugTalker::listCategories()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);

    QUrl url(d->apiURL);
    QUrlQuery q;
    q.addQueryItem(QLatin1String("method"),    QLatin1String("smugmug.categories.get"));
    q.addQueryItem(QLatin1String("SessionID"), d->sessionID);
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader,   d->userAgent);

    d->reply = d->netMngr->get(netRequest);

    d->state = Private::SMUG_LISTCATEGORIES;
}

void SmugTalker::listSubCategories(qint64 categoryID)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);

    QUrl url(d->apiURL);
    QUrlQuery q;
    q.addQueryItem(QLatin1String("method"),     QLatin1String("smugmug.subcategories.get"));
    q.addQueryItem(QLatin1String("SessionID"),  d->sessionID);
    q.addQueryItem(QLatin1String("CategoryID"), QString::number(categoryID));
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader,   d->userAgent);

    d->reply = d->netMngr->get(netRequest);

    d->state = Private::SMUG_LISTSUBCATEGORIES;
}
*/

void SmugTalker::createAlbum(const SmugAlbum& album)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    emit signalBusy(true);

    QUrl url(d->apiURL.arg(QString::fromLatin1("%1!albums").arg(d->user.folderUri)));
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url to post " << url.url();

    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    /**
     * Something must to be remembered here is that we HAVE TO start a name with upper case !!!
     * And url name with '-' instead of space
     */
    QByteArray data;
    data += "{\"Name\": \"";
    data += createAlbumName(album.title).toUtf8();
    data += "\",\"UrlName\":\"";
    data += createAlbumUrl(album.title).toUtf8();
    data += "\",\"Privacy\":\"Public\"}";
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << data;

    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Accept", "application/json");
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader,   d->userAgent);

    d->reply = d->requestor->post(netRequest, reqParams, data);

    d->state = Private::SMUG_CREATEALBUM;
}

bool SmugTalker::addPhoto(const  QString& imgPath,
                          qint64 /*albumID*/,
                          const  QString& albumKey,
                          const  QString& caption)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    emit signalBusy(true);

    QString imgName = QFileInfo(imgPath).fileName();

    // load temporary image to buffer
    QFile imgFile(imgPath);

    if (!imgFile.open(QIODevice::ReadOnly))
    {
        emit signalBusy(false);
        return false;
    }
    QByteArray imgData = imgFile.readAll();
    imgFile.close();

    SmugMPForm form;

    if (!caption.isEmpty())
        form.addPair(QLatin1String("Caption"), caption);

    if (!form.addFile(imgName, imgPath))
        return false;

    form.finish();

    QString customHdr;
    QUrl url(d->uploadUrl);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url to upload " << url.url();

    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());
    netRequest.setHeader(QNetworkRequest::UserAgentHeader,   d->userAgent);
    netRequest.setRawHeader("X-Smug-Caption", caption.toUtf8());
    netRequest.setRawHeader("X-Smug-FileName", imgName.toUtf8());
    netRequest.setRawHeader("X-Smug-AlbumUri", QString::fromLatin1("/api/v2/album/%1").arg(albumKey).toUtf8());
    netRequest.setRawHeader("X-Smug-ResponseType", "JSON");
    netRequest.setRawHeader("X-Smug-Version",   d->apiVersion.toLatin1());

    d->reply = d->requestor->post(netRequest, reqParams, form.formData());

    d->state = Private::SMUG_ADDPHOTO;
    return true;
}

void SmugTalker::getPhoto(const QString& imgPath)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    emit signalBusy(true);

    QUrl url(imgPath);

    QUrlQuery q;
    q.addQueryItem(QLatin1String("APIKey"), d->apikey);

    url.setQuery(q);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "download link for image " << url.url();

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::UserAgentHeader, d->userAgent);

    d->reply = d->netMngr->get(netRequest);

    d->state = Private::SMUG_GETPHOTO;
}

QString SmugTalker::errorToText(int errCode, const QString& errMsg) const
{
    QString transError;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "errorToText: " << errCode << ": " << errMsg;

    switch (errCode)
    {
        case 0:
            transError = QString();
            break;
        case 1:
            transError = i18n("Login failed");
            break;
        case 4:
            transError = i18n("Invalid user/nick/password");
            break;
        case 18:
            transError = i18n("Invalid API key");
            break;
        default:
            transError = errMsg;
            break;
    }

    return transError;
}

void SmugTalker::slotFinished(QNetworkReply* reply)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "error code : " << reply->error() << "error text " << reply->errorString();
    if (reply != d->reply)
    {
        return;
    }

    d->reply = nullptr;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (d->state == Private::SMUG_LOGIN)
        {
            d->sessionID.clear();
            d->user.clear();

            emit signalBusy(false);
            emit signalLoginDone(reply->error(), reply->errorString());
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "error code : " << reply->error() << "error text " << reply->errorString();
        }
        else if (d->state == Private::SMUG_ADDPHOTO)
        {
            emit signalBusy(false);
            emit signalAddPhotoDone(reply->error(), reply->errorString());
        }
        else if (d->state == Private::SMUG_GETPHOTO)
        {
            emit signalBusy(false);
            emit signalGetPhotoDone(reply->error(), reply->errorString(), QByteArray());
        }
        else
        {
            emit signalBusy(false);
            QMessageBox::critical(QApplication::activeWindow(),
                                i18n("Error"), reply->errorString());
        }

        reply->deleteLater();
        return;
    }

    QByteArray buffer = reply->readAll();

    switch(d->state)
    {
        case (Private::SMUG_LOGIN):
            parseResponseLogin(buffer);
            break;
        case (Private::SMUG_LISTALBUMS):
            parseResponseListAlbums(buffer);
            break;
        case (Private::SMUG_LISTPHOTOS):
            parseResponseListPhotos(buffer);
            break;
        case (Private::SMUG_LISTALBUMTEMPLATES):
            parseResponseListAlbumTmpl(buffer);
            break;
        /*
            case (Private::SMUG_LISTCATEGORIES):
                parseResponseListCategories(buffer);
                break;
            case (Private::SMUG_LISTSUBCATEGORIES):
                parseResponseListSubCategories(buffer);
                break;
        */
        case (Private::SMUG_CREATEALBUM):
            parseResponseCreateAlbum(buffer);
            break;
        case (Private::SMUG_ADDPHOTO):
            parseResponseAddPhoto(buffer);
            break;
        case (Private::SMUG_GETPHOTO):
            // all we get is data of the image
            emit signalBusy(false);
            emit signalGetPhotoDone(0, QString(), buffer);
            break;
        default: // Private::SMUG_LOGIN
            break;
    }

    reply->deleteLater();
}

void SmugTalker::parseResponseLogin(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseLogin";
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    emit signalLoginProgress(3);

    if (err.error != QJsonParseError::NoError)
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "failed to parse to json";
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "errCode " << err.error;
        emit signalLoginDone(err.error, errorToText(err.error, err.errorString()));
        emit signalBusy(false);
        return;
    }

    QJsonObject jsonObject  = doc.object();
    QJsonObject response    = jsonObject[QLatin1String("Response")].toObject();
    QJsonObject userObject  = response[QLatin1String("User")].toObject();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "json object " << userObject;

    d->user.displayName     = userObject[QLatin1String("Name")].toString();
    d->user.nickName        = userObject[QLatin1String("NickName")].toString();
    d->user.userUri         = userObject[QLatin1String("Uri")].toString();

    QJsonObject Uris        = userObject[QLatin1String("Uris")].toObject();
    QJsonObject node        = Uris[QLatin1String("Node")].toObject();
    QJsonObject folder      = Uris[QLatin1String("Folder")].toObject();

    d->user.nodeUri         = node[QLatin1String("Uri")].toString();
    d->user.folderUri       = folder[QLatin1String("Uri")].toString();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "json data parse : " << d->user.displayName << "+ " << d->user.nodeUri;

    emit signalLoginProgress(4);
    emit signalBusy(false);
    emit signalLoginDone(0, QString());
}

/**
 * Not necessary anymore
 *
void SmugTalker::parseResponseLogout(const QByteArray& data)
{
    int errCode = -1;
    QString errMsg;

    QDomDocument doc(QLatin1String("logout"));

    if (!doc.setContent(data))
        return;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Parse Logout response:" << endl << data;

    QDomElement e = doc.documentElement();

    for (QDomNode node = e.firstChild(); !node.isNull(); node = node.nextSibling())
    {
        if (!node.isElement())
            continue;

        e = node.toElement();

        if (e.tagName() == QLatin1String("Logout"))
        {
            errCode = 0;
        }
        else if (e.tagName() == QLatin1String("err"))
        {
            errCode = e.attribute(QLatin1String("code")).toInt();
            errMsg  = e.attribute(QLatin1String("msg"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error:" << errCode << errMsg;
        }
    }

    // consider we are logged out in any case
    d->sessionID.clear();
    d->user.clear();

    emit signalBusy(false);
}
*/

void SmugTalker::parseResponseAddPhoto(const QByteArray& data)
{
    // A multi-part put response (which we get now) looks like:
    // <?xml version="1.0" encoding="utf-8"?>
    // <rsp stat="ok">
    //   <method>smugmug.images.upload</method>
    //   <ImageID>884775096</ImageID>
    //   <ImageKey>L7aq5</ImageKey>
    //   <ImageURL>http://froody.smugmug.com/Other/Test/12372176_y7yNq#884775096_L7aq5</ImageURL>
    // </rsp>

    // A simple put response (which we used to get) looks like:
    // <?xml version="1.0" encoding="utf-8"?>
    // <rsp stat="ok">
    //   <method>smugmug.images.upload</method>
    //   <Image id="884790545" Key="seeQa" URL="http://froody.smugmug.com/Other/Test/12372176_y7yNq#884790545_seeQa"/>
    // </rsp>

    // Since all we care about is success or not, we can just check the rsp
    // stat.

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseAddPhoto";

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "json doc " << doc;

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        emit signalAddPhotoDone(err.error, errorToText(err.error, err.errorString()));
        return;
    }

    emit signalBusy(false);
    emit signalAddPhotoDone(err.error, errorToText(err.error, err.errorString()));
}

void SmugTalker::parseResponseCreateAlbum(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseCreateAlbum";

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        emit signalCreateAlbumDone(err.error, err.errorString(), 0, QString());
        return;
    }

    QJsonObject jsonObject  = doc.object();
    QJsonObject response    = jsonObject[QLatin1String("Response")].toObject();
    QJsonObject album       = response[QLatin1String("Album")].toObject();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "json data : " << jsonObject;

    QString newAlbumKey     = album[QLatin1String("AlbumKey")].toString();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "newAlbumKey " << newAlbumKey;

    emit signalBusy(false);
    emit signalCreateAlbumDone(0, errorToText(0, QString()), 0, newAlbumKey);
}

void SmugTalker::parseResponseListAlbums(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        emit signalListAlbumsDone(err.error,i18n("Failed to list albums"), QList<SmugAlbum>());
        return;
    }

    QJsonObject jsonObject  = doc.object();
    QJsonObject response    = jsonObject[QLatin1String("Response")].toObject();
    QJsonArray jsonArray    = response[QLatin1String("Album")].toArray();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseListAlbum : " << jsonObject;

    QList<SmugAlbum> albumList;

    foreach (const QJsonValue& value, jsonArray)
    {
        QJsonObject obj = value.toObject();

        SmugAlbum album;

        album.nodeID        = obj[QLatin1String("NodeID")].toString();
        album.name          = obj[QLatin1String("Name")].toString();
        album.key           = obj[QLatin1String("AlbumKey")].toString();
        album.title         = obj[QLatin1String("Title")].toString();
        album.description   = obj[QLatin1String("Description")].toString();
        album.keywords      = obj[QLatin1String("Keywords")].toString();
        album.canShare      = obj[QLatin1String("CanShare")].toBool();
        album.passwordHint  = obj[QLatin1String("PasswordHint")].toString();
        album.imageCount    = obj[QLatin1String("ImageCount")].toInt();

        albumList.append(album);

        QStringList albumParams;
        albumParams << album.nodeID
                    << album.name
                    << album.key
                    << album.title
                    << album.description
                    << album.keywords
                    << QString::number(album.canShare)
                    << album.passwordHint
                    << QString::number(album.imageCount);
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "album " << albumParams.join(QLatin1String(","));
    }

    std::sort(albumList.begin(), albumList.end(), SmugAlbum::lessThan);

    emit signalBusy(false);
    emit signalListAlbumsDone(err.error, errorToText(err.error, err.errorString()), albumList);
}

void SmugTalker::parseResponseListPhotos(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseListPhotos";

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        emit signalListPhotosDone(err.error, errorToText(err.error, err.errorString()), QList<SmugPhoto>());
        return;
    }

    QJsonObject jsonObject  = doc.object();
    QJsonObject response    = jsonObject[QLatin1String("Response")].toObject();
    QJsonArray jsonArray    = response[QLatin1String("AlbumImage")].toArray();

    QList<SmugPhoto> photosList;

    foreach (const QJsonValue& value, jsonArray)
    {
        QJsonObject obj = value.toObject();

        SmugPhoto photo;
        photo.key           = obj[QLatin1String("ImageKey")].toString();
        photo.caption       = obj[QLatin1String("Caption")].toString();
        photo.keywords      = obj[QLatin1String("Keywords")].toString();
        photo.thumbURL      = obj[QLatin1String("ThumbnailUrl")].toString();
        photo.originalURL   = obj[QLatin1String("ArchivedUri")].toString();

        photosList.append(photo);

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "photo key: "   << photo.key
                                         << ", captions: "  << photo.caption
                                         << ", keywords: "  << photo.keywords
                                         << ", ThumbnailUrl " << photo.thumbURL
                                         << ", originalURL "  << photo.originalURL;

    }

    emit signalBusy(false);
    emit signalListPhotosDone(0, QString(), photosList);
}

void SmugTalker::parseResponseListAlbumTmpl(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "ParseResponseListAlbumTmpl";

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        emit signalListAlbumTmplDone(err.error,i18n("Failed to list album template"), QList<SmugAlbumTmpl>());
        return;
    }

    QJsonObject jsonObject  = doc.object();
    QJsonObject response    = jsonObject[QLatin1String("Response")].toObject();
    QJsonArray jsonArray    = response[QLatin1String("AlbumTemplate")].toArray();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "listAlbumTmpl = " << jsonArray;

    QList<SmugAlbumTmpl> albumTmplList;

    foreach (const QJsonValue &value, jsonArray)
    {
        QJsonObject obj = value.toObject();

        SmugAlbumTmpl albumTmpl;
        albumTmpl.name          = obj[QLatin1String("Name")].toString();
        albumTmpl.uri           = obj[QLatin1String("Uri")].toString();
        albumTmpl.isPublic      = obj[QLatin1String("Public")].toBool();
        albumTmpl.password      = obj[QLatin1String("Password")].toString();
        albumTmpl.passwordHint  = obj[QLatin1String("PasswordHint")].toString();

        albumTmplList.append(albumTmpl);

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "albumTmpl : name " << albumTmpl.name
                                            << ", uri : " << albumTmpl.uri
                                            << ", isPublic " << albumTmpl.isPublic
                                            << ", password " << albumTmpl.password
                                            << ", passwordHint " << albumTmpl.passwordHint;
    }

    emit signalBusy(false);
    emit signalListAlbumTmplDone(0, errorToText(0, QString()), albumTmplList);
}

/**
 * Categories are deprecated in API v2
 *
void SmugTalker::parseResponseListCategories(const QByteArray& data)
{
    int errCode = -1;
    QString errMsg;
    QDomDocument doc(QLatin1String("categories.get"));

    if (!doc.setContent(data))
        return;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Parse Categories response:" << endl << data;

    QList <SmugCategory> categoriesList;
    QDomElement e = doc.documentElement();

    for (QDomNode node = e.firstChild(); !node.isNull(); node = node.nextSibling())
    {
        if (!node.isElement())
            continue;

        e = node.toElement();

        if (e.tagName() == QLatin1String("Categories"))
        {
            for (QDomNode nodeC = e.firstChild(); !nodeC.isNull(); nodeC = nodeC.nextSibling())
            {
                if (!nodeC.isElement())
                    continue;

                QDomElement e = nodeC.toElement();

                if (e.tagName() == QLatin1String("Category"))
                {
                    SmugCategory category;
                    category.id   = e.attribute(QLatin1String("id")).toLongLong();
                    category.name = htmlToText(e.attribute(QLatin1String("Name")));
                    categoriesList.append(category);
                }
            }

            errCode = 0;
        }
        else if (e.tagName() == QLatin1String("err"))
        {
            errCode = e.attribute(QLatin1String("code")).toInt();
            errMsg  = e.attribute(QLatin1String("msg"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error:" << errCode << errMsg;
        }
    }

    if (errCode == 15)  // 15: empty list
        errCode = 0;

    emit signalBusy(false);
    emit signalListCategoriesDone(errCode, errorToText(errCode, errMsg), categoriesList);
}

void SmugTalker::parseResponseListSubCategories(const QByteArray& data)
{
    int errCode = -1;
    QString errMsg;
    QDomDocument doc(QLatin1String("subcategories.get"));

    if (!doc.setContent(data))
        return;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Parse SubCategories response:" << endl << data;

    QList <SmugCategory> categoriesList;
    QDomElement e = doc.documentElement();

    for (QDomNode node = e.firstChild(); !node.isNull(); node = node.nextSibling())
    {
        if (!node.isElement())
            continue;

        e = node.toElement();

        if (e.tagName() == QLatin1String("SubCategories"))
        {
            for (QDomNode nodeC = e.firstChild(); !nodeC.isNull(); nodeC = nodeC.nextSibling())
            {
                if (!nodeC.isElement())
                    continue;

                e = nodeC.toElement();

                if (e.tagName() == QLatin1String("SubCategory"))
                {
                    SmugCategory category;
                    category.id   = e.attribute(QLatin1String("id")).toLongLong();
                    category.name = htmlToText(e.attribute(QLatin1String("Name")));
                    categoriesList.append(category);
                }
            }

            errCode = 0;
        }
        else if (e.tagName() == QLatin1String("err"))
        {
            errCode = e.attribute(QLatin1String("code")).toInt();
            errMsg  = e.attribute(QLatin1String("msg"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error:" << errCode << errMsg;
        }
    }

    if (errCode == 15)  // 15: empty list
        errCode = 0;

    emit signalBusy(false);
    emit signalListSubCategoriesDone(errCode, errorToText(errCode, errMsg), categoriesList);
}
*/

QString SmugTalker::htmlToText(const QString& htmlText) const
{
    QTextDocument txtDoc;
    txtDoc.setHtml(htmlText);
    return txtDoc.toPlainText();
}

} // namespace DigikamGenericSmugPlugin

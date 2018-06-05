/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-01
 * Description : a tool to export images to Smugmug web service
 *
 * Copyright (C) 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// O2 includes
#include "wstoolutils.h"
#include "o0settingsstore.h"
#include "o1requestor.h"
#include "o0globals.h"

namespace Digikam
{

class SmugTalker::Private
{

public:

    enum State
    {
        SMUG_LOGIN = 0,
        SMUG_LOGOUT,
        SMUG_LISTALBUMS,
        SMUG_LISTPHOTOS,
        SMUG_LISTALBUMTEMPLATES,
        SMUG_LISTCATEGORIES,
        SMUG_LISTSUBCATEGORIES,
        SMUG_CREATEALBUM,
        SMUG_ADDPHOTO,
        SMUG_GETPHOTO
    };

public:

    explicit Private()
    {
        parent     = 0;

        // FIXME ?
        //userAgent  = QString::fromLatin1("KIPI-Plugin-Smug/%1 (lure@kubuntu.org)").arg(kipipluginsVersion());
        userAgent       = QString::fromLatin1("digiKam/%1 (digikamdeveloper@gmail.com)").arg(digiKamVersion());

        apiVersion      = QLatin1String("v2");
        apiURL          = QString::fromLatin1("https://api.smugmug.com%1");
        currentUserUrl  = apiURL.arg("/api/v2!authuser");
        requestTokenUrl = QLatin1String("https://api.smugmug.com/services/oauth/1.0a/getRequestToken");
        authUrl         = QLatin1String("https://api.smugmug.com/services/oauth/1.0a/authorize");
        accessTokenUrl  = QLatin1String("https://api.smugmug.com/services/oauth/1.0a/getAccessToken");
        
//         apikey          = QLatin1String("66NGWpNDWmnsW6qZKLp6hNMTDZ9C24pN");
//         clientSecret    = QLatin1String("GbtsCvH3GMGnQ6Lf4XmGXwMQs2pm5SpSvVJdPsQDpHMRbsPQSWqzhxXKRRXhMwP5");
        apikey          = QString::fromLatin1("P3GR322MB4rf3dZRxDZNFv8cbK6sLPdV");
        clientSecret    = QString::fromLatin1("trJrZT3pHQRpZB8Z3LMGCL39g9q7nWJPBzZTQSWhzCnmTmtqqW5xxXdBn6fVhM3p");
        iface           = 0;
        netMngr         = 0;
        reply           = 0;
        state           = SMUG_LOGOUT;
        
        // TODO: Port to O2
        requestor       = 0;
        o1              = 0;
        settings        = 0;
    }

public:

    QWidget*               parent;

    QByteArray             buffer;

    QString                userAgent;
    QString                apiURL;
    QString                currentUserUrl;
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
    
    // TODO: Port Smugmug to O2
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
    
            //TODO: Init O1Smugmug
            d->o1 = new O1SmugMug(this, d->netMngr);
           
            // Config for authentication flow
            d->o1->setRequestTokenUrl(d->requestTokenUrl);
            d->o1->setAuthorizeUrl(d->authUrl);
            d->o1->setAccessTokenUrl(d->accessTokenUrl);
            d->o1->setLocalPort(8000);
            
            // Application credentials
            d->o1->setClientId(d->apikey);
            d->o1->setClientSecret(d->clientSecret);
            // Set userAgent to work around error :
            // O1::onTokenRequestError: 201 "Error transferring requestTokenUrl() - server replied: Forbidden" "Bad bot"
            d->o1->setUserAgent(d->userAgent.toUtf8());
                       
            // Setting to store oauth config
            d->settings = WSToolUtils::getOauthSettings(this);
            O0SettingsStore* const store   = new O0SettingsStore(d->settings, QLatin1String(O2_ENCRYPTION_KEY), this);
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
    if (loggedIn())
    {
        logout();

        while (d->reply && d->reply->isRunning())
        {
            qApp->processEvents();
        }
    }
    
    // Just for test
    unlink();

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
            
            removeUserName(d->user.displayName);
        }
        
        void SmugTalker::removeUserName(const QString& userName)
        {
//             if (userName.startsWith(d->serviceName))
//             {
//                 d->settings->beginGroup(userName);
//                 d->settings->remove(QString());
//                 d->settings->endGroup();
//             }
        }

        bool SmugTalker::loggedIn() const
        {
            //return !d->sessionID.isEmpty();
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

QString SmugTalker::firstLetterToUpperCase(const QString& word)
{
    QString w(word);
    
    // First we remove space at beginning and end
    w = w.trimmed();
    
    // Then we replace first letter with its uppercase
    w.replace(0, 1, w[0].toUpper());

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << w;
    
    return w;
}

QString SmugTalker::createUrlName(const QString& name)
{
    QString n(name);
    
    // First we upper first letter
    n = firstLetterToUpperCase(n);
    
    // Then we replace space with "-"
    QStringList words = n.split(" ");
    n = words.join("-");

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url name : " << n;
    
    return n;
    
}

void SmugTalker::cancel()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(false);
}

//TODO: Port to O2 here
        void SmugTalker::loginWithNickName(const QString& nickName)
        {
            if (d->reply)
            {
                d->reply->abort();
                d->reply = 0;
            }
            
            emit signalBusy(true);
            emit signalLoginProgress(1, 4, i18n("Logging in to SmugMug service..."));
            
            // Build authentication url
            O1SmugMug::AuthorizationUrlBuilder builder;
            builder.setAccess(O1SmugMug::AccessFull);
            builder.setPermissions(O1SmugMug::PermissionsModify);
            builder.setPrepopulatedUsername(nickName);
            d->o1->initAuthorizationUrl(builder);
            
            if(!d->o1->linked())
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
            QUrl url(d->currentUserUrl);
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url = " << url.url();
            
            QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();
            
            QNetworkRequest netRequest(url);
            netRequest.setRawHeader("Accept", "application/json");
            netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
            netRequest.setHeader(QNetworkRequest::UserAgentHeader,   d->userAgent);
            
            d->reply = d->requestor->get(netRequest, reqParams);
            
            d->state = Private::SMUG_LOGIN;
            d->buffer.resize(0);
        }
        
        void SmugTalker::login(const QString& email, const QString& password)
        {
            if (d->reply)
            {
                d->reply->abort();
                d->reply = 0;
            }
            
            emit signalBusy(true);
            emit signalLoginProgress(1, 4, i18n("Logging in to SmugMug service..."));
            
            QUrl url(d->apiURL);
            QUrlQuery q;
            
            if (email.isEmpty())
            {
                q.addQueryItem(QString::fromLatin1("method"),       QString::fromLatin1("smugmug.login.anonymously"));
                q.addQueryItem(QString::fromLatin1("APIKey"),       d->apikey);
            }
            else
            {
                q.addQueryItem(QString::fromLatin1("method"),       QString::fromLatin1("smugmug.login.withPassword"));
                q.addQueryItem(QString::fromLatin1("APIKey"),       d->apikey);
                q.addQueryItem(QString::fromLatin1("EmailAddress"), email);
                q.addQueryItem(QString::fromLatin1("Password"),     password);
            }
            
            url.setQuery(q);
            
            QNetworkRequest netRequest(url);
            netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
            netRequest.setHeader(QNetworkRequest::UserAgentHeader,   d->userAgent);
            
            d->reply = d->netMngr->get(netRequest);
            
            d->state = Private::SMUG_LOGIN;
            d->buffer.resize(0);
            
            d->user.email = email;
        }

        void SmugTalker::logout()
        {
            if (d->reply)
            {
                d->reply->abort();
                d->reply = 0;
            }

            emit signalBusy(true);

            QUrl url(d->apiURL);
            QUrlQuery q;
            q.addQueryItem(QString::fromLatin1("method"),    QString::fromLatin1("smugmug.logout"));
            q.addQueryItem(QString::fromLatin1("SessionID"), d->sessionID);
            url.setQuery(q);

            QNetworkRequest netRequest(url);
            netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
            netRequest.setHeader(QNetworkRequest::UserAgentHeader,   d->userAgent);

            d->reply = d->netMngr->get(netRequest);

            d->state = Private::SMUG_LOGOUT;
            d->buffer.resize(0);
        }

        void SmugTalker::listAlbums(const QString& nickName)
        {
            if (d->reply)
            {
                d->reply->abort();
                d->reply = 0;
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
            d->buffer.resize(0);
        }

void SmugTalker::listPhotos(const qint64 albumID,
                            const QString& albumKey,
                            const QString& albumPassword,
                            const QString& sitePassword)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);

    QUrl url(d->apiURL);
    QUrlQuery q;
    q.addQueryItem(QString::fromLatin1("method"),    QString::fromLatin1("smugmug.images.get"));
//     q.addQueryItem(QString::fromLatin1("SessionID"), d->sessionID);
    q.addQueryItem(QString::fromLatin1("APIKey"), d->apikey);
    q.addQueryItem(QString::fromLatin1("AlbumID"),   QString::number(albumID));
    q.addQueryItem(QString::fromLatin1("AlbumKey"),  albumKey);
    q.addQueryItem(QString::fromLatin1("Heavy"),     QString::fromLatin1("1"));

    if (!albumPassword.isEmpty())
        q.addQueryItem(QString::fromLatin1("Password"),     albumPassword);

    if (!sitePassword.isEmpty())
        q.addQueryItem(QString::fromLatin1("SitePassword"), sitePassword);

    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader,   d->userAgent);

    d->reply = d->netMngr->get(netRequest);

    d->state = Private::SMUG_LISTPHOTOS;
    d->buffer.resize(0);
}

        void SmugTalker::listAlbumTmpl()
        {
            if (d->reply)
            {
                d->reply->abort();
                d->reply = 0;
            }

            emit signalBusy(true);

//             QUrl url(d->apiURL.arg(QString::fromLatin1("%1!albumtemplates").arg(d->user.userUri)));
            QUrl url(d->apiURL.arg(QString::fromLatin1("%1!albumtemplates").arg(d->user.userUri)));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url to listAlbumTmpl " << url.url();
            
            QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();
            
            QNetworkRequest netRequest(url);
            netRequest.setRawHeader("Accept", "application/json");
            netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
            netRequest.setHeader(QNetworkRequest::UserAgentHeader,   d->userAgent);
            
            d->reply = d->requestor->get(netRequest, reqParams);
            
            d->state = Private::SMUG_LISTALBUMTEMPLATES;
            d->buffer.resize(0);
        }

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
    q.addQueryItem(QString::fromLatin1("method"),    QString::fromLatin1("smugmug.categories.get"));
    q.addQueryItem(QString::fromLatin1("SessionID"), d->sessionID);
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader,   d->userAgent);

    d->reply = d->netMngr->get(netRequest);

    d->state = Private::SMUG_LISTCATEGORIES;
    d->buffer.resize(0);
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
    q.addQueryItem(QString::fromLatin1("method"),     QString::fromLatin1("smugmug.subcategories.get"));
    q.addQueryItem(QString::fromLatin1("SessionID"),  d->sessionID);
    q.addQueryItem(QString::fromLatin1("CategoryID"), QString::number(categoryID));
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader,   d->userAgent);

    d->reply = d->netMngr->get(netRequest);

    d->state = Private::SMUG_LISTSUBCATEGORIES;
    d->buffer.resize(0);
}

        void SmugTalker::createAlbum(const SmugAlbum& album)
        {
            if (d->reply)
            {
                d->reply->abort();
                d->reply = 0;
            }

            emit signalBusy(true);
                       
            QUrl url(d->apiURL.arg(QString::fromLatin1("%1/!albums").arg(d->user.folderUri)));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url to post " << url.url();
            
            QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();
            
            /**
             * Something must to be remembereed here is that we HAVE TO start a name with upper case !!!
             * And url name with '-' instead of space
             */
            QByteArray data;
            data += "{\"Name\": \"";
            data += firstLetterToUpperCase(album.title).toUtf8();
            data += "\",\"UrlName\":\"";
            data += createUrlName(album.title).toUtf8();
            data += "\",\"Privacy\":\"Public\"}";
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << QString(data);
            
            QNetworkRequest netRequest(url);
            netRequest.setRawHeader("Accept", "application/json");
            netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
            netRequest.setHeader(QNetworkRequest::UserAgentHeader,   d->userAgent);
            
            d->reply = d->requestor->post(netRequest, reqParams, data);

            d->state = Private::SMUG_CREATEALBUM;
            d->buffer.resize(0);
        }

bool SmugTalker::addPhoto(const  QString& imgPath,
                          qint64 albumID,
                          const  QString& albumKey,
                          const  QString& caption)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
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

    long long imgSize  = imgFile.size();
    QByteArray imgData = imgFile.readAll();
    imgFile.close();

    SmugMPForm form;

    form.addPair(QString::fromLatin1("ByteCount"),    QString::number(imgSize));
    form.addPair(QString::fromLatin1("MD5Sum"),       QString::fromLatin1(
        QCryptographicHash::hash(imgData, QCryptographicHash::Md5).toHex()));
    form.addPair(QString::fromLatin1("AlbumID"),      QString::number(albumID));
    form.addPair(QString::fromLatin1("AlbumKey"),     albumKey);
    form.addPair(QString::fromLatin1("ResponseType"), QString::fromLatin1("REST"));

    if (!caption.isEmpty())
        form.addPair(QString::fromLatin1("Caption"), caption);

    if (!form.addFile(imgName, imgPath))
        return false;

    form.finish();

    QString customHdr;
    QUrl url(QString::fromLatin1("http://upload.smugmug.com/photos/xmladd.mg"));

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());
    netRequest.setHeader(QNetworkRequest::UserAgentHeader,   d->userAgent);
    netRequest.setRawHeader("X-Smug-SessionID", d->sessionID.toLatin1());
    netRequest.setRawHeader("X-Smug-Version",   d->apiVersion.toLatin1());

    d->reply = d->netMngr->post(netRequest, form.formData());

    d->state = Private::SMUG_ADDPHOTO;
    d->buffer.resize(0);
    return true;
}

void SmugTalker::getPhoto(const QString& imgPath)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);

    QNetworkRequest netRequest(QUrl::fromLocalFile(imgPath));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader, d->userAgent);
    netRequest.setRawHeader("X-Smug-SessionID", d->sessionID.toLatin1());
    netRequest.setRawHeader("X-Smug-Version",   d->apiVersion.toLatin1());

    d->reply = d->netMngr->get(netRequest);

    d->state = Private::SMUG_GETPHOTO;
    d->buffer.resize(0);
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

//TODO: Ported to O2 here
        void SmugTalker::slotFinished(QNetworkReply* reply)
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "error code : " << reply->error() << "error text " << reply->errorString();
            if (reply != d->reply)
            {
                return;
            }

            d->reply = 0;

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

            d->buffer.append(reply->readAll());

            switch(d->state)
            {
                case (Private::SMUG_LOGIN):
                    parseResponseLogin(d->buffer);
                    break;
                case (Private::SMUG_LOGOUT):
                    parseResponseLogout(d->buffer);
                    break;
                case (Private::SMUG_LISTALBUMS):
                    parseResponseListAlbums(d->buffer);
                    break;
                case (Private::SMUG_LISTPHOTOS):
                    parseResponseListPhotos(d->buffer);
                    break;
                case (Private::SMUG_LISTALBUMTEMPLATES):
                    parseResponseListAlbumTmpl(d->buffer);
                    break;
                case (Private::SMUG_LISTCATEGORIES):
                    parseResponseListCategories(d->buffer);
                    break;
                case (Private::SMUG_LISTSUBCATEGORIES):
                    parseResponseListSubCategories(d->buffer);
                    break;
                case (Private::SMUG_CREATEALBUM):
                    parseResponseCreateAlbum(d->buffer);
                    break;
                case (Private::SMUG_ADDPHOTO):
                    parseResponseAddPhoto(d->buffer);
                    break;
                case (Private::SMUG_GETPHOTO):
                    // all we get is data of the image
                    emit signalBusy(false);
                    emit signalGetPhotoDone(0, QString(), d->buffer);
                    break;
            }

            reply->deleteLater();
        }

        void SmugTalker::parseResponseLogin(const QByteArray& data)
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseReponseLogin";
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(data, &err);
            
//             qCDebug(DIGIKAM_WEBSERVICES_LOG) << "json data " << doc;
            
            emit signalLoginProgress(3);
            
            if (err.error != QJsonParseError::NoError)
            {
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "failed to parse to json";
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "errCode " << err.error;
                emit signalLoginDone(err.error, errorToText(err.error, err.errorString()));
                emit signalBusy(false);
                return;
            }
            
            QJsonObject userObject  = doc[QLatin1String("Response")][QLatin1String("User")].toObject();
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "json object " << userObject; 
            
            d->user.displayName     = userObject[QLatin1String("Name")].toString();
            d->user.nickName        = userObject[QLatin1String("NickName")].toString();
            d->user.userUri         = userObject[QLatin1String("Uri")].toString();
            
            QJsonObject Uris        = userObject[QLatin1String("Uris")].toObject();
            d->user.nodeUri         = QJsonValue(Uris[QLatin1String("Node")])[QLatin1String("Uri")].toString();
            d->user.folderUri       = QJsonValue(Uris[QLatin1String("Folder")])[QLatin1String("Uri")].toString();       
            
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "json data parse : " << d->user.displayName << "+ " << d->user.nodeUri;
            
            emit signalLoginProgress(4);
            emit signalBusy(false);
            emit signalLoginDone(0, QString(""));
        }

void SmugTalker::parseResponseLogout(const QByteArray& data)
{
    int errCode = -1;
    QString errMsg;

    QDomDocument doc(QString::fromLatin1("logout"));

    if (!doc.setContent(data))
        return;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Parse Logout response:" << endl << data;

    QDomElement e = doc.documentElement();

    for (QDomNode node = e.firstChild(); !node.isNull(); node = node.nextSibling())
    {
        if (!node.isElement())
            continue;

        e = node.toElement();

        if (e.tagName() == QString::fromLatin1("Logout"))
        {
            errCode = 0;
        }
        else if (e.tagName() == QString::fromLatin1("err"))
        {
            errCode = e.attribute(QString::fromLatin1("code")).toInt();
            errMsg  = e.attribute(QString::fromLatin1("msg"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error:" << errCode << errMsg;
        }
    }

    // consider we are logged out in any case
    d->sessionID.clear();
    d->user.clear();

    emit signalBusy(false);
}

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

    int errCode = -1;
    QString errMsg;
    QDomDocument doc(QString::fromLatin1("addphoto"));

    if (!doc.setContent(data))
        return;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Parse Add Photo response:" << endl << data;

    QDomElement document = doc.documentElement();

    if (document.tagName() == QString::fromLatin1("rsp"))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "rsp stat: " << document.attribute(QString::fromLatin1("stat"));

        if (document.attribute(QString::fromLatin1("stat")) == QString::fromLatin1("ok"))
        {
            errCode = 0;
        }
        else if (document.attribute(QString::fromLatin1("stat")) == QString::fromLatin1("fail"))
        {
            QDomElement error = document.firstChildElement(QString::fromLatin1("err"));
            errCode           = error.attribute(QString::fromLatin1("code")).toInt();
            errMsg            = error.attribute(QString::fromLatin1("msg"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "error" << errCode << ":" << errMsg << endl;
        }
    }
    else
    {
        errCode = -2;
        errMsg  = QString::fromLatin1("Malformed response from smugmug: ") + document.tagName();
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error:" << errCode << errMsg;
    }

    emit signalBusy(false);
    emit signalAddPhotoDone(errCode, errorToText(errCode, errMsg));
}

        void SmugTalker::parseResponseCreateAlbum(const QByteArray& data)
        {        
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseCreateAlbum";
            
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(data, &err);
            
            if(err.error != QJsonParseError::NoError)
            {
                emit signalBusy(false);
                emit signalCreateAlbumDone(err.error, err.errorString(),0,0);
                return;
            }
            
            QJsonObject jsonObject = doc[QLatin1String("Response")][QLatin1String("Album")].toObject();
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "json data : " << doc; 
            
            QString newAlbumKey    = jsonObject[QLatin1String("AlbumKey")].toString();

            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "newAlbumKey " << newAlbumKey;
            
            emit signalBusy(false);
            emit signalCreateAlbumDone(0, errorToText(0, QString("")), 0, newAlbumKey);
        }

        void SmugTalker::parseResponseListAlbums(const QByteArray& data)
        {
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(data, &err);
            
            if(err.error != QJsonParseError::NoError)
            {
                emit signalBusy(false);
                emit signalListAlbumsDone(err.error,i18n("Failed to list albums"), QList<SmugAlbum>());
                return;
            }
            
            QJsonObject jsonObject = doc[QLatin1String("Response")].toObject();
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseReponseListAlbum : " << jsonObject;

            QList<SmugAlbum> albumList;
            
            QJsonArray jsonArray = jsonObject[QLatin1String("Album")].toArray();
            
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
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "album " << albumParams.join(",");
            }
            
            std::sort(albumList.begin(), albumList.end(), SmugAlbum::lessThan);

            emit signalBusy(false);
            emit signalListAlbumsDone(err.error, errorToText(err.error, err.errorString()), albumList);
        }

void SmugTalker::parseResponseListPhotos(const QByteArray& data)
{
    int errCode = -1;
    QString errMsg;
    QDomDocument doc(QString::fromLatin1("images.get"));

    if (!doc.setContent(data))
        return;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Parse Photos response:" << endl << data;

    QList <SmugPhoto> photosList;
    QDomElement e = doc.documentElement();

    for (QDomNode node = e.firstChild(); !node.isNull(); node = node.nextSibling())
    {
        if (!node.isElement())
            continue;

        e = node.toElement();

        if (e.tagName() == QString::fromLatin1("Album"))
        {
            node = e.firstChild();

            if (!node.isElement())
                continue;

            e = node.toElement();
        }

        if (e.tagName() == QString::fromLatin1("Images"))
        {
            for (QDomNode nodeP = e.firstChild(); !nodeP.isNull(); nodeP = nodeP.nextSibling())
            {
                if (!nodeP.isElement())
                    continue;

                e = nodeP.toElement();

                if (e.tagName() == QString::fromLatin1("Image"))
                {
                    SmugPhoto photo;
                    photo.id       = e.attribute(QString::fromLatin1("id")).toLongLong();
                    photo.key      = e.attribute(QString::fromLatin1("Key"));
                    photo.caption  = htmlToText(e.attribute(QString::fromLatin1("Caption")));
                    photo.keywords = htmlToText(e.attribute(QString::fromLatin1("Keywords")));
                    photo.thumbURL = e.attribute(QString::fromLatin1("ThumbURL"));

                    // try to get largest size available
                    if (e.hasAttribute(QString::fromLatin1("Video1280URL")))
                        photo.originalURL = e.attribute(QString::fromLatin1("Video1280URL"));
                    else if (e.hasAttribute(QString::fromLatin1("Video960URL")))
                        photo.originalURL = e.attribute(QString::fromLatin1("Video960URL"));
                    else if (e.hasAttribute(QString::fromLatin1("Video640URL")))
                        photo.originalURL = e.attribute(QString::fromLatin1("Video640URL"));
                    else if (e.hasAttribute(QString::fromLatin1("Video320URL")))
                        photo.originalURL = e.attribute(QString::fromLatin1("Video320URL"));
                    else if (e.hasAttribute(QString::fromLatin1("OriginalURL")))
                        photo.originalURL = e.attribute(QString::fromLatin1("OriginalURL"));
                    else if (e.hasAttribute(QString::fromLatin1("X3LargeURL")))
                        photo.originalURL = e.attribute(QString::fromLatin1("X3LargeURL"));
                    else if (e.hasAttribute(QString::fromLatin1("X2LargeURL")))
                        photo.originalURL = e.attribute(QString::fromLatin1("X2LargeURL"));
                    else if (e.hasAttribute(QString::fromLatin1("XLargeURL")))
                        photo.originalURL = e.attribute(QString::fromLatin1("XLargeURL"));
                    else if (e.hasAttribute(QString::fromLatin1("LargeURL")))
                        photo.originalURL = e.attribute(QString::fromLatin1("LargeURL"));
                    else if (e.hasAttribute(QString::fromLatin1("MediumURL")))
                        photo.originalURL = e.attribute(QString::fromLatin1("MediumURL"));
                    else if (e.hasAttribute(QString::fromLatin1("SmallURL")))
                        photo.originalURL = e.attribute(QString::fromLatin1("SmallURL"));

                    photosList.append(photo);
                }
            }

            errCode = 0;
        }
        else if (e.tagName() == QString::fromLatin1("err"))
        {
            errCode = e.attribute(QString::fromLatin1("code")).toInt();
            errMsg  = e.attribute(QString::fromLatin1("msg"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error:" << errCode << errMsg;
        }
    }

    if (errCode == 15)  // 15: empty list
        errCode = 0;

    emit signalBusy(false);
    emit signalListPhotosDone(errCode, errorToText(errCode, errMsg), photosList);
}

        void SmugTalker::parseResponseListAlbumTmpl(const QByteArray& data)
        {            
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "ParseResponseListAlbumTmpl";
            
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(data, &err);
            
            if(err.error != QJsonParseError::NoError)
            {
                emit signalBusy(false);                
                emit signalListAlbumTmplDone(err.error,i18n("Failed to list album template"), QList<SmugAlbumTmpl>());
                return;
            }
            
            QJsonArray jsonArray = doc[QLatin1String("Response")][QLatin1String("AlbumTemplate")].toArray();
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "listAlbumTmpl = " << jsonArray;
            
            QList<SmugAlbumTmpl> albumTmplList;
            
            foreach(const QJsonValue &value, jsonArray)
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
            emit signalListAlbumTmplDone(0, errorToText(0, QString("")), albumTmplList);
        }

void SmugTalker::parseResponseListCategories(const QByteArray& data)
{
    int errCode = -1;
    QString errMsg;
    QDomDocument doc(QString::fromLatin1("categories.get"));

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

        if (e.tagName() == QString::fromLatin1("Categories"))
        {
            for (QDomNode nodeC = e.firstChild(); !nodeC.isNull(); nodeC = nodeC.nextSibling())
            {
                if (!nodeC.isElement())
                    continue;

                QDomElement e = nodeC.toElement();

                if (e.tagName() == QString::fromLatin1("Category"))
                {
                    SmugCategory category;
                    category.id   = e.attribute(QString::fromLatin1("id")).toLongLong();
                    category.name = htmlToText(e.attribute(QString::fromLatin1("Name")));
                    categoriesList.append(category);
                }
            }

            errCode = 0;
        }
        else if (e.tagName() == QString::fromLatin1("err"))
        {
            errCode = e.attribute(QString::fromLatin1("code")).toInt();
            errMsg  = e.attribute(QString::fromLatin1("msg"));
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
    QDomDocument doc(QString::fromLatin1("subcategories.get"));

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

        if (e.tagName() == QString::fromLatin1("SubCategories"))
        {
            for (QDomNode nodeC = e.firstChild(); !nodeC.isNull(); nodeC = nodeC.nextSibling())
            {
                if (!nodeC.isElement())
                    continue;

                e = nodeC.toElement();

                if (e.tagName() == QString::fromLatin1("SubCategory"))
                {
                    SmugCategory category;
                    category.id   = e.attribute(QString::fromLatin1("id")).toLongLong();
                    category.name = htmlToText(e.attribute(QString::fromLatin1("Name")));
                    categoriesList.append(category);
                }
            }

            errCode = 0;
        }
        else if (e.tagName() == QString::fromLatin1("err"))
        {
            errCode = e.attribute(QString::fromLatin1("code")).toInt();
            errMsg  = e.attribute(QString::fromLatin1("msg"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error:" << errCode << errMsg;
        }
    }

    if (errCode == 15)  // 15: empty list
        errCode = 0;

    emit signalBusy(false);
    emit signalListSubCategoriesDone(errCode, errorToText(errCode, errMsg), categoriesList);
}

QString SmugTalker::htmlToText(const QString& htmlText) const
{
    QTextDocument txtDoc;
    txtDoc.setHtml(htmlText);
    return txtDoc.toPlainText();
}

} // namespace Digikam

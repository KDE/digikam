/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-26
 * Description : a tool to export items to Facebook web service
 *
 * Copyright (C) 2008-2010 by Luka Renko <lure at kubuntu dot org>
 * Copyright (c) 2011      by Dirk Tilger <dirk dot kde at miriup dot de>
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

#include "fbtalker.h"

// Qt includes

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <QtAlgorithms>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QList>
#include <QDesktopServices>
#include <QApplication>
#include <QPushButton>
#include <QDialog>
#include <QDialogButtonBox>
#include <QUrlQuery>
#include <QMessageBox>

// Local includes

#include "digikam_version.h"
#include "fbmpform.h"
#include "digikam_debug.h"
#include "o0settingsstore.h"
#include "wstoolutils.h"

namespace Digikam
{

bool operator< (const FbUser& first, const FbUser& second)
{
    return first.name < second.name;
}

bool operator< (const FbAlbum& first, const FbAlbum& second)
{
    return first.title < second.title;
}

// -----------------------------------------------------------------------------

class FbTalker::Private
{

public:

    explicit Private()
    {
        apiURL          = QLatin1String("https://graph.facebook.com/%1/%2");
        authUrl         = QLatin1String("https://www.facebook.com/dialog/oauth");
        tokenUrl        = QLatin1String("https://graph.facebook.com/oauth/access_token");
        
        clientSecret    = QLatin1String("5b0b5cd096e110cd4f4c72f517e2c544");
        apikey          = QLatin1String("400589753481372");
        
        scope           = QLatin1String("user_photos,publish_pages,manage_pages"); //publish_to_groups,user_friends not necessary?
        
        dialog          = 0;
        parent          = 0;
        loginInProgress = false;

        o2              = 0;
    }

    QDialog*               dialog;
    QWidget*               parent;

    QString                apiURL;
    QString                authUrl;
    QString                tokenUrl;
    QString                clientSecret;
    QString                apikey;

    bool                   loginInProgress;

    FbUser                 user;
    
    //Ported to O2 here
    O2*                    o2;
    QString                scope;
};

// -----------------------------------------------------------------------------

FbTalker::FbTalker(QWidget* const parent)
    : WSTalker(parent),
      d(new Private)
{           
    d->parent  = parent;          
    
    //TODO: Ported to O2 here
    
    d->o2      = new O2(this);
    
    d->o2->setClientId(d->apikey);
    d->o2->setClientSecret(d->clientSecret);

    d->o2->setRequestUrl(d->authUrl);
    d->o2->setTokenUrl(d->tokenUrl);
    d->o2->setRefreshTokenUrl(d->tokenUrl);
    d->o2->setLocalhostPolicy(QLatin1String("https://www.facebook.com/connect/login_success.html"));
    d->o2->setUseExternalWebInterceptor(true);
    d->o2->setLocalPort(8000);
    d->o2->setGrantFlow(O2::GrantFlow::GrantFlowImplicit);
    d->o2->setScope(d->scope);
    
    m_store->setGroupKey(QLatin1String("Facebook"));
    d->o2->setStore(m_store);
    
    connect(d->o2, SIGNAL(linkingFailed()),
            this, SLOT(slotLinkingFailed()));                    
    connect(d->o2, SIGNAL(linkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));
    connect(d->o2, SIGNAL(openBrowser(QUrl)),
            this, SLOT(slotOpenBrowser(QUrl)));
    connect(d->o2, SIGNAL(closeBrowser()),
            this, SLOT(slotCloseBrowser()));
}

FbTalker::~FbTalker()
{
    //removeAllAccounts();
    
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "call destructor";
    saveUserAccount(d->user.name, d->user.id, d->o2->expires(), d->o2->token(), d->o2->refreshToken());
    
    delete d;
}

//TODO: Ported to O2 here
// ----------------------------------------------------------------------------------------------
void FbTalker::link()
{
    emit signalBusy(true);

    d->loginInProgress = true;    
    d->o2->link();                
}

void FbTalker::unlink()
{
    emit signalBusy(true);
    d->o2->unlink();
}

void FbTalker::slotResponseTokenReceived(const QMap<QString, QString>& rep)
{
    d->o2->onVerificationReceived(rep);
}

bool FbTalker::linked() const
{
    return d->o2->linked();
}

void FbTalker::resetTalker(const QString& expire, const QString& accessToken, const QString& refreshToken)
{
    m_store->setValue(QString(O2_KEY_EXPIRES).arg(d->apikey), expire);
    m_store->setValue(QString(O2_KEY_LINKED).arg(d->apikey), "1");
    m_store->setValue(QString(O2_KEY_REFRESH_TOKEN).arg(d->apikey), refreshToken);
    m_store->setValue(QString(O2_KEY_TOKEN).arg(d->apikey), accessToken);
}

FbUser FbTalker::getUser() const
{
    return d->user;
}

void FbTalker::authenticate()
{
    d->loginInProgress = true;

    emit signalLoginProgress(2, 9, i18n("Validate previous session..."));
    
    WSTalker::authenticate();
}

void FbTalker::getLoggedInUser()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "getLoggedInUser called ";
    
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);
    emit signalLoginProgress(3);
    
    QUrl url(d->apiURL.arg("me")
                        .arg(""));
    QUrlQuery q;
    q.addQueryItem(QLatin1String("fields"), QLatin1String("id,name,link"));
    q.addQueryItem(QLatin1String("access_token"), d->o2->token().toUtf8());
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                        QLatin1String("application/x-www-form-urlencoded"));

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url = " << netRequest.url();
    m_reply = m_netMngr->get(netRequest);
    
    m_state = WSTalker::GETUSER;
    m_buffer.resize(0);
}
// ----------------------------------------------------------------------------------------------

/** Compute MD5 signature using url queries keys and values:
 *  http://wiki.developers.facebook.com/index.php/How_Facebook_Authenticates_Your_Application
 *  This method was used for the legacy authentication scheme and has been obsoleted with OAuth2 authentication.
 */
/*
 Q S*tring FbTalker::getApiSig(const QMap<QString, QString>& args)
 {
 QString concat;
 // NOTE: QMap iterator will sort alphabetically
 
 for (QMap<QString, QString>::const_iterator it = args.constBegin();
 it != args.constEnd();
 ++it)
 {
 concat.append(it.key());
 concat.append("=");
 concat.append(it.value());
}

if (args["session_key"].isEmpty())
    concat.append(d->clientSecret);
else
    concat.append(d->sessionSecret);

KMD5 md5(concat.toUtf8());
return md5.hexDigest().data();
}
*/


void FbTalker::logout()
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    QMap<QString, QString> args;
    args[QString::fromLatin1("next")]         = QString::fromLatin1("http://www.digikam.org");
    args[QString::fromLatin1("access_token")] = d->o2->token().toUtf8();

    QUrl url(QString::fromLatin1("https://www.facebook.com/logout.php"));
    QUrlQuery q;
    q.addQueryItem(QString::fromLatin1("next"), QString::fromLatin1("http://www.digikam.org"));
    q.addQueryItem(QString::fromLatin1("access_token"), d->o2->token().toUtf8());
    url.setQuery(q);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Logout URL: " << url;
    QDesktopServices::openUrl(url);

    emit signalBusy(false);
}


//TODO: Ported to O2
//----------------------------------------------------------------------------------------------------
void FbTalker::listAlbums(long long userID)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Requesting albums for user " << userID;

    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);
    
    QUrl url;
    
    /*
     * If userID is specified, load albums of that user,
     * else load albums of current user
     */
    if(!userID)
    {
        url = QUrl(d->apiURL.arg(d->user.id)
                            .arg("albums"));
    }
    else
    {
        url = QUrl(d->apiURL.arg(userID)
                            .arg("albums"));
    }

    QUrlQuery q;
    q.addQueryItem(QLatin1String("fields"),
                    QLatin1String("id,name,description,privacy,link,location"));
    q.addQueryItem(QLatin1String("access_token"), d->o2->token().toUtf8());
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                            QLatin1String("application/x-www-form-urlencoded"));

    m_reply = m_netMngr->get(netRequest);
    
    m_state = WSTalker::LISTALBUMS;
    m_buffer.resize(0);
}

void FbTalker::createAlbum(const FbAlbum& album)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);

    QUrlQuery params;            
    params.addQueryItem("access_token", d->o2->token().toUtf8());
    params.addQueryItem("name", album.title);
    
    if (!album.location.isEmpty())
        params.addQueryItem("location", album.location);
    /*
     * description is deprecated and now a param of message
     */
    if (!album.description.isEmpty())
        params.addQueryItem("message", album.description);

    // TODO (Dirk): Wasn't that a requested feature in Bugzilla?
    switch (album.privacy)
    {
        case FB_ME:
            params.addQueryItem("privacy","{'value':'SELF'}");
            break;
        case FB_FRIENDS:
            params.addQueryItem("privacy","{'value':'ALL_FRIENDS'}");
            break;
        case FB_FRIENDS_OF_FRIENDS:
            params.addQueryItem("privacy","{'value':'FRIENDS_OF_FRIENDS'}");
            break;
        case FB_EVERYONE:
            params.addQueryItem("privacy","{'value':'EVERYONE'}");
            break;
        case FB_CUSTOM:
            //TODO
            params.addQueryItem("privacy","{'value':'CUSTOM'}");
            break;
    }
    
    QUrl url(QUrl(d->apiURL.arg(d->user.id)
                            .arg("albums")));
//     url.setQuery(params);
    
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, 
                        QLatin1String("application/x-www-form-urlencoded"));

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url to create new album " << netRequest.url() << params.query(); 
    
    m_reply = m_netMngr->post(netRequest, params.query().toUtf8());
    
    m_state = WSTalker::CREATEALBUM;
    m_buffer.resize(0);
}

bool FbTalker::addPhoto(const QString& imgPath, const QString& albumID, const QString& caption)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Adding photo " << imgPath << " to album with id "
                                    << albumID << " using caption '" << caption << "'";

    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);

    QMap<QString, QString> args;
    args[QString::fromLatin1("access_token")] = d->o2->token().toUtf8();

    if (!caption.isEmpty())
        args[QString::fromLatin1("message")]  = caption;

    FbMPForm form;

    for (QMap<QString, QString>::const_iterator it = args.constBegin();
        it != args.constEnd();
        ++it)
    {
        form.addPair(it.key(), it.value());
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "FORM: " << endl << form.formData();
    
    if (!form.addFile(QUrl::fromLocalFile(imgPath).fileName(), imgPath))
    {
        emit signalBusy(false);
        return false;
    }

    form.finish();

    QVariant arg_1;
    if(albumID.isEmpty())
    {
        arg_1 = d->user.id;
    }
    else
    {
        arg_1 = albumID;
    }


    QNetworkRequest netRequest(QUrl(d->apiURL.arg(arg_1.toString())
                                                .arg("photos")));
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());

    m_reply = m_netMngr->post(netRequest, form.formData());
    
    m_state = WSTalker::ADDPHOTO;
    m_buffer.resize(0);

    return true;
}
//----------------------------------------------------------------------------------------------------

QString FbTalker::errorToText(int errCode, const QString &errMsg)
{
    QString transError;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "errorToText: " << errCode << ": " << errMsg;

    switch (errCode)
    {
        case 0:
            transError = QString::fromLatin1("");
            break;
        case 2:
            transError = i18n("The service is not available at this time.");
            break;
        case 4:
            transError = i18n("The application has reached the maximum number of requests allowed.");
            break;
        case 102:
            transError = i18n("Invalid session key or session expired. Try to log in again.");
            break;
        case 120:
            transError = i18n("Invalid album ID.");
            break;
        case 321:
            transError = i18n("Album is full.");
            break;
        case 324:
            transError = i18n("Missing or invalid file.");
            break;
        case 325:
            transError = i18n("Too many unapproved photos pending.");
            break;
        default:
            transError = errMsg;
            break;
    }

    return transError;
}

/*
 * (Trung) This has to be adapted in slotFinished in WSTalker
 * 
void FbTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != m_reply)
    {
        return;
    }

    m_reply = 0;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (d->loginInProgress)
        {
            authenticationDone(reply->error(), reply->errorString());
        }
        else if (d->state == Private::FB_ADDPHOTO)
        {
            emit signalBusy(false);
            emit signalAddPhotoDone(reply->error(), reply->errorString());
        }
        else
        {
            emit signalBusy(false);
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18n("Error"), reply->errorString());
        }
        
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << reply->error() << " text :"<< QString(reply->readAll());

        reply->deleteLater();
        return;
    }

    m_buffer.append(reply->readAll());

    switch(d->state)
    {
        case (Private::FB_GETLOGGEDINUSER):
            parseResponseGetLoggedInUser(m_buffer);
            break;
        case (Private::FB_LISTALBUMS):
            parseResponseListAlbums(m_buffer);
            break;
        case (Private::FB_CREATEALBUM):
            parseResponseCreateAlbum(m_buffer);
            break;
        case (Private::FB_ADDPHOTO):
            parseResponseAddPhoto(m_buffer);
            break;
    }

    reply->deleteLater();
}
*/

void FbTalker::authenticationDone(int errCode, const QString &errMsg)
{
    if (errCode != 0)
    {
        d->user.clear();
    }

    emit signalBusy(false);
    emit signalLoginDone(errCode, errMsg);
    d->loginInProgress = false;
}

int FbTalker::parseErrorResponse(const QDomElement& e, QString& errMsg)
{
    int errCode = -1;

    for (QDomNode node = e.firstChild();
         !node.isNull();
         node = node.nextSibling())
    {
        if (!node.isElement())
            continue;

        if (node.nodeName() == QString::fromLatin1("error_code"))
        {
            errCode = node.toElement().text().toInt();
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error Code:" << errCode;
        }
        else if (node.nodeName() == QString::fromLatin1("error_msg"))
        {
            errMsg = node.toElement().text();
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error Text:" << errMsg;
        }
    }

    return errCode;
}

//TODO: Port to O2
void FbTalker::parseResponseGetLoggedInUser(const QByteArray& data)
{
    QString errMsg;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Logged in data " << doc;
    
    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        return;
    }

    QJsonObject jsonObject = doc.object();
    d->user.id             = jsonObject[QString::fromLatin1("id")].toString();
    
    if (!(QString::compare(jsonObject[QLatin1String("id")].toString(),
                            QLatin1String(""), 
                            Qt::CaseInsensitive) == 0))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "ID found in response of GetLoggedInUser";
    }

    d->user.name       = jsonObject[QString::fromLatin1("name")].toString();
    d->user.profileURL = jsonObject[QString::fromLatin1("link")].toString();

    m_userName = d->user.name;
    
    authenticationDone(0, QString::fromLatin1(""));
}

void FbTalker::parseResponseAddPhoto(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) <<"Parse Add Photo data is "<<data;
    int errCode       = -1;
    QString errMsg;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        return;
    }

    QJsonObject jsonObject = doc.object();

    if (jsonObject.contains(QString::fromLatin1("id")))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Id of photo exported is" << jsonObject[QString::fromLatin1("id")].toString();
        errCode = 0;
    }

    if (jsonObject.contains(QString::fromLatin1("error")))
    {
        QJsonObject obj = jsonObject[QString::fromLatin1("error")].toObject();
        errCode         = obj[QString::fromLatin1("code")].toInt();
        errMsg          = obj[QString::fromLatin1("message")].toString();
    }
    
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "add photo : " << doc;

    emit signalBusy(false);
    emit signalAddPhotoDone(errCode, errorToText(errCode, errMsg));
}

void FbTalker::parseResponseCreateAlbum(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) <<"Parse Create album data is"<<data;
    int errCode       = -1;
    QString errMsg;
    QString newAlbumID;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        return;
    }

    QJsonObject jsonObject = doc.object();

    if (jsonObject.contains(QString::fromLatin1("id")))
    {
        newAlbumID = jsonObject[QString::fromLatin1("id")].toString();
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Id of album created is" << newAlbumID;
        errCode    = 0;
    }

    if (jsonObject.contains(QString::fromLatin1("error")))
    {
        QJsonObject obj = jsonObject[QString::fromLatin1("error")].toObject();
        errCode         = obj[QString::fromLatin1("code")].toInt();
        errMsg          = obj[QString::fromLatin1("message")].toString();
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "error create photo : " << doc;
    
    emit signalBusy(false);
    emit signalCreateAlbumDone(errCode, errorToText(errCode, errMsg),
                               newAlbumID);
}

void FbTalker::parseResponseListAlbums(const QByteArray& data)
{
    int errCode = -1;
    QString errMsg;
    QJsonParseError err;
    QList<WSAlbum> albumsList; // QList <FbAlbum> albumsList;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        return;
    }

    QJsonObject jsonObject = doc.object();

    if (jsonObject.contains(QString::fromLatin1("data")))
    {
        QJsonArray jsonArray = jsonObject[QString::fromLatin1("data")].toArray();

        foreach (const QJsonValue & value, jsonArray)
        {
            QJsonObject obj   = value.toObject();
            WSAlbum album; //FbAlbum album;
            album.id          = obj[QString::fromLatin1("id")].toString();
            album.title       = obj[QString::fromLatin1("name")].toString();
            album.location    = obj[QString::fromLatin1("location")].toString();
            album.url         = obj[QString::fromLatin1("link")].toString();
            album.description = obj[QString::fromLatin1("description")].toString();

            if (QString::compare(obj[QString::fromLatin1("privacy")].toString(),
                                 QString::fromLatin1("ALL_FRIENDS"), Qt::CaseInsensitive) == 0)
            {
                album.privacy = FB_FRIENDS;
            }
            else if (QString::compare(obj[QString::fromLatin1("privacy")].toString(),
                                      QString::fromLatin1("FRIENDS_OF_FRIENDS"), Qt::CaseInsensitive) == 0)
            {
                album.privacy = FB_FRIENDS;
            }
            else if (QString::compare(obj[QString::fromLatin1("privacy")].toString(),
                                      QString::fromLatin1("EVERYONE"), Qt::CaseInsensitive) == 0)
            {
                album.privacy = FB_EVERYONE;
            }
            else if (QString::compare(obj[QString::fromLatin1("privacy")].toString(),
                                      QString::fromLatin1("CUSTOM"), Qt::CaseInsensitive) == 0)
            {
                album.privacy = FB_CUSTOM;
            }
            else if (QString::compare(obj[QString::fromLatin1("privacy")].toString(),
                                      QString::fromLatin1("SELF"), Qt::CaseInsensitive) == 0)
            {
                album.privacy = FB_ME;
            }
            
            albumsList.append(album);
        }

        errCode = 0;
    }

    if (jsonObject.contains(QString::fromLatin1("error")))
    {
        QJsonObject obj = jsonObject[QString::fromLatin1("error")].toObject();
        errCode         = obj[QString::fromLatin1("code")].toInt();
        errMsg          = obj[QString::fromLatin1("message")].toString();
    }

    // std::sort(albumsList.begin(), albumsList.end());

    emit signalBusy(false);
    emit signalListAlbumsDone(errCode, errorToText(errCode, errMsg),
                              albumsList);
}

} // namespace Digikam

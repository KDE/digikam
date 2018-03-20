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

// C++ includes

#include <ctime>

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

    enum State
    {
        FB_GETLOGGEDINUSER = 0,
        FB_LISTALBUMS,
        FB_CREATEALBUM,
        FB_ADDPHOTO,
        FB_EXCHANGESESSION
    };

public:

    explicit Private()
    {
        state           = FB_GETLOGGEDINUSER;
        apiVersion      = QString::fromLatin1("2.4");
        apiURL          = QUrl(QString::fromLatin1("https://graph.facebook.com"));
        secretKey       = QString::fromLatin1("5b0b5cd096e110cd4f4c72f517e2c544");
        appID           = QString::fromLatin1("400589753481372");
        dialog          = 0;
        parent          = 0;
        reply           = 0;
        loginInProgress = false;
        sessionExpires  = 0;
        netMngr         = 0;
    }

    QDialog*               dialog;
    QWidget*               parent;

    QByteArray             buffer;

    QUrl                   apiURL;
    QString                apiVersion;
    QString                secretKey;
    QString                appID;

    bool                   loginInProgress;
    QString                accessToken;

    /* Session expiration
     * 0 = doesn't expire or has been invalidated; rest = time of expiry
     */
    unsigned int           sessionExpires;
    QTime                  callID;

    FbUser                 user;

    QNetworkAccessManager* netMngr;

    QNetworkReply*         reply;

    State                  state;
};

// -----------------------------------------------------------------------------

FbTalker::FbTalker(QWidget* const parent)
    : d(new Private)
{
    d->parent  = parent;
    d->netMngr = new QNetworkAccessManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));
}

FbTalker::~FbTalker()
{
    // do not logout - may reuse session for next upload

    if (d->reply)
    {
        d->reply->abort();
    }

    delete d;
}

bool FbTalker::loggedIn() const
{
    return !d->accessToken.isEmpty();
}

QString FbTalker::getAccessToken() const
{
    return d->accessToken;
}

unsigned int FbTalker::getSessionExpires() const
{
    return d->sessionExpires;
}

FbUser FbTalker::getUser() const
{
    return d->user;
}

void FbTalker::cancel()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(false);
}

/** Compute MD5 signature using url queries keys and values:
    http://wiki.developers.facebook.com/index.php/How_Facebook_Authenticates_Your_Application
    This method was used for the legacy authentication scheme and has been obsoleted with OAuth2 authentication.
*/
/*
QString FbTalker::getApiSig(const QMap<QString, QString>& args)
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
        concat.append(d->secretKey);
    else
        concat.append(d->sessionSecret);

    KMD5 md5(concat.toUtf8());
    return md5.hexDigest().data();
}
*/

QString FbTalker::getCallString(const QMap<QString, QString>& args)
{
    QString concat;
    QUrl url;
    QUrlQuery q;

    // NOTE: QMap iterator will sort alphabetically

    for (QMap<QString, QString>::const_iterator it = args.constBegin();
         it != args.constEnd();
         ++it)
    {
/*
        if (!concat.isEmpty())
            concat.append("&");
*/

/*
        concat.append(it.key());
        concat.append("=");
        concat.append(it.value());
*/
        q.addQueryItem(it.key(), it.value());
        url.setQuery(q);
    }

    concat.append(url.query());

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "CALL: " << concat;

    return concat;
}

void FbTalker::authenticate(const QString &accessToken, unsigned int sessionExpires)
{
    d->loginInProgress = true;

    if (!accessToken.isEmpty() && ( sessionExpires == 0 ||
        sessionExpires > (unsigned int)(time(0) + 900)))
    {
        // sessionKey seems to be still valid for at least 15 minutes
        // - check if it still works
        d->accessToken    = accessToken;
        d->sessionExpires = sessionExpires;

        emit signalLoginProgress(2, 9, i18n("Validate previous session..."));

        // get logged in user - this will check if session is still valid
        getLoggedInUser();
    }
    else
    {
        // session expired -> get new authorization token and session
        doOAuth();
    }
}

/**
 * upgrade session key to OAuth
 *
 * This method (or step) can be removed after June 2012 (a year after its
 * implementation), since it is only a convenience method for those people
 * who just upgraded and have an active session using the old authentication.
 */
void FbTalker::exchangeSession(const QString& sessionKey)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);
    emit signalLoginProgress(1, 9, i18n("Upgrading to OAuth..."));

    QMap<QString, QString> args;
    args[QString::fromLatin1("client_id")]     = d->appID;
    args[QString::fromLatin1("client_secret")] = d->secretKey;
    args[QString::fromLatin1("sessions")]      = sessionKey;

    QByteArray tmp(getCallString(args).toUtf8());

    QNetworkRequest netRequest(QUrl(QLatin1String("https://graph.facebook.com/oauth/exchange_sessions")));
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                         QLatin1String("application/x-www-form-urlencoded"));

    d->reply = d->netMngr->post(netRequest, tmp);
    d->state = Private::FB_EXCHANGESESSION;
    d->buffer.resize(0);
}

/**
 * Authenticate using OAuth
 *
 * TODO (Dirk): There's some GUI code slipped in here,
 * that really doesn't feel like it's belonging here.
 */
void FbTalker::doOAuth()
{
    // just in case
    d->loginInProgress = true;

    // TODO (Dirk):
    // Find out whether this signalBusy is used here appropriately.
    emit signalBusy(true);

    QUrl url(QString::fromLatin1("https://www.facebook.com/dialog/oauth"));
    QUrlQuery q(url);
    q.addQueryItem(QString::fromLatin1("client_id"), d->appID);
    q.addQueryItem(QString::fromLatin1("redirect_uri"),
                     QString::fromLatin1("https://www.facebook.com/connect/login_success.html"));
    // TODO (Dirk): Check which of these permissions can be optional.
    q.addQueryItem(QString::fromLatin1("scope"),
                     QString::fromLatin1("user_photos,publish_actions,user_friends"));
    q.addQueryItem(QString::fromLatin1("response_type"), QString::fromLatin1("token"));
    url.setQuery(q);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "OAuth URL: " << url;
    QDesktopServices::openUrl(url);

    emit signalBusy(false);

    d->dialog                       = new QDialog(QApplication::activeWindow(), 0);
    d->dialog->setModal(true);
    d->dialog->setWindowTitle(i18n("Facebook Application Authorization"));
    QLineEdit* const textbox        = new QLineEdit();
    QDialogButtonBox* const buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, d->dialog);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    d->dialog->connect(buttons, SIGNAL(accepted()),
                      this, SLOT(slotAccept()));

    d->dialog->connect(buttons, SIGNAL(rejected()),
                      this, SLOT(slotReject()));

    QPlainTextEdit* const infobox = new QPlainTextEdit(i18n("Please follow the instructions in the browser window. "
                                                            "When done, copy the Internet address from your browser into the textbox below and press \"OK\"."));
    infobox->setReadOnly(true);

    QVBoxLayout* const vbx = new QVBoxLayout(d->dialog);
    vbx->addWidget(infobox);
    vbx->addWidget(textbox);
    vbx->addWidget(buttons);
    d->dialog->setLayout(vbx);

    d->dialog->exec();

    if ( d->dialog->result() == QDialog::Accepted )
    {
        // Error code and reason from the Facebook service
        QString errorReason;
        QString errorCode;

        url                        = QUrl( textbox->text() );
        QString fragment           = url.fragment();
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Split out the fragment from the URL: " << fragment;
        QStringList params         = fragment.split(QLatin1Char('&'));
        QList<QString>::iterator i = params.begin();

        while( i != params.end() )
        {
            QStringList keyvalue = (*i).split(QLatin1Char('='));

            if ( keyvalue.size() == 2 )
            {
                if ( ! keyvalue[0].compare( QString::fromLatin1("access_token") ) )
                {
                    d->accessToken = keyvalue[1];
                }
                else if ( ! keyvalue[0].compare( QString::fromLatin1("expires_in") ) )
                {
                    d->sessionExpires = keyvalue[1].toUInt();

                    if ( d->sessionExpires != 0 )
                    {
                        d->sessionExpires += QDateTime::currentMSecsSinceEpoch() / 1000;
                    }
                }
                else if ( ! keyvalue[0].compare( QString::fromLatin1("error_reason") ) )
                {
                    errorReason = keyvalue[1];
                }
                else if ( ! keyvalue[0].compare( QString::fromLatin1("error") ) )
                {
                    errorCode = keyvalue[1];
                }
            }

            ++i;
        }

        if (!d->accessToken.isEmpty() && errorCode.isEmpty() && errorReason.isEmpty())
        {
            return getLoggedInUser();
        }
    }

    authenticationDone(-1, i18n("Canceled by user."));

    // TODO (Dirk): Correct?
    emit signalBusy(false);
}

void FbTalker::getLoggedInUser()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);
    emit signalLoginProgress(3);

    QUrl url(QString::fromLatin1("https://graph.facebook.com/me"));
    QUrlQuery q;
    q.addQueryItem(QString::fromLatin1("access_token"), d->accessToken);
    q.addQueryItem(QString::fromLatin1("fields"), QString::fromLatin1("id,name,link"));
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                         QLatin1String("application/x-www-form-urlencoded"));

    d->reply = d->netMngr->get(netRequest);
    d->state = Private::FB_GETLOGGEDINUSER;
    d->buffer.resize(0);
}

void FbTalker::logout()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    QMap<QString, QString> args;
    args[QString::fromLatin1("next")]         = QString::fromLatin1("http://www.digikam.org");
    args[QString::fromLatin1("access_token")] = d->accessToken;

    QUrl url(QString::fromLatin1("https://www.facebook.com/logout.php"));
    QUrlQuery q;
    q.addQueryItem(QString::fromLatin1("next"), QString::fromLatin1("http://www.digikam.org"));
    q.addQueryItem(QString::fromLatin1("access_token"), d->accessToken);
    url.setQuery(q);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Logout URL: " << url;
    QDesktopServices::openUrl(url);

    emit signalBusy(false);
}

void FbTalker::listAlbums(long long userID)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Requesting albums for user " << userID;

    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);

    QUrl url(QString::fromLatin1("https://graph.facebook.com/me/albums"));
    QUrlQuery q;
    q.addQueryItem(QString::fromLatin1("fields"),
                   QString::fromLatin1("id,name,description,privacy,link,location"));
    q.addQueryItem(QString::fromLatin1("access_token"), d->accessToken);
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                         QLatin1String("application/x-www-form-urlencoded"));

    d->reply = d->netMngr->get(netRequest);

    d->state = Private::FB_LISTALBUMS;
    d->buffer.resize(0);
}

void FbTalker::createAlbum(const FbAlbum& album)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);

    QMap<QString, QString> args;
    args[QString::fromLatin1("access_token")] = d->accessToken;
    args[QString::fromLatin1("name")]         = album.title;

    if (!album.location.isEmpty())
        args[QString::fromLatin1("location")] = album.location;
    if (!album.description.isEmpty())
        args[QString::fromLatin1("description")] = album.description;

    // TODO (Dirk): Wasn't that a requested feature in Bugzilla?
    switch (album.privacy)
    {
        case FB_ME:
            args[QString::fromLatin1("privacy")] = QString::fromLatin1("{'value':'SELF'}");
            break;
        case FB_FRIENDS:
            args[QString::fromLatin1("visible")] = QString::fromLatin1("friends");
            break;
        case FB_FRIENDS_OF_FRIENDS:
            args[QString::fromLatin1("visible")] = QString::fromLatin1("friends-of-friends");
            break;
        case FB_NETWORKS:
            args[QString::fromLatin1("visible")] = QString::fromLatin1("networks");
            break;
        case FB_EVERYONE:
            args[QString::fromLatin1("visible")] = QString::fromLatin1("everyone");
            break;
        case FB_CUSTOM:
            //TODO
            args[QString::fromLatin1("privacy")] = QString::fromLatin1("{'value':'CUSTOM'}");
            break;
    }

    QByteArray tmp(getCallString(args).toUtf8());

    QNetworkRequest netRequest(QUrl(QLatin1String("https://graph.facebook.com/me/albums")));
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    d->reply = d->netMngr->post(netRequest, tmp);
    d->state = Private::FB_CREATEALBUM;
    d->buffer.resize(0);
}

bool FbTalker::addPhoto(const QString& imgPath, const QString& albumID, const QString& caption)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Adding photo " << imgPath << " to album with id "
                                     << albumID << " using caption '" << caption << "'";

    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);

    QMap<QString, QString> args;
    args[QString::fromLatin1("access_token")] = d->accessToken;

    if (!caption.isEmpty())
        args[QString::fromLatin1("message")]  = caption;

    FbMPForm form;

    for (QMap<QString, QString>::const_iterator it = args.constBegin();
         it != args.constEnd();
         ++it)
    {
        form.addPair(it.key(), it.value());
    }

    if (!form.addFile(QUrl::fromLocalFile(imgPath).fileName(), imgPath))
    {
        emit signalBusy(false);
        return false;
    }

    form.finish();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "FORM: " << endl << form.formData();

    QNetworkRequest netRequest(QUrl(QLatin1String("https://graph.facebook.com/v2.4/") +
                                    albumID + QLatin1String("/photos")));
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());

    d->reply = d->netMngr->post(netRequest, form.formData());
    d->state = Private::FB_ADDPHOTO;
    d->buffer.resize(0);

    return true;
}

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

void FbTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != d->reply)
    {
        return;
    }

    d->reply = 0;

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

        reply->deleteLater();
        return;
    }

    d->buffer.append(reply->readAll());

    switch(d->state)
    {
        case (Private::FB_EXCHANGESESSION):
            parseExchangeSession(d->buffer);
            break;
        case (Private::FB_GETLOGGEDINUSER):
            parseResponseGetLoggedInUser(d->buffer);
            break;
        case (Private::FB_LISTALBUMS):
            parseResponseListAlbums(d->buffer);
            break;
        case (Private::FB_CREATEALBUM):
            parseResponseCreateAlbum(d->buffer);
            break;
        case (Private::FB_ADDPHOTO):
            parseResponseAddPhoto(d->buffer);
            break;
    }

    reply->deleteLater();
}

void FbTalker::authenticationDone(int errCode, const QString &errMsg)
{
    if (errCode != 0)
    {
        d->accessToken.clear();
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

void FbTalker::parseExchangeSession(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Parse exchange_session response:" << endl << data;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if(err.error == QJsonParseError::NoError)
    {
        QJsonObject jsonObject = doc.object();
        d->accessToken         = jsonObject[QString::fromLatin1("access_token")].toString();
        d->sessionExpires      = jsonObject[QString::fromLatin1("expires")].toInt();

        if( d->sessionExpires != 0 )
        {
            d->sessionExpires += QDateTime::currentMSecsSinceEpoch() / 1000;
        }

        if( d->accessToken.isEmpty() )
            // Session did not convert. Reauthenticate.
            doOAuth();
        else
            // Session converted to OAuth. Proceed normally.
            getLoggedInUser();
    }
    else
    {
        int errCode = -1;
        QString errMsg(QString::fromLatin1("Parse Error"));
        authenticationDone(errCode, errorToText(errCode, errMsg));
    }
}

void FbTalker::parseResponseGetLoggedInUser(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Logged in data " << data;
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
    d->user.id             = jsonObject[QString::fromLatin1("id")].toString().toLongLong();

    if (!(QString::compare(jsonObject[QString::fromLatin1("id")].toString(),
                           QString::fromLatin1(""), Qt::CaseInsensitive) == 0))
    {
        errCode = 0;
    }

    d->user.name       = jsonObject[QString::fromLatin1("name")].toString();
    d->user.profileURL = jsonObject[QString::fromLatin1("link")].toString();

    if(errCode!=0)
    {
        // it seems that session expired -> create new token and session
        d->accessToken.clear();
        d->sessionExpires = 0;
        d->user.clear();

        doOAuth();
    }
    else
    {
        authenticationDone(0, QString::fromLatin1(""));
    }
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

    emit signalBusy(false);
    emit signalCreateAlbumDone(errCode, errorToText(errCode, errMsg),
                               newAlbumID);
}

void FbTalker::parseResponseListAlbums(const QByteArray& data)
{
    int errCode = -1;
    QString errMsg;
    QJsonParseError err;
    QList <FbAlbum> albumsList;
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
            FbAlbum album;
            album.id          = obj[QString::fromLatin1("id")].toString();
            album.title       = obj[QString::fromLatin1("name")].toString();
            album.location    = obj[QString::fromLatin1("location")].toString();
            album.url         = obj[QString::fromLatin1("link")].toString();
            album.description = obj[QString::fromLatin1("description")].toString();

            if (QString::compare(obj[QString::fromLatin1("privacy")].toString(),
                                 QString::fromLatin1("friends"), Qt::CaseInsensitive) == 0)
            {
                album.privacy = FB_FRIENDS;
            }
            else if (QString::compare(obj[QString::fromLatin1("privacy")].toString(),
                                      QString::fromLatin1("custom"), Qt::CaseInsensitive) == 0)
            {
                album.privacy = FB_CUSTOM;
            }
            else if (QString::compare(obj[QString::fromLatin1("privacy")].toString(),
                                      QString::fromLatin1("everyone"), Qt::CaseInsensitive) == 0)
            {
                album.privacy = FB_EVERYONE;
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

    std::sort(albumsList.begin(), albumsList.end());

    emit signalBusy(false);
    emit signalListAlbumsDone(errCode, errorToText(errCode, errMsg),
                              albumsList);
}

void FbTalker::slotAccept()
{
    d->dialog->close();
    d->dialog->setResult(QDialog::Accepted);
}

void FbTalker::slotReject()
{
    d->dialog->close();
    d->dialog->setResult(QDialog::Rejected);
}

} // namespace Digikam

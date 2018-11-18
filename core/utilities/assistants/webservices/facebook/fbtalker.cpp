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
#include <QApplication>
#include <QPushButton>
#include <QDialog>
#include <QDialogButtonBox>
#include <QUrlQuery>
#include <QSettings>
#include <QMessageBox>
#include <QNetworkCookieJar>
#include <QNetworkAccessManager>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_version.h"
#include "fbmpform.h"
#include "wstoolutils.h"

#ifdef HAVE_QWEBENGINE
#   include "webwidget_qwebengine.h"
#else
#   include "webwidget.h"
#endif

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

class Q_DECL_HIDDEN FbTalker::Private
{
public:

    enum State
    {
        FB_GETLOGGEDINUSER = 0,
        FB_LOGOUTUSER,
        FB_LISTALBUMS,
        FB_CREATEALBUM,
        FB_ADDPHOTO
    };

public:

    explicit Private()
    {
        apiURL             = QLatin1String("https://graph.facebook.com/%1/%2");
        authUrl            = QLatin1String("https://www.facebook.com/dialog/oauth");
        tokenUrl           = QLatin1String("https://graph.facebook.com/oauth/access_token");
        redirectUrl        = QLatin1String("https://www.facebook.com/connect/login_success.html");
        scope              = QLatin1String("user_photos,publish_pages,manage_pages"); //publish_to_groups,user_friends not necessary?
        apikey             = QLatin1String("400589753481372");
        clientSecret       = QLatin1String("5b0b5cd096e110cd4f4c72f517e2c544");

        serviceName        = QLatin1String("Facebook");
        serviceDate        = QLatin1String("token_date");
        serviceKey         = QLatin1String("access_token");

        dialog             = 0;
        parent             = 0;
        settings           = 0;
        netMngr            = 0;
        reply              = 0;
        view               = 0;
        state              = FB_GETLOGGEDINUSER;
    }

    QString                apiURL;
    QString                authUrl;
    QString                tokenUrl;
    QString                redirectUrl;
    QString                scope;
    QString                apikey;
    QString                clientSecret;
    QString                accessToken;
    QString                serviceName;
    QString                serviceDate;
    QString                serviceKey;

    QDialog*               dialog;
    QWidget*               parent;

    QSettings*             settings;

    QNetworkAccessManager* netMngr;
    QNetworkReply*         reply;

    WebWidget*             view;

    State                  state;

    QMap<QString, QString> urlParametersMap;

    FbUser                 user;
};

// -----------------------------------------------------------------------------

FbTalker::FbTalker(QWidget* const parent)
    : d(new Private())
{
    d->parent   = parent;
    d->netMngr  = new QNetworkAccessManager(this);
    d->view     = new WebWidget(d->parent);
    d->view->resize(800, 600);

    d->settings = WSToolUtils::getOauthSettings(this);

    connect(this, SIGNAL(linkingFailed()),
            this, SLOT(slotLinkingFailed()));

    connect(this, SIGNAL(linkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    connect(d->view, SIGNAL(urlChanged(QUrl)),
            this, SLOT(slotCatchUrl(QUrl)));

    connect(d->view, SIGNAL(closeView(bool)),
            this, SIGNAL(signalBusy(bool)));
}

FbTalker::~FbTalker()
{
    if (d->reply)
    {
        d->reply->abort();
    }

    delete d;
}

// ----------------------------------------------------------------------------------------------

void FbTalker::link()
{
    emit signalBusy(true);

    QUrl url(d->authUrl);
    QUrlQuery query(url);
    query.addQueryItem(QLatin1String("client_id"), d->apikey);
    query.addQueryItem(QLatin1String("scope"), d->scope);
    query.addQueryItem(QLatin1String("redirect_uri"), d->redirectUrl);
    query.addQueryItem(QLatin1String("response_type"), "token");
    url.setQuery(query);

    d->view->setWindowFlags(Qt::Dialog);
    d->view->load(url);
    d->view->show();
}

void FbTalker::unlink()
{
    d->accessToken = QString();
    d->user        = FbUser();

    d->settings->beginGroup(d->serviceName);
    d->settings->remove(QString());
    d->settings->endGroup();

#ifdef HAVE_QWEBENGINE
    d->view->page()->profile()->cookieStore()->deleteAllCookies();
#else
    d->view->page()->networkAccessManager()->setCookieJar(new QNetworkCookieJar());
#endif

    emit linkingSucceeded();
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

void FbTalker::slotLinkingFailed()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Facebook fail";
    emit signalBusy(false);
}

void FbTalker::slotLinkingSucceeded()
{
    if (d->accessToken.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UNLINK to Facebook ok";
        emit signalBusy(false);
        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Facebook ok";
    d->view->close();
    writeSettings();

    getLoggedInUser();
}

void FbTalker::slotCatchUrl(const QUrl& url)
{
    d->urlParametersMap = parseUrlParameters(url.toString());
    QString accessToken = d->urlParametersMap.value("access_token");

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Received URL from webview in link function:" << url ;

    if (!accessToken.isEmpty())
    {
        qDebug(DIGIKAM_WEBSERVICES_LOG) << "Access Token Received";
        d->accessToken = accessToken;
        emit linkingSucceeded();
    }
    else
    {
        emit linkingFailed();
    }
}

QMap<QString, QString> FbTalker::parseUrlParameters(const QString& url)
{
    QMap<QString, QString> urlParameters;

    if (url.indexOf(QLatin1Char('#')) == -1)
    {
        return urlParameters;
    }

    QString tmp           = url.right(url.length() - url.indexOf(QLatin1Char('#')) - 1);
    tmp                   = tmp.right(tmp.length() - tmp.indexOf(QLatin1Char('#')) - 1);
    QStringList paramlist = tmp.split(QLatin1Char('&'));

    for (int i = 0 ; i < paramlist.count() ; ++i)
    {
        QStringList paramarg = paramlist.at(i).split(QLatin1Char('='));

        if (paramarg.count() == 2)
        {
            urlParameters.insert(paramarg.at(0), paramarg.at(1));
        }
    }

    return urlParameters;
}

FbUser FbTalker::getUser() const
{
    return d->user;
}

bool FbTalker::linked()
{
    return (!d->accessToken.isEmpty());
}

void FbTalker::getLoggedInUser()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "getLoggedInUser called ";

    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);
    emit signalLoginProgress(3);

    QUrl url(d->apiURL.arg("me").arg(""));

    QUrlQuery q;
    q.addQueryItem(QLatin1String("access_token"), d->accessToken.toUtf8());
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                         QLatin1String("application/x-www-form-urlencoded"));

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url = " << netRequest.url();
    d->reply = d->netMngr->get(netRequest);

    d->state = Private::FB_GETLOGGEDINUSER;
}

// ----------------------------------------------------------------------------------------------

void FbTalker::logout()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);

    QUrl url(QLatin1String("https://www.facebook.com/logout.php"));
    QUrlQuery q;
    q.addQueryItem(QLatin1String("access_token"), d->accessToken.toUtf8());
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                         QLatin1String("application/x-www-form-urlencoded"));

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url = " << netRequest.url();
    d->reply = d->netMngr->get(netRequest);

    d->state = Private::FB_LOGOUTUSER;
}

//----------------------------------------------------------------------------------------------------

void FbTalker::listAlbums(long long userID)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Requesting albums for user " << userID;

    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);

    QUrl url;

    /*
     * If userID is specified, load albums of that user,
     * else load albums of current user
     */
    if (!userID)
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
    q.addQueryItem(QLatin1String("access_token"), d->accessToken.toUtf8());
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                            QLatin1String("application/x-www-form-urlencoded"));

    d->reply = d->netMngr->get(netRequest);

    d->state = Private::FB_LISTALBUMS;
}

void FbTalker::createAlbum(const FbAlbum& album)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);

    QUrlQuery params;
    params.addQueryItem("access_token", d->accessToken.toUtf8());
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
    url.setQuery(params);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, 
                         QLatin1String("application/x-www-form-urlencoded"));

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url to create new album " << netRequest.url() << params.query(); 

    d->reply = d->netMngr->post(netRequest, params.query().toUtf8());

    d->state = Private::FB_CREATEALBUM;
}

void FbTalker::addPhoto(const QString& imgPath, const QString& albumID, const QString& caption)
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
    args[QLatin1String("access_token")] = d->accessToken.toUtf8();

    if (!caption.isEmpty())
        args[QLatin1String("message")]  = caption;

    FbMPForm form;

    for (QMap<QString, QString>::const_iterator it = args.constBegin() ;
        it != args.constEnd() ; ++it)
    {
        form.addPair(it.key(), it.value());
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "FORM: " << endl << form.formData();

    if (!form.addFile(QUrl::fromLocalFile(imgPath).fileName(), imgPath))
    {
        emit signalAddPhotoDone(666, i18n("Cannot open file"));
        emit signalBusy(false);
        return;
    }

    form.finish();

    QVariant arg_1;

    if (albumID.isEmpty())
    {
        arg_1 = d->user.id;
    }
    else
    {
        arg_1 = albumID;
    }

    QNetworkRequest netRequest(QUrl(d->apiURL.arg(arg_1.toString()).arg("photos")));
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());

    d->reply = d->netMngr->post(netRequest, form.formData());

    d->state = Private::FB_ADDPHOTO;
}

//----------------------------------------------------------------------------------------------------

QString FbTalker::errorToText(int errCode, const QString &errMsg)
{
    QString transError;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "errorToText: " << errCode << ": " << errMsg;

    switch (errCode)
    {
        case 0:
            transError = QLatin1String("");
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
        if (d->state == Private::FB_ADDPHOTO)
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

    QByteArray buffer = reply->readAll();

    switch(d->state)
    {
        case (Private::FB_GETLOGGEDINUSER):
            parseResponseGetLoggedInUser(buffer);
            break;
        case (Private::FB_LOGOUTUSER):
            parseResponseLogoutUser(buffer);
            break;
        case (Private::FB_LISTALBUMS):
            parseResponseListAlbums(buffer);
            break;
        case (Private::FB_CREATEALBUM):
            parseResponseCreateAlbum(buffer);
            break;
        case (Private::FB_ADDPHOTO):
            parseResponseAddPhoto(buffer);
            break;
    }

    reply->deleteLater();
}

int FbTalker::parseErrorResponse(const QDomElement& e, QString& errMsg)
{
    int errCode = -1;

    for (QDomNode node = e.firstChild() ;
         !node.isNull() ; node = node.nextSibling())
    {
        if (!node.isElement())
            continue;

        if (node.nodeName() == QLatin1String("error_code"))
        {
            errCode = node.toElement().text().toInt();
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error Code:" << errCode;
        }
        else if (node.nodeName() == QLatin1String("error_msg"))
        {
            errMsg = node.toElement().text();
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error Text:" << errMsg;
        }
    }

    return errCode;
}

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
    d->user.id             = jsonObject[QLatin1String("id")].toString();

    if (!(QString::compare(jsonObject[QLatin1String("id")].toString(), QLatin1String(""), Qt::CaseInsensitive) == 0))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "ID found in response of GetLoggedInUser";
    }

    d->user.name       = jsonObject[QLatin1String("name")].toString();
    d->user.profileURL = jsonObject[QLatin1String("link")].toString();

    emit signalLoginDone(0, QString());
}

void FbTalker::parseResponseLogoutUser(const QByteArray& /*data*/)
{
    unlink();
    emit signalLoginDone(-1, QString());
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

    if (jsonObject.contains(QLatin1String("id")))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Id of photo exported is" << jsonObject[QLatin1String("id")].toString();
        errCode = 0;
    }

    if (jsonObject.contains(QLatin1String("error")))
    {
        QJsonObject obj = jsonObject[QLatin1String("error")].toObject();
        errCode         = obj[QLatin1String("code")].toInt();
        errMsg          = obj[QLatin1String("message")].toString();
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

    if (jsonObject.contains(QLatin1String("id")))
    {
        newAlbumID = jsonObject[QLatin1String("id")].toString();
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Id of album created is" << newAlbumID;
        errCode    = 0;
    }

    if (jsonObject.contains(QLatin1String("error")))
    {
        QJsonObject obj = jsonObject[QLatin1String("error")].toObject();
        errCode         = obj[QLatin1String("code")].toInt();
        errMsg          = obj[QLatin1String("message")].toString();
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "error create photo : " << doc;

    emit signalBusy(false);
    emit signalCreateAlbumDone(errCode, errorToText(errCode, errMsg), newAlbumID);
}

void FbTalker::parseResponseListAlbums(const QByteArray& data)
{
    int errCode = -1;
    QString errMsg;
    QJsonParseError err;
    QList<FbAlbum> albumsList;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        return;
    }

    QJsonObject jsonObject = doc.object();

    if (jsonObject.contains(QLatin1String("data")))
    {
        QJsonArray jsonArray = jsonObject[QLatin1String("data")].toArray();

        foreach (const QJsonValue& value, jsonArray)
        {
            QJsonObject obj   = value.toObject();
            FbAlbum album;
            album.id          = obj[QLatin1String("id")].toString();
            album.title       = obj[QLatin1String("name")].toString();
            album.location    = obj[QLatin1String("location")].toString();
            album.url         = obj[QLatin1String("link")].toString();
            album.description = obj[QLatin1String("description")].toString();
            album.uploadable  = obj[QLatin1String("can_upload")].toBool();

            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "can_upload " << album.uploadable;

            if (QString::compare(obj[QLatin1String("privacy")].toString(),
                                 QLatin1String("ALL_FRIENDS"), Qt::CaseInsensitive) == 0)
            {
                album.privacy = FB_FRIENDS;
            }
            else if (QString::compare(obj[QLatin1String("privacy")].toString(),
                                      QLatin1String("FRIENDS_OF_FRIENDS"), Qt::CaseInsensitive) == 0)
            {
                album.privacy = FB_FRIENDS;
            }
            else if (QString::compare(obj[QLatin1String("privacy")].toString(),
                                      QLatin1String("EVERYONE"), Qt::CaseInsensitive) == 0)
            {
                album.privacy = FB_EVERYONE;
            }
            else if (QString::compare(obj[QLatin1String("privacy")].toString(),
                                      QLatin1String("CUSTOM"), Qt::CaseInsensitive) == 0)
            {
                album.privacy = FB_CUSTOM;
            }
            else if (QString::compare(obj[QLatin1String("privacy")].toString(),
                                      QLatin1String("SELF"), Qt::CaseInsensitive) == 0)
            {
                album.privacy = FB_ME;
            }

            albumsList.append(album);
        }

        errCode = 0;
    }

    if (jsonObject.contains(QLatin1String("error")))
    {
        QJsonObject obj = jsonObject[QLatin1String("error")].toObject();
        errCode         = obj[QLatin1String("code")].toInt();
        errMsg          = obj[QLatin1String("message")].toString();
    }

    std::sort(albumsList.begin(), albumsList.end());

    emit signalBusy(false);
    emit signalListAlbumsDone(errCode, errorToText(errCode, errMsg), albumsList);
}

void FbTalker::writeSettings()
{
    d->settings->beginGroup(d->serviceName);
    d->settings->setValue(d->serviceDate, QDateTime::currentDateTime());
    d->settings->setValue(d->serviceKey,  d->accessToken);
    d->settings->endGroup();
}

void FbTalker::readSettings()
{
    d->settings->beginGroup(d->serviceName);
    QDateTime dateTime = d->settings->value(d->serviceDate).toDateTime();
    d->accessToken     = d->settings->value(d->serviceKey).toString();
    d->settings->endGroup();

    if (d->accessToken.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Linking...";
        link();
    }
    else if (dateTime.secsTo(QDateTime::currentDateTime()) > 3600)
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Access token has expired";
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Linking...";
        d->accessToken = QString();
        link();
    }
    else
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Already Linked";
        emit linkingSucceeded();
    }
}

} // namespace Digikam

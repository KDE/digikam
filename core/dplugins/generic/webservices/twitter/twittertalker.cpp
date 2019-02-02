/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-06-29
 * Description : a tool to export images to Twitter social network
 *
 * Copyright (C) 2018 by Tarek Talaat <tarektalaat93 at gmail dot com>
 * Copyright (C) 2019 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

#include "twittertalker.h"

// Qt includes

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QByteArray>
#include <QUrl>
#include <QUrlQuery>
#include <QList>
#include <QPair>
#include <QFileInfo>
#include <QWidget>
#include <QSettings>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopServices>
#include <QNetworkAccessManager>

/*
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>
*/

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "wstoolutils.h"
#include "twitterwindow.h"
#include "twitteritem.h"
#include "twittermpform.h"
#include "previewloadthread.h"
#include "o0settingsstore.h"
#include "o1requestor.h"

namespace DigikamGenericTwitterPlugin
{

class Q_DECL_HIDDEN TwTalker::Private
{
public:

    enum State
    {
        TW_USERNAME = 0,
        TW_LISTFOLDERS,
        TW_CREATEFOLDER,
        TW_ADDPHOTO, 
        TW_CREATETWEET
    };

public:

    explicit Private()
    {
        clientId        = QLatin1String("lkRgRsucipXsUEvKh0ECblreC");
        clientSecret    = QLatin1String("6EThTiPQHZTMo7F83iLHrfNO89fkDVvM9hVWaYH9D49xEOyMBe");
        //scope          = QLatin1String("User.Read Files.ReadWrite");

        requestTokenUrl = QLatin1String("https://api.twitter.com/oauth/request_token");
        authUrl         = QLatin1String("https://api.twitter.com/oauth/authenticate");
        accessTokenUrl  = QLatin1String("https://api.twitter.com/oauth/access_token");

        redirectUrl     = QLatin1String("http://127.0.0.1:8000");

        state           = TW_USERNAME;

        parent          = 0;
        netMngr         = 0;
        reply           = 0;
        settings        = 0;
        o1Twitter       = 0;
    }

public:

    QString                clientId;
    QString                clientSecret;
    QString                authUrl;
    QString                requestTokenUrl;
    QString                accessTokenUrl;
    QString                scope;
    QString                redirectUrl;
    QString                accessToken;

    QWidget*               parent;

    QNetworkAccessManager* netMngr;

    QNetworkReply*         reply;

    State                  state;

    DMetadata              meta;

    QMap<QString, QString> urlParametersMap;

    //QWebEngineView*        view;

    QSettings*             settings;

    O1Twitter*             o1Twitter;
    O1Requestor*           requestor;
};

TwTalker::TwTalker(QWidget* const parent)
    : d(new Private)
{
    d->parent  = parent;
    d->netMngr = new QNetworkAccessManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    d->o1Twitter = new O1Twitter(this);
    d->o1Twitter->setClientId(d->clientId);
    d->o1Twitter->setClientSecret(d->clientSecret);
    d->o1Twitter->setLocalPort(8000);

    d->requestor = new O1Requestor(d->netMngr, d->o1Twitter, this);

    d->settings                  = WSToolUtils::getOauthSettings(this);
    O0SettingsStore* const store = new O0SettingsStore(d->settings, QLatin1String(O2_ENCRYPTION_KEY), this);
    store->setGroupKey(QLatin1String("Twitter"));
    d->o1Twitter->setStore(store);

    connect(d->o1Twitter, SIGNAL(linkingFailed()),
            this, SLOT(slotLinkingFailed()));

    connect(d->o1Twitter, SIGNAL(linkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));

    connect(d->o1Twitter, SIGNAL(openBrowser(QUrl)),
            this, SLOT(slotOpenBrowser(QUrl)));
}

TwTalker::~TwTalker()
{
    if (d->reply)
    {
        d->reply->abort();
    }

    WSToolUtils::removeTemporaryDir("Twitter");

    delete d;
}

void TwTalker::link()
{
/*
    emit signalBusy(true);
    QUrl url(d->requestTokenUrl);
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("OAuth oauth_callback= \"%1\"").arg(d->redirectUrl).toUtf8());
    QNetworkAccessManager requestMngr;
    QNetworkReply* reply;
    reply = requestMngr.post(netRequest);

    if (reply->error() != QNetworkReply::NoError){

    }

    QByteArray buffer;
    buffer.append(reply->readAll());
    QString response = fromLatin1(buffer);

    QMap<QString, QString> headers;

    // Discard the first line
    response = response.mid(response.indexOf('\n') + 1).trimmed();

    foreach (QString line, response.split('\n'))
    {
        int colon = line.indexOf(':');
        QString headerName = line.left(colon).trimmed();
        QString headerValue = line.mid(colon + 1).trimmed();

        headers.insertMulti(headerName, headerValue);
    }

    QString oauthToken = headers[oauth_token];
    QSting oauthTokenSecret = headers[oauth_token_secret];

    QUrlQuery query(url);
    query.addQueryItem(QLatin1String("client_id"),     d->clientId);
    query.addQueryItem(QLatin1String("scope"),         d->scope);
    query.addQueryItem(QLatin1String("redirect_uri"),  d->redirectUrl);
    query.addQueryItem(QLatin1String("response_type"), "token");
    url.setQuery(query);

    d->view = new QWebEngineView(d->parent);
    d->view->setWindowFlags(Qt::Dialog);
    d->view->load(url);
    d->view->show();

    connect(d->view, SIGNAL(urlChanged(QUrl)),
            this, SLOT(slotCatchUrl(QUrl)));
*/

    emit signalBusy(true);
    d->o1Twitter->link();
}

void TwTalker::unLink()
{
/*
    d->accessToken = QString();
    d->view->page()->profile()->cookieStore()->deleteAllCookies();
    emit oneDriveLinkingSucceeded();
*/

    d->o1Twitter->unlink();

    d->settings->beginGroup(QLatin1String("Dropbox"));
    d->settings->remove(QString());
    d->settings->endGroup();
}

void TwTalker::slotOpenBrowser(const QUrl& url)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Open Browser...";
    QDesktopServices::openUrl(url);
}

QMap<QString, QString> TwTalker::ParseUrlParameters(const QString &url)
{
    QMap<QString, QString> urlParameters;

    if (url.indexOf(QLatin1Char('?')) == -1)
    {
        return urlParameters;
    }

    QString tmp           = url.right(url.length()-url.indexOf(QLatin1Char('?'))-1);
    tmp                   = tmp.right(tmp.length() - tmp.indexOf(QLatin1Char('#'))-1);
    QStringList paramlist = tmp.split(QLatin1Char('&'));

    for (int i = 0 ; i < paramlist.count() ; ++i)
    {
        QStringList paramarg = paramlist.at(i).split(QLatin1Char('='));
        urlParameters.insert(paramarg.at(0),paramarg.at(1));
    }

    return urlParameters;
}

void TwTalker::slotLinkingFailed()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Twitter fail";
    emit signalBusy(false);
}

void TwTalker::slotLinkingSucceeded()
{
    if (!d->o1Twitter->linked())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UNLINK to Twitter ok";
        emit signalBusy(false);
        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Twitter ok";
    QVariantMap extraTokens = d->o1Twitter->extraTokens();

    if (!extraTokens.isEmpty())
    {
        //emit extraTokensReady(extraTokens);
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Extra tokens in response:";

        foreach (const QString& key, extraTokens.keys())
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "\t"
                                             << key
                                             << ":"
                                             << (extraTokens.value(key).toString().left(3) + QLatin1String("..."));
        }
    }

    emit signalLinkingSucceeded();

    getUserName();
}

bool TwTalker::authenticated()
{
    return d->o1Twitter->linked();
}

void TwTalker::cancel()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(false);
}

bool TwTalker::addPhoto(const QString& imgPath,
                        const QString& uploadFolder,
                        bool rescale,
                        int maxDim,
                        int imageQuality)
{
    //qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Status update message:" << message.toLatin1().constData();
    emit signalBusy(true);

    TwMPForm form;
    QImage image     = PreviewLoadThread::loadHighQualitySynchronously(imgPath).copyQImage();
    qint64 imageSize = QFileInfo(imgPath).size();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "SIZE of image using qfileinfo:   " << imageSize;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << " ";

    if (image.isNull())
    {
        emit signalBusy(false);
        return false;
    }

    QString path = WSToolUtils::makeTemporaryDir("Twitter").filePath(QFileInfo(imgPath)
                                                 .baseName().trimmed() + QLatin1String(".jpg"));

    if (rescale && (image.width() > maxDim || image.height() > maxDim))
    {
        image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    image.save(path, "JPEG", imageQuality);

    if (d->meta.load(imgPath))
    {
        d->meta.setItemDimensions(image.size());
        d->meta.setItemOrientation(DMetadata::ORIENTATION_NORMAL);
        d->meta.setMetadataWritingMode((int)DMetadata::WRITE_TO_FILE_ONLY);
        d->meta.save(path, true);
    }

    if (!form.addFile(path))
    {
        emit signalBusy(false);
        return false;
    }

    QString uploadPath = uploadFolder + QUrl(QUrl::fromLocalFile(imgPath)).fileName();

    if (form.formData().isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Form DATA Empty:";
    }

    if (form.formData().isNull())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Form DATA null:";
    }

    QUrl url = QUrl(QLatin1String("https://upload.twitter.com/1.1/media/upload.json"));

    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    //These are the parameters passed for the first step in chuncked media upload.
    //reqParams << O0RequestParameter(QByteArray("command"), QByteArray("INIT"));
    //reqParams << O0RequestParameter(QByteArray("media_type"), QByteArray("image/jpeg"));
    //reqParams << O0RequestParameter(QByteArray("total_bytes"), QString::fromLatin1("%1").arg(imageSize).toUtf8());

    //reqParams << O0RequestParameter(QByteArray("media"), form.formData());

    //QByteArray postData = O1::createQueryParameters(reqParams);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());
    d->reply = d->requestor->post(request, reqParams, form.formData());

    d->state = Private::TW_ADDPHOTO;

    return true;
}

void TwTalker::getUserName()
{
    /*
     * The endpoint below allows to get more than just account name (e.g. profile avatar, links to tweets posted, etc.)
     * Look at debug message printed to console for futher ideas and exploitation
     */
    QUrl url(QLatin1String("https://api.twitter.com/1.1/account/verify_credentials.json"));

    QNetworkRequest request(url);
    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    d->reply = d->requestor->get(request, reqParams);
    d->state = Private::TW_USERNAME;
    
    emit signalBusy(true);
}

void TwTalker::createTweet(const QString& mediaId)
{
    QUrl url = QUrl(QLatin1String("https://api.twitter.com/1.1/statuses/update.json"));
    
    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();
    reqParams << O0RequestParameter(QByteArray("status"), QByteArray(""));
    reqParams << O0RequestParameter(QByteArray("media_ids"), mediaId.toUtf8());
    QByteArray  postData = O1::createQueryParameters(reqParams);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_XFORM));
    d->reply = d->requestor->post(request, reqParams, postData);

    d->state = Private::TW_CREATETWEET;
}

void TwTalker::slotFinished(QNetworkReply* reply)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "TwTalker::slotFinished";

    if (reply != d->reply)
    {
        return;
    }

    d->reply = 0;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (d->state != Private::TW_CREATEFOLDER)
        {
            emit signalBusy(false);
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18n("Error"), reply->errorString());

            reply->deleteLater();
            return;
        }
    }

    QByteArray buffer = reply->readAll();

    switch (d->state)
    {
        case Private::TW_LISTFOLDERS:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In TW_LISTFOLDERS";
            parseResponseListFolders(buffer);
            break;
        case Private::TW_CREATEFOLDER:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In TW_CREATEFOLDER";
            parseResponseCreateFolder(buffer);
            break;
        case Private::TW_ADDPHOTO:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In TW_ADDPHOTO";
            parseResponseAddPhoto(buffer);
            break;
        case Private::TW_USERNAME:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In TW_USERNAME";
            parseResponseUserName(buffer);
            break;
        case Private::TW_CREATETWEET:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In TW_CREATETWEET";
            parseResponseCreateTweet(buffer);
        default:
            break;
    }

    reply->deleteLater();
}

void TwTalker::parseResponseAddPhoto(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc      = QJsonDocument::fromJson(data, &err);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseAddPhoto: " << doc;

    if(err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        emit signalAddPhotoFailed(i18n("Failed to upload photo"));
        return;
    }

    QJsonObject jsonObject = doc.object();
    QString mediaId = jsonObject[QLatin1String("media_id_string")].toString();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "media id: " << mediaId;

    // We haven't emit signalAddPhotoSucceeded() here yet, since we need to update the status first
    createTweet(mediaId); 
}

void TwTalker::parseResponseUserName(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseUserName: "<<doc;

    if(err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        return;
    }

    QString name      = doc.object()[QLatin1String("name")].toString();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "user name: "<<name;
    
    emit signalBusy(false);
    emit signalSetUserName(name);
}

void TwTalker::parseResponseCreateTweet(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseCreateTweet: " << doc;

    if(err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        emit signalAddPhotoFailed(i18n("Failed to create tweet for photo uploaded"));
        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Tweet posted successfully!";    
    emit signalBusy(false);
    emit signalAddPhotoSucceeded();
}

void TwTalker::parseResponseListFolders(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        emit signalListAlbumsFailed(i18n("Failed to list folders"));
        return;
    }

    QJsonObject jsonObject = doc.object();
    //qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Json: " << doc;
    QJsonArray jsonArray   = jsonObject[QLatin1String("value")].toArray();

    //qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Json response: " << jsonArray;

    QList<QPair<QString, QString> > list;
    list.append(qMakePair(QLatin1String(""), QLatin1String("root")));

    foreach (const QJsonValue& value, jsonArray)
    {
        QString path;
        QString folderName;
        QJsonObject folder;

        QJsonObject obj = value.toObject();
        folder          = obj[QLatin1String("folder")].toObject();

        if (!folder.isEmpty())
        {
            folderName    = obj[QLatin1String("name")].toString();
            path          = QLatin1Char('/') + folderName;
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Folder Name is" << folderName;
            list.append(qMakePair(path, folderName));
        }
    }

    emit signalBusy(false);
    emit signalListAlbumsDone(list);
}

void TwTalker::parseResponseCreateFolder(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object();
    bool fail              = jsonObject.contains(QLatin1String("error"));

    emit signalBusy(false);

    if (fail)
    {
      QJsonParseError err;
      QJsonDocument doc = QJsonDocument::fromJson(data, &err);
      qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseCreateFolder ERROR: " << doc;
      emit signalCreateFolderFailed(jsonObject[QLatin1String("error_summary")].toString());
    }
    else
    {
        emit signalCreateFolderSucceeded();
    }
}

} // namespace DigikamGenericTwitterPlugin

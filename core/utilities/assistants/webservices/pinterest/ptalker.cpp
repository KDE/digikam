/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Pinterest web service
 *
 * Copyright (C) 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
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

#include "ptalker.h"

// Qt includes

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QByteArray>
#include <QList>
#include <QPair>
#include <QFileInfo>
#include <QWidget>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopServices>
#include <QUrlQuery>
#include <QHttpMultiPart>
#include <QNetworkCookieJar>

// KDE includes

#include <kconfig.h>
#include <kwindowconfig.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "digikam_version.h"
#include "wstoolutils.h"
#include "pwindow.h"
#include "pitem.h"
#include "pmpform.h"
#include "previewloadthread.h"

#ifdef HAVE_QWEBENGINE
#   include "webwidget_qwebengine.h"
#else
#   include "webwidget.h"
#endif

namespace Digikam
{

class PTalker::Private
{
public:

    enum State
    {
        P_USERNAME = 0,
        P_LISTBOARDS,
        P_CREATEBOARD,
        P_ADDPIN,
        P_ACCESSTOKEN
    };

public:

    explicit Private()
    {
        clientId     = QLatin1String("4983380570301022071");
        clientSecret = QLatin1String("2a698db679125930d922a2dfb897e16b668a67c6f614593636e83fc3d8d9b47d");

        authUrl      = QLatin1String("https://api.pinterest.com/oauth/");
        tokenUrl     = QLatin1String("https://api.pinterest.com/v1/oauth/token");
        redirectUrl  = QLatin1String("https://login.live.com/oauth20_desktop.srf");

        scope        = QLatin1String("read_public,write_public");

        state        = P_USERNAME;
        netMngr      = 0;
        reply        = 0;
        accessToken  = QString();
    }

public:

    QString                clientId;
    QString                clientSecret;
    QString                authUrl;
    QString                tokenUrl;
    QString                redirectUrl;
    QString                accessToken;
    QString                scope;
    QWidget*               parent;

    QNetworkAccessManager* netMngr;

    QNetworkReply*         reply;

    State                  state;

    QByteArray             buffer;

    DMetadata              meta;

    QMap<QString, QString> urlParametersMap;

    WebWidget*             view;

    QString                userName;

    QSettings*             settings;
};

PTalker::PTalker(QWidget* const parent)
    : d(new Private)
{
    d->parent  = parent;
    d->netMngr = new QNetworkAccessManager(this);
    d->view    = new WebWidget(d->parent);

#ifndef HAVE_QWEBENGINE
    d->view->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true);
#endif

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    connect(this, SIGNAL(pinterestLinkingFailed()),
            this, SLOT(slotLinkingFailed()));

    connect(this, SIGNAL(pinterestLinkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));
}

PTalker::~PTalker()
{
    if (d->reply)
    {
        d->reply->abort();
    }

    delete d;
}

void PTalker::link()
{
    emit signalBusy(true);
    QUrl url(d->authUrl);
    QUrlQuery query(url);
    query.addQueryItem(QLatin1String("client_id"),     d->clientId);
    query.addQueryItem(QLatin1String("scope"),         d->scope);
    query.addQueryItem(QLatin1String("redirect_uri"),  d->redirectUrl);
    query.addQueryItem(QLatin1String("response_type"), "code");
    url.setQuery(query);

    d->view->setWindowFlags(Qt::Dialog);
    d->view->show();
    d->view->load(url);

    connect(d->view, SIGNAL(urlChanged(QUrl)),
            this, SLOT(slotCatchUrl(QUrl)));
    connect(d->view, SIGNAL(closeView(bool)),
            this, SIGNAL(signalBusy(bool)));
}

void PTalker::unLink()
{
    d->accessToken = QString();
    KConfig config;
    KConfigGroup grp   = config.group("Pinterest User Settings");
    grp.deleteGroup();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "group deleted";
#ifdef HAVE_QWEBENGINE
    d->view->page()->profile()->cookieStore()->deleteAllCookies();
#else
    d->view->page()->networkAccessManager()->setCookieJar(new QNetworkCookieJar());
#endif

    emit pinterestLinkingSucceeded();
}

void PTalker::slotCatchUrl(const QUrl& url)
{
    d->urlParametersMap = ParseUrlParameters(url.toString());
    QString code        = d->urlParametersMap.value("code");
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Received URL from webview in link function: " << url ;

    if (!code.isEmpty())
    {
       qCDebug(DIGIKAM_WEBSERVICES_LOG) << "CODE Received " ;
        d->view->close();
        getToken(code);
        emit signalBusy(false);
    }
}

void PTalker::getToken(const QString& code)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Code: " << code;
    QUrl url(d->tokenUrl);
    QUrlQuery query(url);
    query.addQueryItem(QLatin1String("grant_type"),    "authorization_code");
    query.addQueryItem(QLatin1String("client_id"),     d->clientId);
    query.addQueryItem(QLatin1String("client_secret"), d->clientSecret);
    query.addQueryItem(QLatin1String("code"),          code);
    url.setQuery(query);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Token Request URL:    " << url.toString();

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    netRequest.setRawHeader("Accept", "application/json");

    d->reply = d->netMngr->post(netRequest,QByteArray());

    d->state = Private::P_ACCESSTOKEN;
    d->buffer.resize(0);
}

QMap<QString, QString> PTalker::ParseUrlParameters(const QString& url)
{
    QMap<QString, QString> urlParameters;

    if (url.indexOf('?') == -1)
    {
        return urlParameters;
    }

    QString tmp           = url.right(url.length()-url.indexOf('?') - 1);
    QStringList paramlist = tmp.split('&');

    for (int i = 0 ; i < paramlist.count() ; ++i)
    {
        QStringList paramarg = paramlist.at(i).split('=');
        urlParameters.insert(paramarg.at(0),paramarg.at(1));
    }

    return urlParameters;
}

void PTalker::slotLinkingFailed()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Pinterest fail";
    emit signalBusy(false);
}

void PTalker::slotLinkingSucceeded()
{
    if (d->accessToken.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UNLINK to Pinterest ok";
        emit signalBusy(false);
        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Pinterest ok";
    writeSettings();
    emit signalLinkingSucceeded();
}

bool PTalker::authenticated()
{
    return (!d->accessToken.isEmpty());
}

void PTalker::cancel()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(false);
}

void PTalker::createBoard(QString& boardName)
{
    QUrl url("https://api.pinterest.com/v1/boards/");
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(d->accessToken).toUtf8());

    QByteArray postData = QString::fromUtf8("{\"name\": \"%1\"}").arg(boardName).toUtf8();
    //qCDebug(DIGIKAM_WEBSERVICES_LOG) << "createBoard:" << postData;
    d->reply = d->netMngr->post(netRequest, postData);

    d->state = Private::P_CREATEBOARD;
    d->buffer.resize(0);
    emit signalBusy(true);
}

void PTalker::getUserName()
{
    QUrl url(QLatin1String("https://api.pinterest.com/v1/me/?fields=username"));

    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(d->accessToken).toUtf8());

    d->reply = d->netMngr->get(netRequest);
    d->state = Private::P_USERNAME;
    d->buffer.resize(0);
    emit signalBusy(true);
}

/** Get list of boards by parsing json sent by pinterest
 */
void PTalker::listBoards(const QString& /*path*/)
{
    QUrl url(QLatin1String("https://api.pinterest.com/v1/me/boards/"));;

    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(d->accessToken).toUtf8());
    //netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));

    d->reply = d->netMngr->get(netRequest);

    d->state = Private::P_LISTBOARDS;
    d->buffer.resize(0);
    emit signalBusy(true);
}

bool PTalker::addPin(const QString& imgPath, const QString& uploadBoard, bool rescale, int maxDim, int imageQuality)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);

    PMPForm form;
    QImage image = PreviewLoadThread::loadHighQualitySynchronously(imgPath).copyQImage();

    if (image.isNull())
    {
        return false;
    }

    QString path = WSToolUtils::makeTemporaryDir("pinterest").filePath(QFileInfo(imgPath)
                                                 .baseName().trimmed() + QLatin1String(".jpg"));

    if (rescale && (image.width() > maxDim || image.height() > maxDim))
    {
        image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    image.save(path, "JPEG", imageQuality);

    if (d->meta.load(imgPath))
    {
        d->meta.setImageDimensions(image.size());
        d->meta.setImageOrientation(DMetadata::ORIENTATION_NORMAL);
        d->meta.setImageProgramId(QLatin1String("digiKam"), digiKamVersion());
        d->meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
        d->meta.save(path);
    }

    QString boardparam = d->userName + QLatin1Char('/') + uploadBoard;

    if (!form.addFile(path))
    {
        emit signalBusy(false);
        return false;
    }

    QUrl url(QString::fromLatin1("https://api.pinterest.com/v1/pins/?access_token=%1").arg(d->accessToken));

    QHttpMultiPart * multipart = new QHttpMultiPart (QHttpMultiPart::FormDataType);
    ///Board Section
    QHttpPart board;
    QString boardHeader = QString("form-data; name=\"board\"") ;
    board.setHeader(QNetworkRequest::ContentDispositionHeader, boardHeader);

    QByteArray postData = boardparam.toUtf8();
    board.setBody(postData);
    multipart->append(board);

    ///Note section
    QHttpPart note;
    QString noteHeader = QString("form-data; name=\"note\"") ;
    note.setHeader(QNetworkRequest::ContentDispositionHeader, noteHeader);

    postData = QByteArray();

    note.setBody(postData);
    multipart->append(note);

    ///image section
    QFile* const file = new QFile(imgPath);
    file->open(QIODevice::ReadOnly);

    QHttpPart imagepart;
    QString imagepartHeader = QLatin1String("form-data; name=\"image\"; filename=\"") +
                              QFileInfo(imgPath).fileName() + QLatin1Char('"');

    imagepart.setHeader(QNetworkRequest::ContentDispositionHeader,imagepartHeader);
    imagepart.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("image/jpeg"));

    imagepart.setBodyDevice(file);
    multipart->append(imagepart);

    QString content = QLatin1String("multipart/form-data;boundary=") + multipart->boundary();
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, content);

    d->reply = d->netMngr->post(netRequest, multipart);

    d->state = Private::P_ADDPIN;
    d->buffer.resize(0);
    emit signalBusy(true);

    return true;
}

void PTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != d->reply)
    {
        return;
    }

    d->reply = 0;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (d->state != Private::P_CREATEBOARD)
        {
            emit signalBusy(false);
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18n("Error"), reply->errorString());

            //qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error content: " << QString(reply->readAll());
            reply->deleteLater();
            return;
        }
    }

    d->buffer.append(reply->readAll());
    //qCDebug(DIGIKAM_WEBSERVICES_LOG) << "BUFFER" << QString(d->buffer);

    switch (d->state)
    {
        case Private::P_LISTBOARDS:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In P_LISTBOARDS";
            parseResponseListBoards(d->buffer);
            break;
        case Private::P_CREATEBOARD:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In P_CREATEBOARD";
            parseResponseCreateBoard(d->buffer);
            break;
        case Private::P_ADDPIN:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In P_ADDPIN";
            parseResponseAddPin(d->buffer);
            break;
        case Private::P_USERNAME:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In P_USERNAME";
            parseResponseUserName(d->buffer);
            break;
        case Private::P_ACCESSTOKEN:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In P_ACCESSTOKEN";
            parseResponseAccessToken(d->buffer);
            break;
        default:
            break;
    }

    reply->deleteLater();
}

void PTalker::parseResponseAccessToken(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object();
    d->accessToken         = jsonObject[QLatin1String("access_token")].toString();

    if (!d->accessToken.isEmpty())
    {
        qDebug(DIGIKAM_WEBSERVICES_LOG) << "Access token Received: " << d->accessToken;
        emit pinterestLinkingSucceeded();
    }
    else
    {
        emit pinterestLinkingFailed();
    }

    emit signalBusy(false);
}

void PTalker::parseResponseAddPin(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object()[QLatin1String("data")].toObject();
    bool success           = jsonObject.contains(QLatin1String("id"));
    emit signalBusy(false);

    if (!success)
    {
        emit signalAddPinFailed(i18n("Failed to upload Pin"));
    }
    else
    {
        emit signalAddPinSucceeded();
    }
}

void PTalker::parseResponseUserName(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object()[QLatin1String("data")].toObject();
    QString name           = jsonObject[QLatin1String("username")].toString();

    emit signalBusy(false);
    d->userName = name;
    emit signalSetUserName(name);
}

void PTalker::parseResponseListBoards(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        emit signalListBoardsFailed(i18n("Failed to list boards"));
        return;
    }

    QJsonObject jsonObject = doc.object();
    //qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Json Listing Boards : " << doc;
    QJsonArray jsonArray   = jsonObject[QLatin1String("data")].toArray();

    QList<QPair<QString, QString> > list;
    QString boardID;
    QString boardName;

    foreach (const QJsonValue& value, jsonArray)
    {
        QString boardID;
        QString boardName;
        QJsonObject obj = value.toObject();
        boardID         = obj[QLatin1String("id")].toString();
        boardName       = obj[QLatin1String("name")].toString();

        list.append(qMakePair(boardID, boardName));
    }

    emit signalBusy(false);
    emit signalListBoardsDone(list);
}

void PTalker::parseResponseCreateBoard(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object();
    bool fail              = jsonObject.contains(QLatin1String("error"));

    emit signalBusy(false);

    if (fail)
    {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        emit signalCreateBoardFailed(jsonObject[QLatin1String("error_summary")].toString());
    }
    else
    {
        emit signalCreateBoardSucceeded();
    }
}

void PTalker::writeSettings()
{
    KConfig config;
    KConfigGroup grp = config.group("Pinterest User Settings");

    grp.writeEntry("access_token", d->accessToken);
    config.sync();
}

void PTalker::readSettings()
{
    KConfig config;
    KConfigGroup grp   = config.group("Pinterest User Settings");

    //qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In read settings";
    if (!grp.readEntry("access_token", false))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Linking...";
        link();
    }
    else
    {
        d->accessToken = grp.readEntry("access_token",QString());
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Already Linked";
        emit pinterestLinkingSucceeded();

    }
}

} // namespace Digikam

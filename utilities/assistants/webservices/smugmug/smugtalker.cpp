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
#include <QTextDocument>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>
#include <QCryptographicHash>
#include <QUrlQuery>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "smugmpform.h"
#include "smugitem.h"

namespace Digikam
{

SmugTalker::SmugTalker(DInfoInterface* const iface, QWidget* const parent)
{
    m_parent     = parent;
    m_iface      = iface;
    m_reply      = 0;
    m_state      = SMUG_LOGOUT;

    // FIXME ?
    //m_userAgent  = QString::fromLatin1("KIPI-Plugin-Smug/%1 (lure@kubuntu.org)").arg(kipipluginsVersion());
    m_userAgent  = QString::fromLatin1("digiKam/%1 (digikamdeveloper@gmail.com)").arg(digiKamVersion());

    m_apiVersion = QString::fromLatin1("1.2.2");
    m_apiURL     = QString::fromLatin1("https://api.smugmug.com/services/api/rest/%1/").arg(m_apiVersion);
    m_apiKey     = QString::fromLatin1("R83lTcD4TvMsIiXqpdrA9OdIJ22uA4Wi");

    m_netMngr    = new QNetworkAccessManager(this);

    connect(m_netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));
}

SmugTalker::~SmugTalker()
{
    if (loggedIn())
    {
        logout();

        while (m_reply && m_reply->isRunning())
        {
            qApp->processEvents();
        }
    }

    if (m_reply)
        m_reply->abort();
}

bool SmugTalker::loggedIn() const
{
    return !m_sessionID.isEmpty();
}

SmugUser SmugTalker::getUser() const
{
    return m_user;
}

void SmugTalker::cancel()
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(false);
}

void SmugTalker::login(const QString& email, const QString& password)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);
    emit signalLoginProgress(1, 4, i18n("Logging in to SmugMug service..."));

    QUrl url(m_apiURL);
    QUrlQuery q;

    if (email.isEmpty())
    {
        q.addQueryItem(QString::fromLatin1("method"), QString::fromLatin1("smugmug.login.anonymously"));
        q.addQueryItem(QString::fromLatin1("APIKey"), m_apiKey);
    }
    else
    {
        q.addQueryItem(QString::fromLatin1("method"),       QString::fromLatin1("smugmug.login.withPassword"));
        q.addQueryItem(QString::fromLatin1("APIKey"),       m_apiKey);
        q.addQueryItem(QString::fromLatin1("EmailAddress"), email);
        q.addQueryItem(QString::fromLatin1("Password"),     password);
    }

    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);

    m_reply = m_netMngr->get(netRequest);

    m_state = SMUG_LOGIN;
    m_buffer.resize(0);

    m_user.email = email;
}

void SmugTalker::logout()
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);

    QUrl url(m_apiURL);
    QUrlQuery q;
    q.addQueryItem(QString::fromLatin1("method"),    QString::fromLatin1("smugmug.logout"));
    q.addQueryItem(QString::fromLatin1("SessionID"), m_sessionID);
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);

    m_reply = m_netMngr->get(netRequest);

    m_state = SMUG_LOGOUT;
    m_buffer.resize(0);
}

void SmugTalker::listAlbums(const QString& nickName)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);

    QUrl url(m_apiURL);
    QUrlQuery q;
    q.addQueryItem(QString::fromLatin1("method"),    QString::fromLatin1("smugmug.albums.get"));
    q.addQueryItem(QString::fromLatin1("SessionID"), m_sessionID);
    q.addQueryItem(QString::fromLatin1("Heavy"),     QString::fromLatin1("1"));

    if (!nickName.isEmpty())
        q.addQueryItem(QString::fromLatin1("NickName"), nickName);

    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);

    m_reply = m_netMngr->get(netRequest);

    m_state = SMUG_LISTALBUMS;
    m_buffer.resize(0);
}

void SmugTalker::listPhotos(const qint64 albumID,
                            const QString& albumKey,
                            const QString& albumPassword,
                            const QString& sitePassword)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);

    QUrl url(m_apiURL);
    QUrlQuery q;
    q.addQueryItem(QString::fromLatin1("method"),    QString::fromLatin1("smugmug.images.get"));
    q.addQueryItem(QString::fromLatin1("SessionID"), m_sessionID);
    q.addQueryItem(QString::fromLatin1("AlbumID"),   QString::number(albumID));
    q.addQueryItem(QString::fromLatin1("AlbumKey"),  albumKey);
    q.addQueryItem(QString::fromLatin1("Heavy"),     QString::fromLatin1("1"));

    if (!albumPassword.isEmpty())
        q.addQueryItem(QString::fromLatin1("Password"), albumPassword);

    if (!sitePassword.isEmpty())
        q.addQueryItem(QString::fromLatin1("SitePassword"), sitePassword);

    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);

    m_reply = m_netMngr->get(netRequest);

    m_state = SMUG_LISTPHOTOS;
    m_buffer.resize(0);
}

void SmugTalker::listAlbumTmpl()
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);

    QUrl url(m_apiURL);
    QUrlQuery q;
    q.addQueryItem(QString::fromLatin1("method"),    QString::fromLatin1("smugmug.albumtemplates.get"));
    q.addQueryItem(QString::fromLatin1("SessionID"), m_sessionID);
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);

    m_reply = m_netMngr->get(netRequest);

    m_state = SMUG_LISTALBUMTEMPLATES;
    m_buffer.resize(0);
}

void SmugTalker::listCategories()
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);

    QUrl url(m_apiURL);
    QUrlQuery q;
    q.addQueryItem(QString::fromLatin1("method"),    QString::fromLatin1("smugmug.categories.get"));
    q.addQueryItem(QString::fromLatin1("SessionID"), m_sessionID);
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);

    m_reply = m_netMngr->get(netRequest);

    m_state = SMUG_LISTCATEGORIES;
    m_buffer.resize(0);
}

void SmugTalker::listSubCategories(qint64 categoryID)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);

    QUrl url(m_apiURL);
    QUrlQuery q;
    q.addQueryItem(QString::fromLatin1("method"),     QString::fromLatin1("smugmug.subcategories.get"));
    q.addQueryItem(QString::fromLatin1("SessionID"),  m_sessionID);
    q.addQueryItem(QString::fromLatin1("CategoryID"), QString::number(categoryID));
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);

    m_reply = m_netMngr->get(netRequest);

    m_state = SMUG_LISTSUBCATEGORIES;
    m_buffer.resize(0);
}

void SmugTalker::createAlbum(const SmugAlbum& album)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);

    QUrl url(m_apiURL);
    QUrlQuery q;
    q.addQueryItem(QString::fromLatin1("method"),     QString::fromLatin1("smugmug.albums.create"));
    q.addQueryItem(QString::fromLatin1("SessionID"),  m_sessionID);
    q.addQueryItem(QString::fromLatin1("Title"),      album.title);
    q.addQueryItem(QString::fromLatin1("CategoryID"), QString::number(album.categoryID));

    if (album.subCategoryID > 0)
        q.addQueryItem(QString::fromLatin1("SubCategoryID"), QString::number(album.subCategoryID));

    if (!album.description.isEmpty())
        q.addQueryItem(QString::fromLatin1("Description"), album.description);

    if (album.tmplID > 0)
    {
        // template will also define privacy settings
        q.addQueryItem(QString::fromLatin1("AlbumTemplateID"), QString::number(album.tmplID));
    }
    else
    {
        if (!album.password.isEmpty())
            q.addQueryItem(QString::fromLatin1("Password"), album.password);

        if (!album.passwordHint.isEmpty())
            q.addQueryItem(QString::fromLatin1("PasswordHint"), album.passwordHint);

        if (album.isPublic)
            q.addQueryItem(QString::fromLatin1("Public"), QString::fromLatin1("1"));
        else
            q.addQueryItem(QString::fromLatin1("Public"), QString::fromLatin1("0"));
    }

    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);

    m_reply = m_netMngr->get(netRequest);

    m_state = SMUG_CREATEALBUM;
    m_buffer.resize(0);
}

bool SmugTalker::addPhoto(const QString& imgPath,
                          qint64 albumID,
                          const QString& albumKey,
                          const QString& caption)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
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
    netRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
    netRequest.setRawHeader("X-Smug-SessionID", m_sessionID.toLatin1());
    netRequest.setRawHeader("X-Smug-Version", m_apiVersion.toLatin1());

    m_reply = m_netMngr->post(netRequest, form.formData());

    m_state = SMUG_ADDPHOTO;
    m_buffer.resize(0);
    return true;
}

void SmugTalker::getPhoto(const QString& imgPath)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }

    emit signalBusy(true);

    QNetworkRequest netRequest(QUrl::fromLocalFile(imgPath));
    netRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
    netRequest.setRawHeader("X-Smug-SessionID", m_sessionID.toLatin1());
    netRequest.setRawHeader("X-Smug-Version", m_apiVersion.toLatin1());

    m_reply = m_netMngr->get(netRequest);

    m_state = SMUG_GETPHOTO;
    m_buffer.resize(0);
}

QString SmugTalker::errorToText(int errCode, const QString &errMsg)
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
    if (reply != m_reply)
    {
        return;
    }

    m_reply = 0;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (m_state == SMUG_LOGIN)
        {
            m_sessionID.clear();
            m_user.clear();

            emit signalBusy(false);
            emit signalLoginDone(reply->error(), reply->errorString());
        }
        else if (m_state == SMUG_ADDPHOTO)
        {
            emit signalBusy(false);
            emit signalAddPhotoDone(reply->error(), reply->errorString());
        }
        else if (m_state == SMUG_GETPHOTO)
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

    m_buffer.append(reply->readAll());

    switch(m_state)
    {
        case (SMUG_LOGIN):
            parseResponseLogin(m_buffer);
            break;
        case (SMUG_LOGOUT):
            parseResponseLogout(m_buffer);
            break;
        case (SMUG_LISTALBUMS):
            parseResponseListAlbums(m_buffer);
            break;
        case (SMUG_LISTPHOTOS):
            parseResponseListPhotos(m_buffer);
            break;
        case (SMUG_LISTALBUMTEMPLATES):
            parseResponseListAlbumTmpl(m_buffer);
            break;
        case (SMUG_LISTCATEGORIES):
            parseResponseListCategories(m_buffer);
            break;
        case (SMUG_LISTSUBCATEGORIES):
            parseResponseListSubCategories(m_buffer);
            break;
        case (SMUG_CREATEALBUM):
            parseResponseCreateAlbum(m_buffer);
            break;
        case (SMUG_ADDPHOTO):
            parseResponseAddPhoto(m_buffer);
            break;
        case (SMUG_GETPHOTO):
            // all we get is data of the image
            emit signalBusy(false);
            emit signalGetPhotoDone(0, QString(), m_buffer);
            break;
    }

    reply->deleteLater();
}

void SmugTalker::parseResponseLogin(const QByteArray& data)
{
    int errCode = -1;
    QString errMsg;

    emit signalLoginProgress(3);

    QDomDocument doc(QString::fromLatin1("login"));

    if (!doc.setContent(data))
        return;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Parse Login response:" << endl << data;

    QDomElement e = doc.documentElement();

    for (QDomNode node = e.firstChild();
         !node.isNull();
         node = node.nextSibling())
    {
        if (!node.isElement())
            continue;

        e = node.toElement();

        if (e.tagName() == QString::fromLatin1("Login"))
        {
            m_user.accountType   = e.attribute(QString::fromLatin1("AccountType"));
            m_user.fileSizeLimit = e.attribute(QString::fromLatin1("FileSizeLimit")).toInt();

            for (QDomNode nodeL = e.firstChild(); !nodeL.isNull(); nodeL = nodeL.nextSibling())
            {
                if (!nodeL.isElement())
                    continue;

                e = nodeL.toElement();

                if (e.tagName() == QString::fromLatin1("Session"))
                {
                    m_sessionID = e.attribute(QString::fromLatin1("id"));
                }
                else if (e.tagName() == QString::fromLatin1("User"))
                {
                    m_user.nickName    = e.attribute(QString::fromLatin1("NickName"));
                    m_user.displayName = e.attribute(QString::fromLatin1("DisplayName"));
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

    emit signalLoginProgress(4);

    if (errCode != 0) // if login failed, reset user properties
    {
        m_sessionID.clear();
        m_user.clear();
    }

    emit signalBusy(false);
    emit signalLoginDone(errCode, errorToText(errCode, errMsg));
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
    m_sessionID.clear();
    m_user.clear();

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
    int errCode = -1;
    QString errMsg;
    QDomDocument doc(QString::fromLatin1("createalbum"));

    if (!doc.setContent(data))
        return;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Parse Create Album response:" << endl << data;

    int newAlbumID = -1;
    QString newAlbumKey;
    QDomElement e  = doc.documentElement();

    for (QDomNode node = e.firstChild(); !node.isNull(); node = node.nextSibling())
    {
        if (!node.isElement())
            continue;

        e = node.toElement();

        if (e.tagName() == QString::fromLatin1("Album"))
        {
            newAlbumID  = e.attribute(QString::fromLatin1("id")).toLongLong();
            newAlbumKey = e.attribute(QString::fromLatin1("Key"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "AlbumID: " << newAlbumID;
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Key: " << newAlbumKey;
            errCode = 0;
        }
        else if (e.tagName() == QString::fromLatin1("err"))
        {
            errCode = e.attribute(QString::fromLatin1("code")).toInt();
            errMsg  = e.attribute(QString::fromLatin1("msg"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error:" << errCode << errMsg;
        }
    }

    emit signalBusy(false);
    emit signalCreateAlbumDone(errCode, errorToText(errCode, errMsg),
                               newAlbumID, newAlbumKey);
}

void SmugTalker::parseResponseListAlbums(const QByteArray& data)
{
    int errCode = -1;
    QString errMsg;
    QDomDocument doc(QString::fromLatin1("albums.get"));

    if (!doc.setContent(data))
        return;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Parse Albums response:" << endl << data;

    QList <SmugAlbum> albumsList;
    QDomElement e = doc.documentElement();

    for (QDomNode node = e.firstChild(); !node.isNull(); node = node.nextSibling())
    {
        if (!node.isElement())
            continue;

        e = node.toElement();

        if (e.tagName() == QString::fromLatin1("Albums"))
        {
            for (QDomNode nodeA = e.firstChild(); !nodeA.isNull(); nodeA = nodeA.nextSibling())
            {
                if (!nodeA.isElement())
                    continue;

                e = nodeA.toElement();

                if (e.tagName() == QString::fromLatin1("Album"))
                {
                    SmugAlbum album;
                    album.id           = e.attribute(QString::fromLatin1("id")).toLongLong();
                    album.key          = e.attribute(QString::fromLatin1("Key"));
                    album.title        = htmlToText(e.attribute(QString::fromLatin1("Title")));
                    album.description  = htmlToText(e.attribute(QString::fromLatin1("Description")));
                    album.keywords     = htmlToText(e.attribute(QString::fromLatin1("Keywords")));
                    album.isPublic     = e.attribute(QString::fromLatin1("Public")) == QString::fromLatin1("1");
                    album.password     = htmlToText(e.attribute(QString::fromLatin1("Password")));
                    album.passwordHint = htmlToText(e.attribute(QString::fromLatin1("PasswordHint")));
                    album.imageCount   = e.attribute(QString::fromLatin1("ImageCount")).toInt();

                    for (QDomNode nodeC = e.firstChild(); !nodeC.isNull(); nodeC = node.nextSibling())
                    {
                        if (!nodeC.isElement())
                            continue;

                        e = nodeC.toElement();

                        if (e.tagName() == QString::fromLatin1("Category"))
                        {
                            album.categoryID = e.attribute(QString::fromLatin1("id")).toLongLong();
                            album.category   = htmlToText(e.attribute(QString::fromLatin1("Name")));
                        }
                        else if (e.tagName() == QString::fromLatin1("SubCategory"))
                        {
                            album.subCategoryID = e.attribute(QString::fromLatin1("id")).toLongLong();
                            album.subCategory   = htmlToText(e.attribute(QString::fromLatin1("Name")));
                        }
                    }
                    albumsList.append(album);
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

    std::sort(albumsList.begin(), albumsList.end(), SmugAlbum::lessThan);

    emit signalBusy(false);
    emit signalListAlbumsDone(errCode, errorToText(errCode, errMsg), albumsList);
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
    int errCode = -1;
    QString errMsg;
    QDomDocument doc(QString::fromLatin1("albumtemplates.get"));

    if (!doc.setContent(data))
        return;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Parse AlbumTemplates response:" << endl << data;

    QList<SmugAlbumTmpl> albumTList;
    QDomElement e = doc.documentElement();

    for (QDomNode node = e.firstChild(); !node.isNull(); node = node.nextSibling())
    {
        if (!node.isElement())
            continue;

        e = node.toElement();

        if (e.tagName() == QString::fromLatin1("AlbumTemplates"))
        {
            for (QDomNode nodeT = e.firstChild(); !nodeT.isNull(); nodeT = nodeT.nextSibling())
            {
                if (!nodeT.isElement())
                    continue;

                QDomElement e = nodeT.toElement();

                if (e.tagName() == QString::fromLatin1("AlbumTemplate"))
                {
                    SmugAlbumTmpl tmpl;
                    tmpl.id           = e.attribute(QString::fromLatin1("id")).toLongLong();
                    tmpl.name         = htmlToText(e.attribute(QString::fromLatin1("AlbumTemplateName")));
                    tmpl.isPublic     = e.attribute(QString::fromLatin1("Public")) == QString::fromLatin1("1");
                    tmpl.password     = htmlToText(e.attribute(QString::fromLatin1("Password")));
                    tmpl.passwordHint = htmlToText(e.attribute(QString::fromLatin1("PasswordHint")));
                    albumTList.append(tmpl);
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
    emit signalListAlbumTmplDone(errCode, errorToText(errCode, errMsg), albumTList);
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

QString SmugTalker::htmlToText(const QString& htmlText)
{
    QTextDocument txtDoc;
    txtDoc.setHtml(htmlText);
    return txtDoc.toPlainText();
}

} // namespace Digikam

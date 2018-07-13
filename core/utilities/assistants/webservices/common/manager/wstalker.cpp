/* ============================================================
 * 
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-08
 * Description : Base class for web service talkers.
 *
 * Copyright (C) 2018 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */ 

#include "wstalker.h"

// Qt includes

#include <QApplication>
#include <QMessageBox>
#include <QObject>
#include <QDateTime>

// KDE includes

#include <klocalizedstring.h>


// Local includes

#include "digikam_debug.h"
#include "wswizard.h"
#include "o0globals.h"

namespace Digikam
{
    
WSTalker::WSTalker(QWidget* const parent)
  : QObject(parent),
    m_netMngr(new QNetworkAccessManager(this)),
    m_reply(0),
    m_state(WSTalker::DEFAULT),
    m_buffer(0),
    m_settings(dynamic_cast<WSWizard*>(parent)->settings()->oauthSettings),
    m_store(new O0SettingsStore(m_settings, QLatin1String(O2_ENCRYPTION_KEY), this)),
    m_userName(dynamic_cast<WSWizard*>(parent)->settings()->userName)
{
    connect(m_netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));
}

WSTalker::~WSTalker()
{
    if (m_reply)
    {
        m_reply->abort();
    }
    
    delete m_reply;
    delete m_netMngr;
    
    delete m_settings;
    delete m_store;
}

void WSTalker::cancel()
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = 0;
    }
    
    emit signalBusy(false);
}

QString WSTalker::getUserID(const QString& userName)
{
    QString userID;
    
    if(!userName.isEmpty())
    {
        m_settings->beginGroup(m_store->groupKey());
        m_settings->beginGroup("users");
        userID = m_settings->value(userName).toString();
        m_settings->endGroup();
        m_settings->endGroup();
    }
    
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "ID of user " << userName << " : " << userID;
    return userID;
}

void WSTalker::link()
{
}

void WSTalker::unlink()
{
}

bool WSTalker::linked() const
{
    return false;
}

void WSTalker::authenticate()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "username chosen: " << m_userName;
    bool authenticateValide = loadUserAccount(m_userName);
    
    /* If user account already exists and doesn't expire yet (authenticateValide == true), linking to his account
     * Otherwise, unlink() current account and link to new account
     */
    if(authenticateValide)
    {
        link();
    }
    else
    {
        reauthenticate();
    } 
}

void WSTalker::reauthenticate()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "reauthenticate";
    
    unlink();
    
    // Wait until user account is unlinked completely
    while(linked());
    
    link();
}

QMap<QString, QVariant> WSTalker::getUserAccountInfo(const QString& userName)
{
    QString userID = getUserID(userName);
    
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "getUserAccountInfo with userID: " << userID;
    
    QMap<QString, QVariant> map;
    if(userID.isEmpty())
    {
        return map;
    }
    
    m_settings->beginGroup(m_store->groupKey());
    m_settings->beginGroup(userID);
    QStringList keys = m_settings->allKeys();
    foreach(const QString& key, keys)
    {
        QVariant value = m_settings->value(key);
        map.insert(key, value);
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << key << "(key), " << value.toString() << "(value)"; 
    }
    m_settings->endGroup();
    m_settings->endGroup();
    
    return map;
}

void WSTalker::saveUserAccount(const QString& userName,
                               const QString& userID,
                               long long int expire,
                               const QString& accessToken, 
                               const QString& refreshToken)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "saveUserAccount with username : " << userName << ", userID: " << userID;
    
    if(userID.isEmpty())
    {
        return; 
    }
    
    m_settings->beginGroup(m_store->groupKey());
    
    m_settings->beginGroup("users");
    m_settings->setValue(userName, userID);
    m_settings->endGroup();
    
    m_settings->beginGroup(userID);
    m_settings->setValue("expiration_time", expire);
    m_settings->setValue("access_token", accessToken);
    m_settings->setValue("refresh_token", refreshToken);
    m_settings->endGroup();
    
    m_settings->endGroup();
    
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "current " << QDateTime::currentMSecsSinceEpoch() / 1000;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "expire " << expire; 
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "access_token" << accessToken;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "refresh_token" << refreshToken;
}

bool WSTalker::loadUserAccount(const QString& userName)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "loadUserAccount with user name : " << userName;
    
    /* User logins using new account, return false so that we can unlink() current account, 
     * before link() to new account
     */
    if(userName.isEmpty())
    {
        return false;
    }
    
    QMap<QString, QVariant> map = getUserAccountInfo(userName);
    
    /* If user exists but does not login yet, return false so that we can unlink() current account, 
     * before link() to new account
     */
    if(map.isEmpty())
    {
        return false;
    }
    
    QString expire        = map["expiration_time"].toString();
    QString accessToken   = map["access_token"].toString();
    QString refreshToken  = map["refresh_token"].toString();
    
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "expired moment : " << expire;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "current time : " << QDateTime::currentMSecsSinceEpoch() / 1000;
    
    /* If access token is not expired yet, retrieve all tokens and return true so that we can link() 
     * directly to new account.
     */
    if(expire.toLongLong() > QDateTime::currentMSecsSinceEpoch() / 1000)
    {
        resetTalker(expire, accessToken, refreshToken);
        return true;
    }
    
    return false;
}

void WSTalker::resetTalker(const QString& expire, const QString& accessToken, const QString& refreshToken)
{
}

void WSTalker::removeUserAccount(const QString& userName)
{
    QString userID = getUserID(userName);
    
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "removeUserAccount with userID: " << userID;
    
    if(userID.isEmpty())
    {
       return; 
    }
    
    m_settings->beginGroup(m_store->groupKey());
    
    m_settings->beginGroup(userID);
    m_settings->remove("");
    m_settings->endGroup();
    
    m_settings->beginGroup("users");
    m_settings->remove(userName);
    m_settings->endGroup();
    
    m_settings->endGroup();  
}

void WSTalker::removeAllAccounts()
{
    m_settings->beginGroup(m_store->groupKey());
    m_settings->remove("");
    m_settings->endGroup();
}

void WSTalker::parseResponseGetLoggedInUser(const QByteArray& data)
{
}

void WSTalker::parseResponseListAlbums(const QByteArray& data)
{
}

void WSTalker::parseResponseCreateAlbum(const QByteArray& data)
{
}

void WSTalker::parseResponseAddPhoto(const QByteArray& data)
{
}

void WSTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != m_reply)
    {
        return;
    }
    
    m_reply = 0;
    
    if (reply->error() != QNetworkReply::NoError)
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << reply->error() << " text :"<< QString(reply->readAll());
        
        emit signalBusy(false);
        QMessageBox::critical(QApplication::activeWindow(),
                              i18n(QString::fromLatin1("Error %1").arg(reply->error()).toLatin1()),
                              reply->errorString());
        
        reply->deleteLater();
        return;
    }
    
    m_buffer.append(reply->readAll());
    
    switch(m_state)
    {
        case WSTalker::GETUSER :
            parseResponseGetLoggedInUser(m_buffer);
            break;
        case WSTalker::LISTALBUMS :
            parseResponseListAlbums(m_buffer);
            break;
        case WSTalker::CREATEALBUM :
            parseResponseCreateAlbum(m_buffer);
            break;
        case WSTalker::ADDPHOTO :
            parseResponseAddPhoto(m_buffer);
            break;
        case WSTalker::DEFAULT :
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "slotFinished at state = default";
            break;
    }
    
    reply->deleteLater();    
}

void WSTalker::slotOpenBrowser(const QUrl& url)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Open Browser...";
    emit signalOpenBrowser(url);
}

void WSTalker::slotCloseBrowser()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Close Browser...";
    emit signalCloseBrowser();
}

void WSTalker::slotResponseTokenReceived(const QMap<QString, QString>& rep)
{
}

} // Digikam

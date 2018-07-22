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

#ifndef DIGIKAM_WS_TALKER_H
#define DIGIKAM_WS_TALKER_H

// Qt includes

#include <QtGlobal>
#include <QList>
#include <QPair>
#include <QString>
#include <QUrl>
#include <QWidget>
#include <QSettings>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QMap>

// Local includes

#include "o0settingsstore.h"

namespace Digikam
{
    
class WSTalker : public QObject
{
    Q_OBJECT
    
public:
    
    enum State
    {
        DEFAULT = 0,
        GETUSER,
        LISTALBUMS,
        CREATEALBUM,
        ADDPHOTO
    };
    
public:
    
    explicit WSTalker(QWidget* const parent);
    ~WSTalker();
    
    QString getUserID(const QString& userName);
    
    virtual void link();
    virtual void unlink();
    virtual bool linked() const;
    
    virtual void authenticate();
    void reauthenticate();
    
    void cancel();
    
protected:
    
    QMap<QString, QVariant> getUserAccountInfo(const QString& userName);
    
    void saveUserAccount(const QString& userName,
                         const QString& userID,
                         long long int expire,
                         const QString& accessToken, 
                         const QString& refreshToken = QString());
    void removeUserAccount(const QString& userName);
    bool loadUserAccount(const QString& userName);
    void removeAllAccounts();
    
    virtual void resetTalker(const QString& expire, const QString& accessToken, const QString& refreshToken);
    
    virtual void getLoggedInUser();

    virtual void parseResponseGetLoggedInUser(const QByteArray& data);
    virtual void parseResponseListAlbums(const QByteArray& data);
    virtual void parseResponseCreateAlbum(const QByteArray& data);
    virtual void parseResponseAddPhoto(const QByteArray& data);
    
Q_SIGNALS:
    
    void signalBusy(bool val);
    void signalOpenBrowser(const QUrl& url);
    void signalCloseBrowser();
    void signalAuthenticationComplete(bool);
    
protected Q_SLOTS:
    
    void slotFinished(QNetworkReply* reply);
    void slotOpenBrowser(const QUrl& url);
    void slotCloseBrowser();
    virtual void slotLinkingFailed();
    virtual void slotLinkingSucceeded();
    virtual void slotResponseTokenReceived(const QMap<QString, QString>& rep);

protected:
    
    QNetworkAccessManager*  m_netMngr;
    QNetworkReply*          m_reply;
    
    State                   m_state;
    QByteArray              m_buffer;
    
    QSettings*              m_settings;
    O0SettingsStore*        m_store;
    
    QString                 m_userName;
};
    
} // namespace Digikam

#endif // DIGIKAM_WS_TALKER_H  




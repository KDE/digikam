/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-21
 * Description : a tool to export items to Google web services
 *
 * Copyright (C) 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * Copyright (C) 2015-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#ifndef GS_SESSION_H
#define GS_SESSION_H

// Qt includes

#include <QList>
#include <QString>
#include <QObject>
#include <QStringList>
#include <QDialog>
#include <QNetworkReply>
#include <QNetworkAccessManager>

namespace Digikam
{

class GSSession : public QObject
{
    Q_OBJECT

public:

    explicit GSSession(QWidget* const parent, const QString& scope);
    ~GSSession();

Q_SIGNALS:

    void signalBusy(bool val);
    void signalAccessTokenFailed(int errCode, const QString& errMsg);
    void signalAccessTokenObtained();
    void signalTextBoxEmpty();
    void signalRefreshTokenObtained(const QString& msg);

private Q_SLOTS:

    void slotAuthFinished(QNetworkReply* reply);
    void slotAccept();
    void slotReject();

public:

    void doOAuth();
    void getAccessToken();
    void getAccessTokenFromRefreshToken(const QString& msg);
    bool authenticated();

    QString     getValue(const QString&, const QString&);
    QStringList getParams(const QString&, const QStringList&, const QString&);
    QString     getToken(const QString&, const QString&, const QString&);
    int         getTokenEnd(const QString&, int);

protected:

    QString                m_scope;
    QString                m_accessToken;
    QString                m_refreshToken;

    QString                m_bearerAccessToken;
    QByteArray             m_buffer;

    QNetworkReply*         m_reply;

private:

    void parseResponseAccessToken(const QByteArray& data);
    void parseResponseRefreshToken(const QByteArray& data);

private:

    enum Auth_State
    {
        GD_ACCESSTOKEN=0,
        GD_REFRESHTOKEN
    };

private:

    int                    m_continuePos;

    QWidget*               m_parent;
    
    Auth_State             m_authState;
    QString                m_tokenUri;
    QString                m_clientSecret;
    QString                m_clientId;
    QString                m_responseType;
    QString                m_redirectUri;
    QString                m_code;

    QDialog*               m_window;
    
    QNetworkAccessManager* m_netMngr;
};

} // namespace Digikam

#endif // GS_SESSION_H

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

#ifndef GS_TALKER_BASE_H
#define GS_TALKER_BASE_H

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

class GSTalkerBase : public QObject
{
    Q_OBJECT

public:

    explicit GSTalkerBase(QWidget* const parent, const QString& scope);
    ~GSTalkerBase();

public:

    void        doOAuth();
    void        getAccessToken();
    void        getAccessTokenFromRefreshToken(const QString& msg);
    bool        authenticated()                                               const;
    QString     getValue(const QString&, const QString&)                      const;
    QStringList getParams(const QString&, const QStringList&, const QString&) const;
    QString     getToken(const QString&, const QString&, const QString&)      const;
    int         getTokenEnd(const QString&, int)                              const;

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

protected:

    QString        m_scope;
    QString        m_accessToken;
    QString        m_refreshToken;

    QString        m_bearerAccessToken;
    QByteArray     m_buffer;

    QNetworkReply* m_reply;

private:

    void parseResponseAccessToken(const QByteArray& data);
    void parseResponseRefreshToken(const QByteArray& data);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // GS_TALKER_BASE_H

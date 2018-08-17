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

#ifndef DIGIKAM_GS_TALKER_BASE_H
#define DIGIKAM_GS_TALKER_BASE_H

// Qt includes

#include <QList>
#include <QString>
#include <QObject>
#include <QStringList>
#include <QDialog>
#include <QNetworkReply>
#include <QNetworkAccessManager>

// O2 includes

#include "o2.h"

namespace Digikam
{

class GSTalkerBase : public QObject
{
    Q_OBJECT

public:

    explicit GSTalkerBase(QWidget* const parent, const QStringList& scope, const QString& serviceName);
    ~GSTalkerBase();

public:

    void        link();
    void        unlink();
    void        doOAuth();
    bool        authenticated() const;

Q_SIGNALS:

    void signalBusy(bool val);
    void signalLinkingSucceeded();
    void signalAccessTokenObtained();
    void signalAuthenticationRefused();

private Q_SLOTS:

    void slotLinkingSucceeded();
    void slotLinkingFailed();
    void slotOpenBrowser(const QUrl&);

private:

    void parseResponseGetLoggedInUser(const QByteArray& data);

protected:

    QStringList    m_scope;
    QString        m_accessToken;

    QString        m_bearerAccessToken;
    QByteArray     m_buffer;

    QNetworkReply* m_reply;
    QString        m_serviceName;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_GS_TALKER_BASE_H

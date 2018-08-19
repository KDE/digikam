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

#include "gstalkerbase.h"

// Qt includes

#include <QByteArray>
#include <QtAlgorithms>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QList>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QPair>
#include <QFileInfo>
#include <QDebug>
#include <QApplication>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrlQuery>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "gdmpform.h"
#include "digikam_debug.h"

// O2 includes

#include "o0globals.h"
#include "o0settingsstore.h"
#include "wstoolutils.h"

namespace Digikam
{

class GSTalkerBase::Private
{
public:

    explicit Private()
    {
        parent          = 0;

        apikey          = QLatin1String("258540448336-hgdegpohibcjasvk1p595fpvjor15pbc.apps.googleusercontent.com");
        clientSecret    = QLatin1String("iiIKTNM4ggBXiTdquAzbs2xw");
        /* Old api key and secret below only work for gdrive, not gphoto
         * Switch to new api key and secret above
         * apikey       = QLatin1String("735222197981-mrcgtaqf05914buqjkts7mk79blsquas.apps.googleusercontent.com");
         * clientSecret = QLatin1String("4MJOS0u1-_AUEKJ0ObA-j22U");
         */

        authUrl         = QLatin1String("https://accounts.google.com/o/oauth2/auth");
        tokenUrl        = QLatin1String("https://accounts.google.com/o/oauth2/token");
        refreshUrl      = QLatin1String("https://accounts.google.com/o/oauth2/token");

        o2              = 0;
        settings        = 0;
    }

    QWidget*   parent;

    QString    authUrl;
    QString    tokenUrl;
    QString    refreshUrl;

    QString    apikey;
    QString    clientSecret;

    O2*        o2;
    QSettings* settings;
};

GSTalkerBase::GSTalkerBase(QWidget* const parent, const QStringList & scope, const QString& serviceName)
    : d(new Private)
{
    m_reply         = 0;
    m_scope         = scope;
    m_serviceName   = serviceName;
    d->parent       = parent;

    // Ported to O2
    d->o2 = new O2(this);
    d->o2->setClientId(d->apikey);
    d->o2->setClientSecret(d->clientSecret);

    // OAuth2 flow control
    d->o2->setRequestUrl(d->authUrl);
    d->o2->setTokenUrl(d->tokenUrl);
    d->o2->setRefreshTokenUrl(d->refreshUrl);
    d->o2->setLocalPort(8000);
    d->o2->setGrantFlow(O2::GrantFlow::GrantFlowAuthorizationCode);
    d->o2->setScope(m_scope.join(" "));

    // OAuth configuration saved to between dk sessions
    d->settings                  = WSToolUtils::getOauthSettings(this);
    O0SettingsStore* const store = new O0SettingsStore(d->settings, QLatin1String(O2_ENCRYPTION_KEY), this);
    store->setGroupKey(m_serviceName);
    d->o2->setStore(store);

    // Refresh token permission when offline
    QMap<QString, QVariant> extraParams;
    extraParams.insert("access_type", "offline");
    d->o2->setExtraRequestParams(extraParams);

    connect(d->o2, SIGNAL(linkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));

    connect(this, SIGNAL(signalLinkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));

    connect(d->o2, SIGNAL(linkingFailed()),
            this, SLOT(slotLinkingFailed()));

    connect(d->o2, SIGNAL(openBrowser(QUrl)),
            this, SLOT(slotOpenBrowser(QUrl)));

}

GSTalkerBase::~GSTalkerBase()
{
    if (m_reply)
        m_reply->abort();

    delete d;
}

void GSTalkerBase::link()
{
    emit signalBusy(true);
    d->o2->link();
}

void GSTalkerBase::unlink()
{
    emit signalBusy(true);

    d->o2->unlink();

    d->settings->beginGroup(m_serviceName);
    d->settings->remove("");
    d->settings->endGroup();

    m_bearerAccessToken.clear();
    m_accessToken.clear();
}

void GSTalkerBase::slotLinkingSucceeded()
{
    if (!d->o2->linked())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UNLINK to " << m_serviceName << " ok";
        emit signalBusy(false);
        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to " << m_serviceName << " ok";

    m_accessToken = d->o2->token();
    m_bearerAccessToken = QLatin1String("Bearer ") + m_accessToken;

    emit signalAccessTokenObtained();
}

void GSTalkerBase::slotLinkingFailed()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to " << m_serviceName << " fail";

    emit signalBusy(false);
    emit signalAuthenticationRefused();
}

void GSTalkerBase::slotOpenBrowser(const QUrl& url)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Open browser...";
    QDesktopServices::openUrl(url);;
}

bool GSTalkerBase::authenticated() const
{
    return d->o2->linked();
}

void GSTalkerBase::doOAuth()
{
    int sessionExpires = d->o2->expires();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "current time " << QDateTime::currentMSecsSinceEpoch() / 1000;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "expires at : " << sessionExpires;

    /**
    * If user has not logined yet (sessionExpires == 0), link
    * If access token has expired yet, refresh
    * TODO: Otherwise, provoke slotLinkingSucceeded
    */
    if (sessionExpires == 0)
    {
        link();
    }
    else
    {
        d->o2->refresh();
    }
}

} // namespace Digikam

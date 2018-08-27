/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
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

#include "mediawiki_logout.h"

// Qt includes

#include <QDateTime>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>
#include <QNetworkCookie>
#include <QNetworkReply>
#include <QNetworkRequest>

// Local includes

#include "mediawiki_iface.h"
#include "mediawiki_job_p.h"

namespace mediawiki
{

class LogoutPrivate : public JobPrivate
{
public:

    LogoutPrivate(Iface& mediawiki)
        : JobPrivate(mediawiki)
    {
    }
};

Logout::Logout(Iface& mediawiki, QObject* const parent)
    : Job(*new LogoutPrivate(mediawiki), parent)
{
}

Logout::~Logout()
{
}

void Logout::start()
{
    QTimer::singleShot(0, this, SLOT(doWorkSendRequest()));
}

void Logout::doWorkSendRequest()
{
    Q_D(Logout);

    QUrl url = d->mediawiki.url();
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("xml"));
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("logout"));
    url.setQuery(query);

    QByteArray cookie = "";
    QList<QNetworkCookie> mediawikiCookies = d->manager->cookieJar()->cookiesForUrl(d->mediawiki.url());

    for (int i = 0 ; i < mediawikiCookies.size() ; ++i)
    {
        cookie += mediawikiCookies.at(i).toRawForm(QNetworkCookie::NameAndValueOnly);
        cookie += ';';
    }

    // Set the request
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", d->mediawiki.userAgent().toUtf8());
    request.setRawHeader("Cookie", cookie);

    // Delete cookies
    d->manager->setCookieJar(new QNetworkCookieJar);

    // Send the request
    d->reply = d->manager->get(request);
    connectReply();

    connect(d->reply, SIGNAL(finished()),
            this, SLOT(doWorkProcessReply()));
}

void Logout::doWorkProcessReply()
{
    Q_D(Logout);

    disconnect(d->reply, SIGNAL(finished()),
               this, SLOT(doWorkProcessReply()));

    this->setError(KJob::NoError);
    d->reply->close();
    d->reply->deleteLater();
    emitResult();
}

} // namespace mediawiki

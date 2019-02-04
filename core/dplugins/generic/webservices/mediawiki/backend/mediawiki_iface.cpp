/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Remi Benoit <r3m1 dot benoit at gmail dot com>
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

#include "mediawiki_iface.h"
#include "mediawiki_iface_p.h"

// Qt includes

#include <QString>

namespace MediaWiki
{

Iface::Iface(const QUrl& url, const QString& customUserAgent)
    : d(new Private(url,
            (customUserAgent.isEmpty() ? QString()
                                       : QString(customUserAgent +
                                         QStringLiteral("-")))   +
                                         Private::POSTFIX_USER_AGENT,
            new QNetworkAccessManager()))
{
}

Iface::~Iface()
{
    delete d;
}

QUrl Iface::url() const
{
    return d->url;
}

QString Iface::userAgent() const
{
    return d->userAgent;
}

QNetworkAccessManager* Iface::manager() const
{
    return d->manager;
}

} // namespace MediaWiki

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a MediaWiki C++ interface
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "mediawiki_mediawiki.h"
#include "mediawiki_mediawiki_p.h"

// Qt includes

#include <QString>

namespace mediawiki
{

MediaWiki::MediaWiki(const QUrl& url, const QString& customUserAgent)
    : d_ptr(new MediaWikiPrivate(url,
                                 (customUserAgent.isEmpty() ? QString() 
                                                            : QString(customUserAgent + QStringLiteral("-"))) + MediaWikiPrivate::POSTFIX_USER_AGENT,
                                 new QNetworkAccessManager()))
{
}

MediaWiki::~MediaWiki()
{
    delete d_ptr;
}

QUrl MediaWiki::url() const
{
    return d_ptr->url;
}

QString MediaWiki::userAgent() const
{
    return d_ptr->userAgent;
}

QNetworkAccessManager* MediaWiki::manager() const
{
    return d_ptr->manager;
}

} // namespace mediawiki

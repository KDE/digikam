/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
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

#ifndef MEDIAWIKI_MEDIAWIKI_H
#define MEDIAWIKI_MEDIAWIKI_H

// Qt includes

#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>

// Local includes

#include "digikam_export.h"

namespace mediawiki
{

/**
 * @brief Provides access to wiki powered by Iface.
 */
class DIGIKAM_EXPORT Iface
{
public:

    /**
     * @brief Constructs a Iface by its url api.
     * @param url the url api of the wiki
     * @param customUserAgent you can specify the user agent to use
                              which will be concatenated with the postfix user agent
     *                        else the postfix user agent is used only
     */
    explicit Iface(const QUrl& url, const QString& customUserAgent = QString());

    /**
     * @brief Destructs the Iface.
     */
    ~Iface();

    /**
     * @brief Returns the url api of the wiki.
     * @returns the url api of the wiki
     */
    QUrl url() const;

    /**
     * @brief Returns the user agent of the wiki.
     * @return the user agent of the wiki
     */
    QString userAgent() const;

    /**
     * @brief Returns the network manager instance of the wiki.
     * @return the network manager instance of the wiki
     */
    QNetworkAccessManager* manager() const;

private:

    class Private;
    Private* const d;

    friend class JobPrivate;
};

} // namespace mediawiki

#endif // MEDIAWIKI_MEDIAWIKI_H

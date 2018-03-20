/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShackSession web service
 *
 * Copyright (C) 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * Copyright (C) 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#include "imageshacksession.h"

// Qt includes

#include <QString>
#include <QApplication>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>

// Locla includes

#include "digikam_debug.h"

namespace Digikam
{

class ImageShackSession::Private
{

public:

    explicit Private()
    {
        loggedIn = false;
    }

    bool    loggedIn;

    QString authToken;
    QString username;
    QString email;
    QString password;
    QString credits;
};

ImageShackSession::ImageShackSession()
    : d(new Private)
{
    readSettings();
}

ImageShackSession::~ImageShackSession()
{
    delete d;
}

bool ImageShackSession::loggedIn() const
{
    return d->loggedIn;
}

QString ImageShackSession::username() const
{
    return d->username;
}

QString ImageShackSession::email() const
{
    return d->email;
}

QString ImageShackSession::password() const
{
    return d->password;
}

QString ImageShackSession::authToken() const
{
    return d->authToken;
}

QString ImageShackSession::credits() const
{
    return d->credits;
}

void ImageShackSession::setLoggedIn(bool b)
{
    d->loggedIn = b;
}

void ImageShackSession::setUsername(const QString& username)
{
    d->username = username;
}

void ImageShackSession::setEmail(const QString& email)
{
    d->email = email;
}

void ImageShackSession::setAuthToken(const QString& token)
{
    d->authToken = token;
}

void ImageShackSession::setPassword(const QString& pass)
{
    d->password = pass;
}

void ImageShackSession::setCredits(const QString& credits)
{
    d->credits = credits;
}

void ImageShackSession::logOut()
{
    d->loggedIn = false;
    d->username.clear();
    d->email.clear();
    d->credits.clear();
    saveSettings();
}

void ImageShackSession::readSettings()
{
    static bool bLoaded = false;

    if (bLoaded)
        return;

    bLoaded = true;

    KConfig config;
    KConfigGroup group = config.group("ImageShack Settings");
}

void ImageShackSession::saveSettings()
{
    KConfig config;
    KConfigGroup group = config.group("ImageShack Settings");

    config.sync();
}

} // namespace Digikam

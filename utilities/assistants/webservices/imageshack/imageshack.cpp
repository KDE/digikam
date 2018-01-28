/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * Copyright (C) 2012 Dodon Victor <dodonvictor at gmail dot com>
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

#include "imageshack.h"

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

ImageShack::ImageShack()
{
    readSettings();
    m_loggedIn = false;
}

ImageShack::~ImageShack()
{
}

bool ImageShack::loggedIn() const
{
    return m_loggedIn;
}

QString ImageShack::username() const
{
    return m_username;
}

QString ImageShack::email() const
{
    return m_email;
}

QString ImageShack::password() const
{
    return m_password;
}

QString ImageShack::authToken() const
{
    return m_authToken;
}

QString ImageShack::credits() const
{
    return m_credits;
}

void ImageShack::setUsername(const QString& username)
{
    m_username = username;
}

void ImageShack::setEmail(const QString& email)
{
    m_email = email;
}

void ImageShack::setAuthToken(const QString& token)
{
    m_authToken = token;
}

void ImageShack::setPassword(const QString& pass)
{
    m_password = pass;
}

void ImageShack::logOut()
{
    m_loggedIn = false;
    m_username.clear();
    m_email.clear();
    m_credits.clear();
    saveSettings();
}

void ImageShack::readSettings()
{
    static bool bLoaded = false;
    if (bLoaded) return;
    bLoaded = true;

    KConfig config;
    KConfigGroup group = config.group("ImageShack Settings");
}

void ImageShack::saveSettings()
{
    KConfig config;
    KConfigGroup group = config.group("ImageShack Settings");

    config.sync();
}

} // namespace Digikam

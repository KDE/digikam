/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShackSession web service
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

ImageShackSession::ImageShackSession()
{
    readSettings();
    m_loggedIn = false;
}

ImageShackSession::~ImageShackSession()
{
}

bool ImageShackSession::loggedIn() const
{
    return m_loggedIn;
}

QString ImageShackSession::username() const
{
    return m_username;
}

QString ImageShackSession::email() const
{
    return m_email;
}

QString ImageShackSession::password() const
{
    return m_password;
}

QString ImageShackSession::authToken() const
{
    return m_authToken;
}

QString ImageShackSession::credits() const
{
    return m_credits;
}

void ImageShackSession::setUsername(const QString& username)
{
    m_username = username;
}

void ImageShackSession::setEmail(const QString& email)
{
    m_email = email;
}

void ImageShackSession::setAuthToken(const QString& token)
{
    m_authToken = token;
}

void ImageShackSession::setPassword(const QString& pass)
{
    m_password = pass;
}

void ImageShackSession::logOut()
{
    m_loggedIn = false;
    m_username.clear();
    m_email.clear();
    m_credits.clear();
    saveSettings();
}

void ImageShackSession::readSettings()
{
    static bool bLoaded = false;
    if (bLoaded) return;
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

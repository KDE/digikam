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

#ifndef IMAGE_SHACK_H
#define IMAGE_SHACK_H

// Qt includes

#include <QString>

namespace Digikam
{

class ImageShack
{

public:

    explicit ImageShack();
    ~ImageShack();

    bool    loggedIn()  const;
    QString username()  const;
    QString email()     const;
    QString password()  const;
    QString credits()   const;
    QString authToken() const;

    void setUsername(const QString& username);
    void setEmail(const QString& email);
    void setPassword(const QString& pass);
    void setAuthToken(const QString& token);

    void readSettings();

    void logOut();

private:

    void saveSettings();

private:

    bool    m_loggedIn;

    QString m_authToken;
    QString m_username;
    QString m_email;
    QString m_password;
    QString m_credits;

    friend class ImageShackWindow;
    friend class ImageShackWidget;
    friend class ImageShackTalker;

};

} // namaspace Digikam

#endif // IMAGE_SHACK_H

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-14
 * Description : Yandex authentication module
 *
 * Copyright (C) 2010      by Roman Tsisyk <roman at tsisyk dot com>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "yfauth.h"

// C++ includes

#include <cstdlib> // size_t

// Qt includes

#include <QString>
#include <QByteArray>

// Local includes

#include "yfrsa.h" // CCryptoProviderRSA

namespace YFAuth
{

QString makeCredentials(const QString& publicKey,
                        const QString& login,
                        const QString& password)
{
    // prepare string
    QByteArray credentials = "<credentials login=\"";
    credentials.append(login.toLocal8Bit());
    credentials.append("\" password=\"");
    credentials.append(password.toLocal8Bit());
    credentials.append("\"/>");

    QByteArray encrypted;
    encrypted.resize(MAX_CRYPT_BITS);
    size_t len = 0;

    // encrypt with Yandex RSA
    CCryptoProviderRSA rsaEncrypter;
    rsaEncrypter.ImportPublicKey(publicKey.toLocal8Bit().constData());
    rsaEncrypter.Encrypt(credentials.constData(), credentials.size(),
                         encrypted.data(), len);

    // small checks
    if (len < MAX_CRYPT_BITS)
    {
        encrypted.resize(static_cast<int>(len));
    }

    // encode with base64

    QByteArray ba;
    ba.append(encrypted);

    return QString::fromLatin1(ba.toBase64());
}

} // namespace YFAuth

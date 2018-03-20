/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-12
 * Description : Web Service tool utils methods
 *
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef WS_EXPORT_UTILS_H
#define WS_EXPORT_UTILS_H

// Qt includes

#include <QSettings>
#include <QString>
#include <QDir>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT WSToolUtils
{
public:

    explicit WSToolUtils() {};
    ~WSToolUtils()         {};

    /** Generates random string.
     */
    static QString randomString(const int& length);

    static QDir makeTemporaryDir(const char* prefix);
    static void removeTemporaryDir(const char* prefix);

    static QSettings* getOauthSettings(QObject* const parent);
};

} // namespace Digikam

#endif // WS_EXPORT_UTILS_H

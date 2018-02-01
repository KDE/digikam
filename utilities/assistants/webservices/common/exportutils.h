/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-12
 * Description : Export tool utils methods
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

#ifndef EXPORT_UTILS_H
#define EXPORT_UTILS_H

// Qt includes

#include <QString>
#include <QDir>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ExportUtils
{
public:

    explicit ExportUtils() {};
    ~ExportUtils()         {};

    /** Generates random string.
     */
    static QString randomString(const int& length);

    static QDir makeTemporaryDir(const char* prefix);
    static void removeTemporaryDir(const char* prefix);
};

} // namespace Digikam

#endif // EXPORT_UTILS_H

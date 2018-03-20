/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-06-20
 * Description : Autodetects Mysql initializer binary program and version
 *
 * Copyright (C) 2016-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef MYSQL_INIT_BINARY_H
#define MYSQL_INIT_BINARY_H

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dbinaryiface.h"
#include "dbengineparameters.h"

namespace Digikam
{

class MysqlInitBinary : public DBinaryIface
{

public:

    MysqlInitBinary()
        : DBinaryIface(DbEngineParameters::defaultMysqlInitCmd(),
                       QLatin1String("MariaDB"),
                       QLatin1String("https://mariadb.org/download/"),
                       QString(),
                       QStringList(QLatin1String("--help")),
                       i18n("This binary file is used to initialize the MariaDB data files for the database.")
                      )
        {
            setup();
        }

    ~MysqlInitBinary()
    {
    }
};

} // namespace Digikam

#endif // MYSQL_INIT_BINARY_H

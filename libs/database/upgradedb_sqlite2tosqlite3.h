/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-05
 * Description : SQlite 2 to SQlite 3 interface
 *
 * Copyright (C) 2005 by Renchi Raju <renchi dot raju at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef UPGRADEDB_SQLITE2TOSQLITE3_H
#define UPGRADEDB_SQLITE2TOSQLITE3_H

// Qt includes

#include <QString>

// Local includes

#include "databaseaccess.h"

namespace Digikam
{

extern bool upgradeDB_Sqlite2ToSqlite3(AlbumDB* albumDB, DatabaseBackend* backend, const QString& sql2DBPath);

}  // namespace Digikam

#endif /* UPGRADEDB_SQLITE2TOSQLITE3_H */

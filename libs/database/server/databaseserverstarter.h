/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-08
 * Description : database server starter
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
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

#ifndef DATABASE_SERVER_STARTER_H_
#define DATABASE_SERVER_STARTER_H_

// Qt includes

#include <QProcess>
#include <QString>
#include <QCoreApplication>

// Local includes

#include "digikam_export.h"
#include "databaseservererror.h"
#include "dbengineparameters.h"

namespace Digikam
{

class DIGIKAM_EXPORT DatabaseServerStarter : public QObject
{
    Q_OBJECT

public:

    explicit DatabaseServerStarter(QObject* const parent=0);
    static DatabaseServerError startServerManagerProcess(const QString& dbType = DbEngineParameters::MySQLDatabaseType());
    static void stopServerManagerProcess();
    static void cleanUp();

private:

    static bool isServerRegistered();
    static bool __init;
    static bool init();
};

}  // namespace Digikam

#endif /* DATABASE_SERVER_STARTER_H_ */

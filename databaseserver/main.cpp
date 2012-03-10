/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-07
 * Description : start class for database server manager
 *
 * Copyright (C) 2010-2011 by Holger Foerster <Hamsi2k at freenet dot de>
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

// Qt includes

#include <QApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QDBusReply>

// Local includes

#include "databaseserver.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    DatabaseServer server(&app);
    server.registerOnDBus();
    server.startPolling();

    return app.exec();
}

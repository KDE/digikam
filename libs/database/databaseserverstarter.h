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

#ifndef DATABASESERVER_H_
#define DATABASESERVER_H_

// QT includes

#include <QProcess>
#include <QString>
#include <QDBusAbstractAdaptor>
#include <QCoreApplication>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT DatabaseServerStarter : public QObject
{
    Q_OBJECT

    public:

        DatabaseServerStarter(QObject* parent);
        static void startServerManagerProcess(const QString dbType="QMYSQL");

    private:

        static bool isServerRegistered();
};

}  // namespace Digikam

#endif /* DATABASESERVER_H_ */

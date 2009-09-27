/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-27
 * Description : gui database error handler
 *
 * Copyright (C) 2009 by Holger Foerster <Hamsi2k at freenet dot de>
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

#ifndef DATABASEGUIERRORHANDLER_H
#define DATABASEGUIERRORHANDLER_H

// QT includes
#include <qthread.h>

// KDE includes
#include <kprogressdialog.h>

// Local includes
#include "databasecorebackend.h"

namespace Digikam
{

class DatabaseConnectionChecker : public QThread
{
    Q_OBJECT

    public:
        DatabaseConnectionChecker(const DatabaseParameters parameters);
        bool stop;
        void run();
        void setDialog(QDialog* dialog);

    private:
        QDialog* dialog;
        DatabaseParameters parameters;

    Q_SIGNALS:
        void done();
};

class DatabaseGUIErrorHandler : public DatabaseErrorHandler
{
    Q_OBJECT

public:

    DatabaseGUIErrorHandler(const DatabaseParameters parameters);
    ~DatabaseGUIErrorHandler();


private:
    DatabaseParameters parameters;
    bool               refuseQueries;

private Q_SLOTS:
    virtual void connectionError(DatabaseErrorAnswer *answer);

};

}  // namespace Digikam

#endif /* DATABASEGUIERRORHANDLER_H */

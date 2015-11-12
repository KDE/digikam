/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
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

#ifndef DATABASECOPYMANAGER_H
#define DATABASECOPYMANAGER_H

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"
#include "databasebackend.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT DatabaseCopyManager : public QObject
{
    Q_OBJECT

public:

    enum FinishStates
    {
        success,
        failed,
        canceled
    };

public:

    DatabaseCopyManager();
    ~DatabaseCopyManager();

    void copyDatabases(DatabaseParameters fromDBParameters, DatabaseParameters toDBParameters);

Q_SIGNALS:

    void stepStarted(const QString& stepName);
    void smallStepStarted(int currValue, int maxValue);
    void finished(int finishState, const QString& errorMsg);

public Q_SLOTS:

    void stopProcessing();

private:

    bool copyTable(DatabaseBackend& fromDBbackend, const QString& fromActionName, 
                   DatabaseBackend& toDBbackend, const QString &toActionName);

    void handleClosing(bool isstopThread, DatabaseBackend& fromDBbackend, DatabaseBackend& toDBbackend);

private:

    bool m_isStopProcessing;
};

} // namespace Digikam

#endif // DATABASECOPYMANAGER_H

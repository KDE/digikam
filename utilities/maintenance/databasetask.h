/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-01-29
 * Description : Thread actions task for database cleanup.
 *
 * Copyright (C) 2017 by Mario Frank <mario dot frank at uni minus potsdam dot de>
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

#ifndef DATABASE_TASK_H
#define DATABASE_TASK_H

// Local includes

#include "actionthreadbase.h"

namespace Digikam
{
    class Identity;
}

namespace Digikam
{

class LoadingDescription;
class MaintenanceData;

class DatabaseTask : public ActionJob
{
    Q_OBJECT

public:

    enum Mode{
        Unknown,
        ComputeDatabaseJunk,
        CleanCoreDb,
        CleanThumbsDb,
        CleanRecognitionDb,
        ShrinkDatabases
    };

    explicit DatabaseTask();
    virtual ~DatabaseTask();

    void setMode(Mode mode);
    void setMaintenanceData(MaintenanceData* const data=0);

    void computeDatabaseJunk(bool thumbsDb=false, bool facesDb=false);


Q_SIGNALS:

    void signalFinished();
    void signalFinished(bool done, bool errorFree);

    void signalData(const QList<qlonglong>& staleImageIds,
                    const QList<int>& staleThumbIds,
                    const QList<Identity>& staleIdentities);

    void signalStarted();

    /**
     * Signal to emit the count of additional items to process.
     */
    void signalAddItemsToProcess(int count);

protected:

    void run();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // DATABASE_TASK_H

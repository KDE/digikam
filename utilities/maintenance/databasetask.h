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

namespace FacesEngine
{
    class Identity;
}

namespace Digikam
{

class LoadingDescription;

class DatabaseTask : public ActionJob
{
    Q_OBJECT

public:

    explicit DatabaseTask();
    virtual ~DatabaseTask();

    void setItem(qlonglong imageId);
    void setItems(const QList<qlonglong>& imageIds);
    void setThumbId(int thumbId);
    void setThumbIds(const QList<int>& thumbIds);
    void setIdentity(const FacesEngine::Identity& identity);
    void setIdentities(const QList<FacesEngine::Identity>& identities);

    void computeDatabaseJunk(bool thumbsDb=false, bool facesDb=false);

Q_SIGNALS:

    void signalFinished();

    void signalData(const QList<qlonglong>& staleImageIds,
                    const QList<int>& staleThumbIds,
                    const QList<FacesEngine::Identity>& staleIdentities);

public Q_SLOTS:

    void slotCancel();

protected:

    void run();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // DATABASE_TASK_H

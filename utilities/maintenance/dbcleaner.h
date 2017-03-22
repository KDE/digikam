/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-01-29
 * Description : Database cleaner.
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

#ifndef DBCLEANER_H
#define DBCLEANER_H

// Qt includes

#include <QString>
#include <QObject>

// Local includes

#include "maintenancetool.h"

namespace FacesEngine
{
    class Identity;
}

namespace Digikam
{

class DbCleaner : public MaintenanceTool
{
    Q_OBJECT

public:

    explicit DbCleaner(bool cleanThumbsDb = false, bool cleanFacesDb = false, bool shrinkDatabases = false, ProgressItem* const parent = 0);
    virtual ~DbCleaner();

    void setUseMultiCoreCPU(bool b);

private Q_SLOTS:

    void slotStart();
    void slotCancel();
    void slotAdvance();
    void slotShrinkNextDBInfo(bool done, bool passed);

    void slotFetchedData(const QList<qlonglong>& staleImageIds,
                         const QList<int>& staleThumbIds,
                         const QList<FacesEngine::Identity>& staleIdentities);

    void slotAddItemsToProcess(int count);

    void slotCleanItems();
    void slotCleanedItems();
    void slotCleanedThumbnails();
    void slotCleanedFaces();
    void slotShrinkDatabases();

    void slotDone();

private:

    static QString VACUUM_PENDING;
    static QString VACUUM_DONE;
    static QString VACUUM_NOT_DONE;
    static QString INTEGRITY_FAILED_AFTER_VACUUM;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DBCLEANER_H

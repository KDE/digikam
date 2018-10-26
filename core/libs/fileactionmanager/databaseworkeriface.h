/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-18
 * Description : database worker interface
 *
 * Copyright (C) 2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAM_DATABASE_WORKER_IFACE_H
#define DIGIKAM_DATABASE_WORKER_IFACE_H

// Qt includes

#include <QList>

// Local includes

#include "fileactionmngr.h"
#include "fileactionimageinfolist.h"
#include "iteminfo.h"
#include "workerobject.h"

namespace Digikam
{

class MetadataHub;

class DatabaseWorkerInterface : public WorkerObject
{
    Q_OBJECT

public Q_SLOTS:

    virtual void assignTags(FileActionItemInfoList, const QList<int>&)      {};
    virtual void removeTags(FileActionItemInfoList, const QList<int>&)      {};
    virtual void assignPickLabel(FileActionItemInfoList, int)               {};
    virtual void assignColorLabel(FileActionItemInfoList, int)              {};
    virtual void assignRating(FileActionItemInfoList, int)                  {};
    virtual void editGroup(int, const ItemInfo&, FileActionItemInfoList)   {};
    virtual void setExifOrientation(FileActionItemInfoList, int)            {};
    virtual void applyMetadata(FileActionItemInfoList, DisjointMetadata*)   {};
    virtual void copyAttributes(FileActionItemInfoList, const QStringList&) {};

Q_SIGNALS:

    void writeMetadataToFiles(FileActionItemInfoList infos);
    void writeOrientationToFiles(FileActionItemInfoList infos, int orientation);
    void writeMetadata(FileActionItemInfoList infos, int flag);
};

// ------------------------------------------------------------------------------------

class FileActionMngrDatabaseWorker : public DatabaseWorkerInterface
{
public:

    explicit FileActionMngrDatabaseWorker(FileActionMngr::Private* const d)
        : d(d)
    {
    }

public:

    void assignTags(FileActionItemInfoList infos, const QList<int>& tagIDs);
    void removeTags(FileActionItemInfoList infos, const QList<int>& tagIDs);
    void assignPickLabel(FileActionItemInfoList infos, int pickId);
    void assignColorLabel(FileActionItemInfoList infos, int colorId);
    void assignRating(FileActionItemInfoList infos, int rating);
    void editGroup(int groupAction, const ItemInfo& pick, FileActionItemInfoList infos);
    void setExifOrientation(FileActionItemInfoList infos, int orientation);
    void applyMetadata(FileActionItemInfoList infos, DisjointMetadata* hub);
    void copyAttributes(FileActionItemInfoList infos, const QStringList& derivedPaths);

private:

    void changeTags(FileActionItemInfoList infos, const QList<int>& tagIDs, bool addOrRemove);

private:

    FileActionMngr::Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_DATABASE_WORKER_IFACE_H

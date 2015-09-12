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

#ifndef DATABASEWORKERIFACE_H
#define DATABASEWORKERIFACE_H

// Qt includes

#include <QList>

// Local includes

#include "fileactionmngr.h"
#include "fileactionimageinfolist.h"
#include "imageinfo.h"
#include "workerobject.h"

namespace Digikam
{

class MetadataHub;

class DatabaseWorkerInterface : public WorkerObject
{
    Q_OBJECT

public Q_SLOTS:

    virtual void assignTags(FileActionImageInfoList, const QList<int>&)      {};
    virtual void removeTags(FileActionImageInfoList, const QList<int>&)      {};
    virtual void assignPickLabel(FileActionImageInfoList, int)               {};
    virtual void assignColorLabel(FileActionImageInfoList, int)              {};
    virtual void assignRating(FileActionImageInfoList, int)                  {};
    virtual void editGroup(int, const ImageInfo&, FileActionImageInfoList)   {};
    virtual void setExifOrientation(FileActionImageInfoList, int)            {};
    virtual void applyMetadata(FileActionImageInfoList, DisjointMetadata*)   {};
    virtual void copyAttributes(FileActionImageInfoList, const QStringList&) {};

Q_SIGNALS:

    void writeMetadataToFiles(FileActionImageInfoList infos);
    void writeOrientationToFiles(FileActionImageInfoList infos, int orientation);
    void writeMetadata(FileActionImageInfoList infos, int flag);
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

    void assignTags(FileActionImageInfoList infos, const QList<int>& tagIDs);
    void removeTags(FileActionImageInfoList infos, const QList<int>& tagIDs);
    void assignPickLabel(FileActionImageInfoList infos, int pickId);
    void assignColorLabel(FileActionImageInfoList infos, int colorId);
    void assignRating(FileActionImageInfoList infos, int rating);
    void editGroup(int groupAction, const ImageInfo& pick, FileActionImageInfoList infos);
    void setExifOrientation(FileActionImageInfoList infos, int orientation);
    void applyMetadata(FileActionImageInfoList infos, DisjointMetadata* hub);
    void copyAttributes(FileActionImageInfoList infos, const QStringList& derivedPaths);

private:

    void changeTags(FileActionImageInfoList infos, const QList<int>& tagIDs, bool addOrRemove);

private:

    FileActionMngr::Private* const d;
};

} // namespace Digikam

#endif /* DATABASEWORKERIFACE_H */

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-18
 * Description : database worker interface
 *
 * Copyright (C) 2012 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#include "imageinfo.h"
#include "workerobject.h"
#include "fileactionmngr.h"

namespace Digikam
{

class MetadataHub;

class DatabaseWorkerInterface : public WorkerObject
{
    Q_OBJECT

public Q_SLOTS:

    virtual void assignTags(const QList<ImageInfo>&, const QList<int>&)    {};
    virtual void removeTags(const QList<ImageInfo>&, const QList<int>&)    {};
    virtual void assignPickLabel(const QList<ImageInfo>&, int)             {};
    virtual void assignColorLabel(const QList<ImageInfo>&, int)            {};
    virtual void assignRating(const QList<ImageInfo>&, int)                {};
    virtual void editGroup(int, const ImageInfo&, const QList<ImageInfo>&) {};
    virtual void setExifOrientation(const QList<ImageInfo>&, int)          {};
    virtual void applyMetadata(const QList<ImageInfo>&, MetadataHub*)      {};

Q_SIGNALS:

    void writeMetadataToFiles(const QList<ImageInfo>& infos);
    void writeOrientationToFiles(const QList<ImageInfo>& infos, int orientation);
    void writeMetadata(const QList<ImageInfo>& infos, MetadataHub* hub);
};

// ------------------------------------------------------------------------------------

class FileActionMngrDatabaseWorker : public DatabaseWorkerInterface
{
public:

    FileActionMngrDatabaseWorker(FileActionMngr::FileActionMngrPriv* d)
        : d(d) {}

public:

    void assignTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs);
    void removeTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs);
    void assignPickLabel(const QList<ImageInfo>& infos, int pickId);
    void assignColorLabel(const QList<ImageInfo>& infos, int colorId);
    void assignRating(const QList<ImageInfo>& infos, int rating);
    void editGroup(int groupAction, const ImageInfo& pick, const QList<ImageInfo>& infos);
    void setExifOrientation(const QList<ImageInfo>& infos, int orientation);
    void applyMetadata(const QList<ImageInfo>& infos, MetadataHub* hub);

private:

    void changeTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs, bool addOrRemove);

private:

    FileActionMngr::FileActionMngrPriv* const d;
};

} // namespace Digikam

#endif /* DATABASEWORKERIFACE_H */

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-18
 * Description : file worker interface
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

#ifndef DIGIKAM_FILE_WORKER_IFACE_H
#define DIGIKAM_FILE_WORKER_IFACE_H

// Local includes

#include "fileactionmngr.h"
#include "fileactionimageinfolist.h"
#include "iteminfo.h"
#include "workerobject.h"

namespace Digikam
{

class MetadataHub;

class FileWorkerInterface : public WorkerObject
{
    Q_OBJECT

public Q_SLOTS:

    virtual void writeOrientationToFiles(FileActionItemInfoList, int) {};
    virtual void writeMetadataToFiles(FileActionItemInfoList)         {};
    virtual void writeMetadata(FileActionItemInfoList, int)           {};
    virtual void transform(FileActionItemInfoList, int)               {};

Q_SIGNALS:

    void imageDataChanged(const QString& path, bool removeThumbnails, bool notifyCache);
    void imageChangeFailed(const QString& message, const QStringList& fileNames);
};

// ---------------------------------------------------------------------------------------------

class FileActionMngrFileWorker : public FileWorkerInterface
{

public:

    explicit FileActionMngrFileWorker(FileActionMngr::Private* const d)
        : d(d)
    {
    }

public:

    void writeOrientationToFiles(FileActionItemInfoList infos, int orientation) override;
    void writeMetadataToFiles(FileActionItemInfoList infos) override;
    void writeMetadata(FileActionItemInfoList infos, int flags) override;
    void transform(FileActionItemInfoList infos, int orientation) override;
    void adjustFaceRectangles(const ItemInfo& info, bool rotatedPixels,
                                                    int newOrientation,
                                                    int oldOrientation);

private:

    FileActionMngr::Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_FILE_WORKER_IFACE_H

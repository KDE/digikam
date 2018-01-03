/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
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

#ifndef FILEWORKERIFACE_H
#define FILEWORKERIFACE_H

// Local includes

#include "fileactionmngr.h"
#include "fileactionimageinfolist.h"
#include "imageinfo.h"
#include "workerobject.h"

namespace Digikam
{

class MetadataHub;

class FileWorkerInterface : public WorkerObject
{
    Q_OBJECT

public Q_SLOTS:

    virtual void writeOrientationToFiles(FileActionImageInfoList, int){};
    virtual void writeMetadataToFiles(FileActionImageInfoList)        {};
    virtual void writeMetadata(FileActionImageInfoList, int)          {};
    virtual void transform(FileActionImageInfoList, int)              {};

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

    void writeOrientationToFiles(FileActionImageInfoList infos, int orientation);
    void writeMetadataToFiles(FileActionImageInfoList infos);
    void writeMetadata(FileActionImageInfoList infos, int flags);
    void transform(FileActionImageInfoList infos, int orientation);
    void ajustFaceRectangles(const ImageInfo& info, int action);

private:

    FileActionMngr::Private* const d;
};

} // namespace Digikam

#endif /* FILEWORKERIFACE_H */

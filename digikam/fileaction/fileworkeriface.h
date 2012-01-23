/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-18
 * Description : file worker interface
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

#ifndef FILEWORKERIFACE_H
#define FILEWORKERIFACE_H

// Qt includes

#include <QList>

// Local includes

#include "imageinfo.h"
#include "workerobject.h"
#include "fileactionmngr.h"

namespace Digikam
{

class MetadataHub;

class FileWorkerInterface : public WorkerObject
{
    Q_OBJECT

public Q_SLOTS:

    void writeOrientationToFiles(const QList<ImageInfo>&, int)        {};
    virtual void writeMetadataToFiles(const QList<ImageInfo>&)        {};
    virtual void writeMetadata(const QList<ImageInfo>&, MetadataHub*) {};
    virtual void transform(const QList<ImageInfo>&, int)              {};

Q_SIGNALS:

    void imageDataChanged(const QString& path, bool removeThumbnails, bool notifyCache);
    void imageChangeFailed(const QString& message, const QStringList& fileNames);
};

// ---------------------------------------------------------------------------------------------

class FileActionMngrFileWorker : public FileWorkerInterface
{

public:

    FileActionMngrFileWorker(FileActionMngr::FileActionMngrPriv* d)
        : d(d) {}

public:

    void writeOrientationToFiles(const QList<ImageInfo>& infos, int orientation);
    void writeMetadataToFiles(const QList<ImageInfo>& infos);
    void writeMetadata(const QList<ImageInfo>& infos, MetadataHub* hub);
    void transform(const QList<ImageInfo>& infos, int orientation);

private:

    FileActionMngr::FileActionMngrPriv* const d;
};

} // namespace Digikam

#endif /* FILEWORKERIFACE_H */

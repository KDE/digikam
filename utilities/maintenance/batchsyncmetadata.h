/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-22-01
 * Description : batch sync pictures metadata with
 *               digiKam database
 *
 * Copyright (C) 2007-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef BATCHSYNCMETADATA_H
#define BATCHSYNCMETADATA_H

// Qt includes

#include <QObject>

// Local includes

#include "imageinfo.h"

namespace Digikam
{

class Album;

class BatchSyncMetadata : public QObject
{
    Q_OBJECT

public:

    enum SyncDirection
    {
        WriteFromDatabaseToFile,
        ReadFromFileToDatabase
    };

public:

    /** Constructor which sync all metadata pictures from an Album */
    explicit BatchSyncMetadata(Album* album, SyncDirection direction = WriteFromDatabaseToFile, QObject* parent = 0);

    /** Constructor which sync all metadata from an images list */
    explicit BatchSyncMetadata(const ImageInfoList& list, SyncDirection = WriteFromDatabaseToFile, QObject* parent = 0);

    ~BatchSyncMetadata();

    void parseAlbum();

Q_SIGNALS:

    void signalComplete();
    void signalProgressValue(float);
    void signalBegin(const QString&);
    void startParsingList();

public Q_SLOTS:

    void parseList();
    void slotAbort();

private Q_SLOTS:

    void slotAlbumParsed(const ImageInfoList&);
    void slotComplete();

private:

    void parsePicture();
    void complete();

private:

    class BatchSyncMetadataPriv;
    BatchSyncMetadataPriv* const d;
};

}  // namespace Digikam

#endif /* BATCHSYNCMETADATA_H */

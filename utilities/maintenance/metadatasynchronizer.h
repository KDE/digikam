/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-22-01
 * Description : batch sync pictures metadata with database
 *
 * Copyright (C) 2007-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef METADATASYNCHRONIZER_H
#define METADATASYNCHRONIZER_H

// Local includes

#include "imageinfo.h"
#include "maintenancetool.h"

namespace Digikam
{

class Album;

class MetadataSynchronizer : public MaintenanceTool
{
    Q_OBJECT

public:

    enum SyncDirection
    {
        WriteFromDatabaseToFile,
        ReadFromFileToDatabase
    };

public:

    /** Constructor which sync all pictures metadata pictures from whole Albums collection */
    MetadataSynchronizer(SyncDirection direction, ProgressItem* const parent = 0);

    /** Constructor which sync all pictures metadata from an Album */
    MetadataSynchronizer(Album* const album, SyncDirection direction = WriteFromDatabaseToFile, ProgressItem* const parent = 0);

    /** Constructor which sync all pictures metadata from an images list */
    MetadataSynchronizer(const ImageInfoList& list, SyncDirection = WriteFromDatabaseToFile, ProgressItem* const parent = 0);

    ~MetadataSynchronizer();

private Q_SLOTS:

    void slotStart();
    void slotParseAlbums();
    void slotAlbumParsed(const ImageInfoList&);
    void slotOneAlbumIsComplete();
    void slotCancel();

private:

    void parseList();
    void parsePicture();
    void processOneAlbum();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* METADATASYNCHRONIZER_H */

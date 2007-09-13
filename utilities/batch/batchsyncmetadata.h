/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-22-01
 * Description : batch sync pictures metadata with
 *               digiKam database
 *
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qobject.h>

// Local includes.

#include "imageinfo.h"

class KURL;

namespace Digikam
{

class Album;
class BatchSyncMetadataPriv;

class BatchSyncMetadata : public QObject
{
    Q_OBJECT

public:

    /** Constructor witch sync all metatada pictures from an Album */ 
    BatchSyncMetadata(QObject* parent, Album *album);

    /** Constructor witch sync all metatada from an images list */ 
    BatchSyncMetadata(QObject* parent, const ImageInfoList& list);

    ~BatchSyncMetadata();

    void parseList();
    void parseAlbum();

signals:

    void signalComplete();
    void signalProgressValue(int);
    void signalProgressBarMode(int, const QString&);

public slots:

    void slotAbort();

private:

    void parsePicture();
    void complete();

private slots:

    void slotAlbumParsed(const ImageInfoList&);
    void slotComplete();

private:

    BatchSyncMetadataPriv *d;
};

}  // namespace Digikam

#endif /* BATCHSYNCMETADATA_H */

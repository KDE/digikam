/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-22-01
 * Description : batch sync picture metadata with digiKam database
 *
 * Copyright 2007 by Gilles Caulier
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

// Local includes.

#include "imageinfo.h"
#include "dprogressdlg.h"

class QWidget;

class KURL;

namespace Digikam
{

class BatchSyncMetadataPriv;

class BatchSyncMetadata : public DProgressDlg
{
    Q_OBJECT

public:

    BatchSyncMetadata(QWidget* parent);
    ~BatchSyncMetadata();

signals:

    void signalComplete();

private:

    void abort();
    void parseAlbum();

protected:

    void closeEvent(QCloseEvent *e);

protected slots:

    void slotCancel();

private slots:

    void slotStart();
    void slotAlbumParsed(const ImageInfoList&);
    void slotComplete();

private:

    BatchSyncMetadataPriv *d;
};

}  // namespace Digikam

#endif /* BATCHSYNCMETADATA_H */

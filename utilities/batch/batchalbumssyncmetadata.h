/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2007-22-01
 * Description : batch sync pictures metadata with 
 *               digiKam database
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

#ifndef BATCHALBUMSSYNCMETADATA_H
#define BATCHALBUMSSYNCMETADATA_H

// Local includes.

#include "imageinfo.h"
#include "dprogressdlg.h"

class QWidget;

class KURL;

namespace Digikam
{

class BatchAlbumsSyncMetadataPriv;

class BatchAlbumsSyncMetadata : public DProgressDlg
{
    Q_OBJECT

public:

    BatchAlbumsSyncMetadata(QWidget* parent);
    ~BatchAlbumsSyncMetadata();

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

    BatchAlbumsSyncMetadataPriv *d;
};

}  // namespace Digikam

#endif /* BATCHALBUMSSYNCMETADATA_H */

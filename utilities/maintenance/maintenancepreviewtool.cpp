/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-16
 * Description : Maintenance tool using preview load thread as items processor.
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "maintenancepreviewtool.moc"

// Qt includes

#include <QFileInfo>
#include <QDateTime>
#include <QPixmap>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albuminfo.h"
#include "albummanager.h"
#include "databaseaccess.h"
#include "dimg.h"
#include "imageinfo.h"
#include "previewloadthread.h"

namespace Digikam
{

class MaintenancePreviewTool::MaintenancePreviewToolPriv
{
public:

    MaintenancePreviewToolPriv() :
        previewLoadThread(0)
    {
    }

    PreviewLoadThread* previewLoadThread;
};

MaintenancePreviewTool::MaintenancePreviewTool(const QString& id, Mode mode, int albumId)
    : MaintenancePictPathTool(id, mode, albumId),
      d(new MaintenancePreviewToolPriv)
{
    d->previewLoadThread = new PreviewLoadThread();

    connect(d->previewLoadThread, SIGNAL(signalImageLoaded(LoadingDescription, DImg)),
            this, SLOT(slotGotImagePreview(LoadingDescription, DImg)));
}

MaintenancePreviewTool::~MaintenancePreviewTool()
{
    delete d;
}

PreviewLoadThread* MaintenancePreviewTool::previewLoadThread() const
{
    return d->previewLoadThread;
}

void MaintenancePreviewTool::slotGotImagePreview(const LoadingDescription& desc, const DImg& img)
{
    if (cancel() || allPicturesPath().isEmpty())
    {
        return;
    }

    if (allPicturesPath().first() != desc.filePath)
    {
        return;
    }

    gotNewPreview(desc, img);

    QPixmap pix = DImg(img).smoothScale(22, 22, Qt::KeepAspectRatio).convertToPixmap();
    setThumbnail(pix);
    advance(1);

    if (!allPicturesPath().isEmpty())
    {
        allPicturesPath().removeFirst();
    }

    if (allPicturesPath().isEmpty())
    {
        complete();
    }
    else
    {
        processOne();
    }
}

}  // namespace Digikam

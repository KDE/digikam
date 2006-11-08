/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-19-06
 * Description : digiKam KIO preview extractor interface
 *
 * Copyright 2006 by Gilles Caulier
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

// C Ansi includes.

extern "C"
{
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
}

// QT includes.

#include <qstring.h>
#include <qimage.h>
#include <qdatastream.h>

// KDE includes.

#include <kglobal.h>

// Local includes.

#include "ddebug.h"
#include "imagepreviewjob.h"

namespace Digikam
{

class ImagePreviewJobPriv
{
public:

    bool        exifRotate;
    bool        running;

    int         size;

    // Shared memory segment Id. The segment is allocated to a size
    // of extent x extent x 4 (32 bit image) on first need.
    int         shmid;

    // And the data area
    uchar      *shmaddr;

    KURL        imageUrl;
};

ImagePreviewJob::ImagePreviewJob(const KURL& url, int size, bool exifRotate)
               : KIO::Job(false)
{
    d = new ImagePreviewJobPriv;

    d->imageUrl   = url;
    d->size       = size;
    d->exifRotate = exifRotate;
    d->running    = false;
    d->shmid      = -1;
    d->shmaddr    = 0;

    getImagePreview();
}

ImagePreviewJob::~ImagePreviewJob()
{
    if (d->shmaddr)
    {
        shmdt((char*)d->shmaddr);
        shmctl(d->shmid, IPC_RMID, 0);
    }

    delete d;
}

void ImagePreviewJob::getImagePreview()
{
    if (d->imageUrl.isEmpty())
    {
        d->running = false;
        emit signalCompleted();
        return;
    }

    KURL url(d->imageUrl);
    url.setProtocol("digikampreview");

    KIO::TransferJob *job = KIO::get(url, false, false);
    job->addMetaData("size", QString::number(d->size));
    createShmSeg();

    if (d->shmid != -1)
        job->addMetaData("shmid", QString::number(d->shmid));

    // Rotate preview accordindly with Exif rotation tag if necessary.
    if (d->exifRotate)
        job->addMetaData("exif", "yes");

    connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)),
            this, SLOT(slotImagePreviewData(KIO::Job *, const QByteArray &)));

    addSubjob(job);
    d->running = true;
}

void ImagePreviewJob::slotResult(KIO::Job *job)
{
    subjobs.remove(job);
    Q_ASSERT( subjobs.isEmpty() );

    if (job->error())
        emit signalFailed(d->imageUrl);

    d->running  = false;
    d->imageUrl = KURL();
}

void ImagePreviewJob::createShmSeg()
{
    if (d->shmid == -1)
    {
        if (d->shmaddr) 
        {
            shmdt((char*)d->shmaddr);
            shmctl(d->shmid, IPC_RMID, 0);
        }

        d->shmid = shmget(IPC_PRIVATE, d->size * d->size * 4, IPC_CREAT|0600);
        if (d->shmid != -1)
        {
            d->shmaddr = static_cast<uchar *>(shmat(d->shmid, 0, SHM_RDONLY));
            if (d->shmaddr == (uchar *)-1)
            {
                shmctl(d->shmid, IPC_RMID, 0);
                d->shmaddr = 0;
                d->shmid = -1;
            }
        }
        else
            d->shmaddr = 0;
    }
}

void ImagePreviewJob::slotImagePreviewData(KIO::Job*, const QByteArray &data)
{
    if (data.isEmpty())
        return;

    QImage preview;
    QDataStream stream(data, IO_ReadOnly);
    if (d->shmaddr)
    {
        int width, height, depth;
        stream >> width >> height >> depth;
        preview = QImage(d->shmaddr, width, height, depth,
                         0, 0, QImage::IgnoreEndian);

        // The buffer supplied to the QImage constructor above must remain valid
        // throughout the lifetime of the object.
        // This is not true, the shared memory will be freed or reused.
        // If we pass the object around, we must do a deep copy.
        preview = preview.copy();
    }
    else
    {
        stream >> preview;
    }

    if (preview.isNull()) 
    {
        DWarning() << k_funcinfo << "preview is null" << endl;
        emit signalFailed(d->imageUrl);
        return;
    }

    emit signalImagePreview(d->imageUrl, preview);
}

}  // namespace Digikam

#include "imagepreviewjob.moc"


/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-10-14
 * Description : digiKam KIO thumbnails generator interface
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QColor>
#include <QDataStream>

// KDE includes.

#include <kglobal.h>
#include <kio/previewjob.h>

// Local includes.

#include "ddebug.h"
#include "thumbnailjob.h"
#include "thumbnailjob.moc"

namespace Digikam
{

class ThumbnailJobPriv
{
public:

    bool        highlight;
    bool        exifRotate;
    bool        running;

    int         size;

    // Shared memory segment Id. The segment is allocated to a size
    // of extent x extent x 4 (32 bit image) on first need.
    int         shmid;

    // And the data area
    uchar      *shmaddr;

    KUrl        curr_url;
    KUrl        next_url;
    KUrl::List  urlList;
};

ThumbnailJob::ThumbnailJob(const KUrl& url, int size,
                           bool highlight, bool exifRotate)
            : KIO::Job()
{
    d = new ThumbnailJobPriv;

    d->urlList.append(url);

    d->size       = size;
    d->highlight  = highlight;
    d->exifRotate = exifRotate;
    d->curr_url   = d->urlList.first();
    d->next_url   = d->curr_url;
    d->running    = false;
    d->shmid      = -1;
    d->shmaddr    = 0;

    processNext();
}

ThumbnailJob::ThumbnailJob(const KUrl::List& urlList, int size,
                           bool highlight, bool exifRotate)
            : KIO::Job()
{
    d = new ThumbnailJobPriv;

    d->urlList    = urlList;
    d->size       = size;
    d->highlight  = highlight;
    d->running    = false;
    d->exifRotate = exifRotate;
    d->curr_url   = d->urlList.first();
    d->next_url   = d->curr_url;
    d->shmid      = -1;
    d->shmaddr    = 0;

    processNext();
}

ThumbnailJob::~ThumbnailJob()
{
    if (d->shmaddr)
    {
        shmdt((char*)d->shmaddr);
        shmctl(d->shmid, IPC_RMID, 0);
    }

    delete d;
}

void ThumbnailJob::addItem(const KUrl& url)
{
    d->urlList.append(url);

    if (!d->running && subjobs().isEmpty())
        processNext();
}

void ThumbnailJob::addItems(const KUrl::List& urlList)
{
    for (KUrl::List::const_iterator it = urlList.begin();
         it != urlList.end(); ++it)
    {
        d->urlList.append(*it);
    }

    if (!d->running && subjobs().isEmpty())
        processNext();
}

bool ThumbnailJob::setNextItemToLoad(const KUrl& url)
{
    int index = d->urlList.indexOf(url);
    if (index != -1)
    {
        d->next_url = d->urlList[index];
        return true;
    }

    return false;
}

void ThumbnailJob::removeItem(const KUrl& url)
{
    d->urlList.removeAll(url);
}

void ThumbnailJob::processNext()
{
    if (d->urlList.isEmpty())
    {
        d->running = false;
        emit signalCompleted();
        return;
    }

    KUrl::List::iterator it;
    for (it = d->urlList.begin(); it != d->urlList.end(); ++it)
    {
        if (*it == d->next_url)
            break;
    }

    if (it == d->urlList.end())
        it = d->urlList.begin();

    d->curr_url = *it;
    it = d->urlList.erase(it);
    if (it != d->urlList.end())
    {
        d->next_url = *it;
    }
    else
    {
        d->next_url = KUrl();
    }

    KUrl url(d->curr_url);
    url.setProtocol("digikamthumbnail");

    KIO::TransferJob *job = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);
    job->addMetaData("size", QString::number(d->size));
    createShmSeg();

    if (d->shmid != -1)
        job->addMetaData("shmid", QString::number(d->shmid));

    // Rotate thumbnail accordindly with Exif rotation tag if necessary.
    if (d->exifRotate)
        job->addMetaData("exif", "yes");

    connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)),
            this, SLOT(slotThumbData(KIO::Job *, const QByteArray &)));

    addSubjob(job);
    d->running = true;
}

void ThumbnailJob::slotResult(KJob *job)
{
    // TODO: this slot is not called when the slave crashed ??
    removeSubjob(job);
    Q_ASSERT( subjobs().isEmpty() );

    if (job->error())
    {
        // try again with KDE preview
        KUrl::List list;
        list << d->curr_url;
        KIO::PreviewJob *job = KIO::filePreview(list, d->size);

        connect(job, SIGNAL(gotPreview(const KFileItem &, const QPixmap &)),
                this, SLOT(gotKDEPreview(const KFileItem &, const QPixmap &)));
        connect(job, SIGNAL(failed(const KFileItem &)),
                this, SLOT(failedKDEPreview(const KFileItem &)));

        addSubjob(job);
        return;
    }

    d->running  = false;
    processNext();
}

void ThumbnailJob::gotKDEPreview(const KFileItem &, const QPixmap &pix)
{
    emit signalThumbnail(d->curr_url, pix);
}

void ThumbnailJob::failedKDEPreview(const KFileItem &)
{
    emit signalFailed(d->curr_url);
}

void ThumbnailJob::createShmSeg()
{
    if (d->shmid == -1)
    {
        if (d->shmaddr) 
        {
            shmdt((char*)d->shmaddr);
            shmctl(d->shmid, IPC_RMID, 0);
        }

        d->shmid = shmget(IPC_PRIVATE, 256 * 256 * 4, IPC_CREAT|0600);
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

void ThumbnailJob::slotThumbData(KIO::Job*, const QByteArray &data)
{
    if (data.isEmpty())
        return;

    QImage thumb;
    QDataStream stream(data);
    if (d->shmaddr)
    {
        int width, height, format;
        stream >> width >> height >> format;
        thumb = QImage(d->shmaddr, width, height, (QImage::Format)format);

        // The buffer supplied to the QImage constructor above must remain valid
        // throughout the lifetime of the object.
        // This is not true, the shared memory will be freed or reused.
        // If we pass the object around, we must do a deep copy.
        thumb = thumb.copy();
    }
    else
    {
        stream >> thumb;
    }

    if (thumb.isNull()) 
    {
        DWarning() << "thumbnail is null" << endl;
        emit signalFailed(d->curr_url);
        return;
    }

    emitThumbnail(thumb);
}

void ThumbnailJob::emitThumbnail(QImage& thumb)
{
    if (thumb.isNull())
        return;

    QPixmap pix = QPixmap::fromImage(thumb);

    int w = pix.width();
    int h = pix.height();

    // highlight only when requested and when thumbnail
    // width and height are greater than 10
    if (d->highlight && (w >= 10 && h >= 10))
    {
        QPainter p(&pix);
        p.setPen(QPen(Qt::black, 1));
        p.drawRect(0, 0, w - 1, h - 1);
    }

    emit signalThumbnail(d->curr_url, pix);
}

}  // namespace Digikam

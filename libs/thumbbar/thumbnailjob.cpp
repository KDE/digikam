/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-10-14
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju
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
#include <qdir.h>
#include <qfileinfo.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qcolor.h>
#include <qdatastream.h>

// KDE includes.

#include <kglobal.h>
#include <kdebug.h>

// Local includes.

#include "thumbnailjob.h"

namespace Digikam
{

class ThumbnailJobPriv
{
public:

    KURL::List    urlList;
    int           size;
    bool          highlight;
    bool          exifRotate;

    KURL          curr_url;
    KURL          next_url;
    bool          running;

    // Shared memory segment Id. The segment is allocated to a size
    // of extent x extent x 4 (32 bit image) on first need.
    int           shmid;
    // And the data area
    uchar        *shmaddr;
};

ThumbnailJob::ThumbnailJob(const KURL& url, int size,
                           bool highlight, bool exifRotate)
    : KIO::Job(false)
{
    d = new ThumbnailJobPriv;

    d->urlList.append(url);
    d->size       = size;
    d->highlight  = highlight;
    d->exifRotate = exifRotate;

    d->curr_url = d->urlList.first();
    d->next_url = d->curr_url;
    d->running  = false;
    
    d->shmid   = -1;
    d->shmaddr = 0;

    processNext();
}

ThumbnailJob::ThumbnailJob(const KURL::List& urlList, int size,
                           bool highlight, bool exifRotate)
    : KIO::Job(false)
{
    d = new ThumbnailJobPriv;

    d->urlList    = urlList;
    d->size       = size;
    d->highlight  = highlight;
    d->running    = false;
    d->exifRotate = exifRotate;

    d->curr_url = d->urlList.first();
    d->next_url = d->curr_url;
    
    d->shmid = -1;
    d->shmaddr = 0;

    processNext();
}

ThumbnailJob::~ThumbnailJob()
{
    if (d->shmaddr) {
        shmdt((char*)d->shmaddr);
        shmctl(d->shmid, IPC_RMID, 0);
    }
    delete d;
}

void ThumbnailJob::addItem(const KURL& url)
{
    d->urlList.append(url);

    if (!d->running && subjobs.isEmpty())
        processNext();
}

void ThumbnailJob::addItems(const KURL::List& urlList)
{
    for (KURL::List::const_iterator it = urlList.begin();
         it != urlList.end(); ++it)
    {
        d->urlList.append(*it);
    }

    if (!d->running && subjobs.isEmpty())
        processNext();
}

bool ThumbnailJob::setNextItemToLoad(const KURL& url)
{
    KURL::List::const_iterator it = d->urlList.find(url);
    if (it != d->urlList.end())
    {
        d->next_url = *it;
        return true;
    }

    return false;
}

void ThumbnailJob::removeItem(const KURL& url)
{
    d->urlList.remove(url);
}

void ThumbnailJob::processNext()
{
    if (d->urlList.isEmpty())
    {
        d->running = false;
        emit signalCompleted();
        return;
    }

    KURL::List::iterator it = d->urlList.find(d->next_url);
    if (it == d->urlList.end())
    {
        it = d->urlList.begin();
    }

    d->curr_url = *it;
    it = d->urlList.remove(it);
    if (it != d->urlList.end())
    {
        d->next_url = *it;
    }
    else
    {
        d->next_url = KURL();
    }

    KURL url(d->curr_url);
    url.setProtocol("digikamthumbnail");

    KIO::TransferJob *job = KIO::get(url, false, false);
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

void ThumbnailJob::slotResult(KIO::Job *job)
{
    subjobs.remove(job);
    Q_ASSERT( subjobs.isEmpty() );
    
    if (job->error())
    {
        emit signalFailed(d->curr_url);
    }

    d->running  = false;
    processNext();
}

void ThumbnailJob::createShmSeg()
{
    if (d->shmid == -1)
    {
        if (d->shmaddr) {
            shmdt((char*)d->shmaddr);
            shmctl(d->shmid, IPC_RMID, 0);
        }
        d->shmid = shmget(IPC_PRIVATE, 256 * 256 * 4,
                          IPC_CREAT|0600);
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
    QDataStream stream(data, IO_ReadOnly);
    if (d->shmaddr)
    {
        int width, height, depth;
        stream >> width >> height >> depth;
        thumb = QImage(d->shmaddr, width, height, depth,
                       0, 0, QImage::IgnoreEndian);
    }
    else {
        stream >> thumb;
    }

    if (thumb.isNull()) {
        kdWarning() << k_funcinfo << "thumbnail is null" << endl;
        emit signalFailed(d->curr_url);
        return;
    }

    emitThumbnail(thumb);
}

void ThumbnailJob::emitThumbnail(QImage& thumb)
{
    if (thumb.isNull())
    {
        return;
    }

    QPixmap pix(thumb);

    int w = pix.width();
    int h = pix.height();

    // highlight only when requested and when thumbnail
    // width and height are greater than 10
    if (d->highlight && (w >= 10 && h >= 10))
    {
        QPainter p(&pix);
        p.setPen(QPen(QColor(0,0,0),1));
        p.drawRect(0,0,w,h);
        p.end();
    }

    emit signalThumbnail(d->curr_url, pix);
}

}  // namespace Digikam

#include "thumbnailjob.moc"


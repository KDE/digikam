/* ============================================================
 * File  : camerathumbjob.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-13
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <kdebug.h>
#include <kio/scheduler.h>

#include <qimage.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qcstring.h>
#include <qdatastream.h>

extern "C"
{
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
}

#include "camerathumbjob.h"

class CameraThumbJobPriv
{
public:

    KFileItemList fileList;
    int           size;
    KFileItem*    curr_item;
    KFileItem*    next_item;

    KIO::Slave*   slave; 

    int           shmid;
    uchar        *shmaddr;
};

CameraThumbJob::CameraThumbJob(KIO::Slave* slave, const KFileItemList& items, int size)
    : KIO::Job(false)
{
    d = new CameraThumbJobPriv;
    d->slave     = slave;

    d->fileList  = items;
    d->size      = size;
    d->curr_item = d->fileList.first();
    d->next_item = d->curr_item;

    d->shmid   = -1;
    d->shmaddr = 0;
    
    processNext();
}

CameraThumbJob::~CameraThumbJob()
{
    if (d->shmaddr) {
        ::shmdt((char*)d->shmaddr);
        ::shmctl(d->shmid, IPC_RMID, 0);
    }
    delete d;    
}

void CameraThumbJob::processNext()
{
    if (d->fileList.isEmpty()) {
        m_error = 0;
        emitResult();
        return;
    }

    if (d->next_item != d->fileList.current())
    {
        if (d->fileList.findRef(d->next_item) == -1)
        {
            d->fileList.first();
        }
    }

    d->curr_item = d->fileList.current();
    d->fileList.remove();
    d->next_item = d->fileList.current();

    KURL url(d->curr_item->url());

    KIO::TransferJob *job = KIO::get(url, false, false);
    KIO::Scheduler::assignJobToSlave(d->slave, job);
    addSubjob(job);

    job->addMetaData("thumbnail", "1");
    job->addMetaData("size", QString::number(d->size));

    createShmSeg();
    if (d->shmid != -1)
    {
        job->addMetaData("shmid", QString::number(d->shmid));
    }

    connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)),
            this, SLOT(slotData(KIO::Job *, const QByteArray &)));
}

void CameraThumbJob::slotResult(KIO::Job *job)
{
    subjobs.remove(job);
    Q_ASSERT( subjobs.isEmpty() );
    
    if (job->error() && d->curr_item)
    {
        kdWarning() << "Failed result " << d->curr_item->url()
                    << endl;
        //emit signalFailed(d->curr_item);
    }

    processNext();
}

void CameraThumbJob::slotData(KIO::Job *, const QByteArray &data)
{
    if (data.isEmpty())
    {
        return;
    }

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

    if (thumb.isNull())
    {
        kdWarning() << "Null Thumbnail" << endl;
        return;
    }

    QPixmap pix(thumb.smoothScale(d->size, d->size, QImage::ScaleMin));

    int w = pix.width();
    int h = pix.height();
    QPainter p(&pix);
    p.setPen(QPen(QColor(0,0,0),1));
    p.drawRect(0,0,w,h);
    p.setPen(QPen(QColor(255,255,255),1));
    p.drawRect(1,1,w-2,h-2);
    p.end();

    emit signalThumbnail(d->curr_item, pix);
}

void CameraThumbJob::createShmSeg()
{
    if (d->shmid == -1)
    {
        if (d->shmaddr) {
            shmdt((char*)d->shmaddr);
            shmctl(d->shmid, IPC_RMID, 0);
        }
        d->shmid = shmget(IPC_PRIVATE, d->size * d->size * 4,
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

#include "camerathumbjob.moc"

/* ============================================================
 * File  : thumbnailjob.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-10-14
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

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

#include <qstring.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qcolor.h>
#include <qdatastream.h>

#include <kstandarddirs.h>
#include <kmdcodec.h>
#include <kglobal.h>
#include <kdebug.h>

extern "C"
{
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
}

#include "thumbnailjob.h"

namespace Digikam
{

class ThumbnailJobPriv
{
public:

    enum {
        STATE_STATFILE,
        STATE_CREATETHUMB,
        STATE_STATDIR
    } state;
    
    KURL::List urlList;
    QString    thumbRoot;
    int        size;
    int        cacheSize;
    bool       dir;
    bool       highlight;

    KURL       curr_url;
    time_t     curr_mtime;
    QString    curr_uri;
    QString    curr_thumb;
    QString    curr_dirfile;
    time_t     curr_dirfile_mtime;

    // Shared memory segment Id. The segment is allocated to a size
    // of extent x extent x 4 (32 bit image) on first need.
    int    shmid;
    // And the data area
    uchar *shmaddr;
};

ThumbnailJob::ThumbnailJob(const KURL& url, int size,
                           bool dir, bool highlight)
    : KIO::Job(false)
{
    d = new ThumbnailJobPriv;

    d->urlList.append(url);
    d->size = size;
    d->dir  = dir;
    d->highlight = highlight;

    d->shmid = -1;
    d->shmaddr = 0;
    
    createThumbnailDirs();
    processNext();
}

ThumbnailJob::ThumbnailJob(const KURL::List& urlList, int size,
                           bool highlight)
    : KIO::Job(false)
{
    d = new ThumbnailJobPriv;

    d->urlList = urlList;
    d->size = size;
    d->dir  = false;
    d->highlight = highlight;
    
    d->shmid = -1;
    d->shmaddr = 0;

    createThumbnailDirs();
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
    if (subjobs.isEmpty())
        processNext();
}

void ThumbnailJob::addItems(const KURL::List& urlList)
{
    d->urlList += urlList;
    if (subjobs.isEmpty())
        processNext();
}

void ThumbnailJob::createThumbnailDirs()
{
    d->thumbRoot = QDir::homeDirPath() + "/.thumbnails/";
    
    if (d->size <= 128)
        d->cacheSize = 128;
    else
        d->cacheSize = 256;

    d->thumbRoot += d->cacheSize == 128 ? "normal/" : "large/";
    KStandardDirs::makeDir(d->thumbRoot, 0700);
}

void ThumbnailJob::processNext()
{
    if (d->urlList.isEmpty()) {
        emit signalCompleted();
        return;
    }

    // First, stat the original file
    d->curr_url = d->urlList.first();
    d->urlList.pop_front();

    if (d->dir)
        d->state = ThumbnailJobPriv::STATE_STATDIR;
    else 
        d->state = ThumbnailJobPriv::STATE_STATFILE;
    
    KIO::Job *job = KIO::stat(d->curr_url, false);
    addSubjob(job);
}

void ThumbnailJob::slotResult(KIO::Job *job)
{
    subjobs.remove(job);
    Q_ASSERT( subjobs.isEmpty() );
    
    switch(d->state) {

    case (ThumbnailJobPriv::STATE_STATFILE): {
        if (job->error()) {
            processNext();
            return;
        }

        KIO::UDSEntry entry = ((KIO::StatJob*)job)->statResult();
        KIO::UDSEntry::ConstIterator it = entry.begin();
        d->curr_mtime = 0;
        for( ; it != entry.end(); it++ ) {
            if ((*it).m_uds == KIO::UDS_MODIFICATION_TIME) {
                d->curr_mtime = (time_t)((*it).m_long);
                break;
            }
        }

        if (statThumbnail()) {
            processNext();
            return;
        }

        createThumbnail();
        return;
    }

    case(ThumbnailJobPriv::STATE_STATDIR): {

        if (job->error()) {
            processNext();
            return;
        }

        if (statThumbnail()) {
            processNext();
            return;
        }

        // We couldn't find the thumbnail. create it
        createFolderThumbnail();

        return;
    }

    case(ThumbnailJobPriv::STATE_CREATETHUMB): {

        if (job->error()) {
            emit signalFailed(d->curr_url);
        }

        processNext();
        return;
    }

    default: 
        break;
    }

}

bool ThumbnailJob::statThumbnail()
{
    d->curr_uri = "file://" + QDir::cleanDirPath(d->curr_url.path(1));

    KMD5 md5( QFile::encodeName( d->curr_uri ) );
    d->curr_thumb = QFile::encodeName( md5.hexDigest() ) + ".png";

    QImage thumb;
    if (!thumb.load(d->thumbRoot + d->curr_thumb))
        return false;

    if (d->dir) {
        QString dirFile(thumb.text("Digikam::Highlight",0));
        int  dirFileMTime = thumb.text("Digikam::MTime",0).toInt();
        if (dirFile.isEmpty()) return false;

        QString dirFilePath = d->curr_url.path() + QString("/")
                              + dirFile;

        struct stat info;
        if (stat(dirFilePath.latin1(), &info) != 0)
            return false;

        if (info.st_mtime != dirFileMTime)
            return false;
    }
    else {
        if (thumb.text("Thumb::URI", 0) != d->curr_uri ||
            thumb.text("Thumb::MTime", 0).toInt() != d->curr_mtime)
            return false;
    }

    // Found it
    emitThumbnail(thumb);
    return true;
}

void ThumbnailJob::createThumbnail()
{
    d->state = ThumbnailJobPriv::STATE_CREATETHUMB;
    KURL url(d->curr_url);
    url.setProtocol("digikamthumbnail");

    KIO::TransferJob *job = KIO::get(url, false, false);
    addSubjob(job);

    connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)),
            this, SLOT(slotThumbData(KIO::Job *, const QByteArray &)));

    job->addMetaData("size", QString::number(d->cacheSize));

    createShmSeg();
    if (d->shmid != -1)
        job->addMetaData("shmid", QString::number(d->shmid));
}

void ThumbnailJob::createFolderThumbnail()
{
    QDir dir(d->curr_url.path());
    dir.setFilter(QDir::Files);
    const QFileInfoList *list = dir.entryInfoList();
    if (!list) {
        kdWarning() << "Could not read Directory"
                    << d->curr_url.path() << endl;
        processNext();
        return;
    }
    
    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    bool found = false;
    
    while ( (fi = it.current()) != 0 ) {
        if (QImage::imageFormat(fi->absFilePath()) != 0) {
            found = true;
            break;
        }
        ++it;
    }

    // No images found.
    if (!found) {
        processNext();
        return;
    }

    d->curr_dirfile       = fi->fileName();
    d->curr_dirfile_mtime = fi->lastModified().toTime_t();
        
    d->state = ThumbnailJobPriv::STATE_CREATETHUMB;
    KURL url(fi->absFilePath());
    url.setProtocol("digikamthumbnail");

    KIO::TransferJob *job = KIO::get(url, false, false);
    addSubjob(job);

    connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)),
            this, SLOT(slotThumbData(KIO::Job *, const QByteArray &)));

    job->addMetaData("size", QString::number(d->cacheSize));

    createShmSeg();
    if (d->shmid != -1)
        job->addMetaData("shmid", QString::number(d->shmid));
}

void ThumbnailJob::createShmSeg()
{
    if (d->shmid == -1)
    {
        if (d->shmaddr) {
            shmdt((char*)d->shmaddr);
            shmctl(d->shmid, IPC_RMID, 0);
        }
        d->shmid = shmget(IPC_PRIVATE, d->cacheSize * d->cacheSize * 4,
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
    if (data.isEmpty()) return;

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
        kdWarning() << "thumbnail is null" << endl;
        return;
    }

    if (!d->dir) {
        // set the thumbnail attributes using the 
        // freedesktop.org standards
        thumb.setText(QString("Thumb::URI").latin1(),
                      0, d->curr_uri);
        thumb.setText(QString("Thumb::MTime").latin1(),
                      0, QString::number(d->curr_mtime));
        thumb.setText(QString("Software").latin1(),
                      0, QString("Digikam Thumbnail Generator"));
    }
    else {
        thumb.setText(QString("Digikam::Highlight").latin1(),
                      0, d->curr_dirfile);
        thumb.setText(QString("Digikam::MTime").latin1(),
                      0, QString::number(d->curr_dirfile_mtime));
        thumb.setText(QString("Software").latin1(),
                      0, QString("Digikam Thumbnail Generator"));
    }
        
    thumb.save(d->thumbRoot + d->curr_thumb, "PNG", 0);

    emitThumbnail(thumb);
}

void ThumbnailJob::emitThumbnail(QImage& thumb)
{
    // Scale and highlight
    thumb = thumb.smoothScale(d->size, d->size,
                              QImage::ScaleMin);

    // highlight only when requested and when thumbnail
    // width and height are greater than 10
    if (d->highlight && (thumb.width() >= 10 && thumb.height() >= 10))
        highlight(thumb);

    emit signalThumbnail(d->curr_url, QPixmap(thumb));
}

void ThumbnailJob::highlight(QImage& thumb)
{
    QColor darkColor(48, 48, 48);
    QColor lightColor(215, 215, 215);

    int w = thumb.width();
    int h = thumb.height();

    // Right
    for (int y=0; y<h; y++) {
        if (y > 1 && y < h-2)
            thumb.setPixel(w-3, y, lightColor.rgb());
        thumb.setPixel(w-1, y, darkColor.rgb());
        thumb.setPixel(w-2, y, darkColor.rgb());
    }

    // Bottom
    for (int x=0; x<w; x++) {
        if (x > 1 && x < w-2)
            thumb.setPixel(x, h-3, lightColor.rgb());
        thumb.setPixel(x, h-1, darkColor.rgb());
        thumb.setPixel(x, h-2, darkColor.rgb());
    }

    // Top
    for (int x=0; x<w; x++) {
        if (x > 1 && x < w-2)
            thumb.setPixel(x, 2, lightColor.rgb());
        thumb.setPixel(x, 0, darkColor.rgb());
        thumb.setPixel(x, 1, darkColor.rgb());
    }

    // Left
    for (int y=0; y<h; y++) {
        if (y > 1 && y < h-2)
            thumb.setPixel(2, y, lightColor.rgb());
        thumb.setPixel(0, y, darkColor.rgb());
        thumb.setPixel(1, y, darkColor.rgb());
    }
    
}


}



#include "thumbnailjob.moc"

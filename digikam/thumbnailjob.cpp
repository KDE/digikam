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
#include <qpainter.h>
#include <qcolor.h>
#include <qdatastream.h>
#include <qtimer.h>

#include <kstandarddirs.h>
#include <kmdcodec.h>
#include <kglobal.h>
#include <kfilemetainfo.h>
#include <kdebug.h>
#include <ktempfile.h>

#include "albumsettings.h"
#include "exiforientation_p.h"
#include "thumbdb.h"
#include "thumbnailjob.h"

extern "C"
{
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
}

namespace Digikam
{

class ThumbnailJobPriv
{
public:

    KURL::List    urlList;
    QString       thumbRoot;
    int           size;
    int           cacheSize;
    bool          dir;
    bool          highlight;
    bool          metainfo;

    KURL          curr_url;
    KURL          next_url;
    time_t        curr_mtime;
    QString       curr_uri;
    QString       curr_thumb;
    QString       curr_dirfile;

    // Shared memory segment Id. The segment is allocated to a size
    // of extent x extent x 4 (32 bit image) on first need.
    int    shmid;
    // And the data area
    uchar *shmaddr;

    bool          running;
    QTimer*       timer;
};

ThumbnailJob::ThumbnailJob(const KURL& url, int size,
                           bool dir, bool highlight)
    : KIO::Job(false)
{
    d = new ThumbnailJobPriv;

    d->urlList.append(url);
    d->size      = size;
    d->dir       = dir;
    d->highlight = highlight;
    d->metainfo  = false;
    d->running   = false;

    d->curr_url = d->urlList.first();
    d->next_url = d->curr_url;
    
    d->shmid   = -1;
    d->shmaddr = 0;

    d->timer = new QTimer;
    connect(d->timer, SIGNAL(timeout()),
            SLOT(slotTimeout()));
    
    createThumbnailDirs();

    processNext();
}

ThumbnailJob::ThumbnailJob(const KURL::List& urlList, int size,
                           bool metainfo, bool highlight)
    : KIO::Job(false)
{
    d = new ThumbnailJobPriv;

    d->urlList   = urlList;
    d->size      = size;
    d->dir       = false;
    d->highlight = highlight;
    d->metainfo  = metainfo;
    d->running   = false;

    d->curr_url = d->urlList.first();
    d->next_url = d->curr_url;
    
    d->shmid = -1;
    d->shmaddr = 0;

    d->timer = new QTimer;
    connect(d->timer, SIGNAL(timeout()),
            SLOT(slotTimeout()));

    createThumbnailDirs();

    processNext();
}

ThumbnailJob::~ThumbnailJob()
{
    delete d->timer;

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

    
    // First, stat the original file
    d->running = true;
    d->timer->start(0, true);
}

void ThumbnailJob::slotResult(KIO::Job *job)
{
    subjobs.remove(job);
    Q_ASSERT( subjobs.isEmpty() );
    
    if (job->error())
    {
        emit signalFailed(d->curr_url);
    }

    processNext();
}

bool ThumbnailJob::statThumbnail()
{
    d->curr_uri = "file://" + QDir::cleanDirPath(d->curr_url.path(1));

    KMD5 md5( QFile::encodeName( d->curr_uri ) );
    d->curr_thumb = QFile::encodeName( md5.hexDigest() ) + ".png";

    QString file = d->thumbRoot + d->curr_thumb;

    if (!d->dir)
    {
        struct stat stbuf;
        if (::stat(QFile::encodeName(file), &stbuf) == -1)
        {
            // stat failed
            return false;
        }

        if (stbuf.st_mtime < d->curr_mtime) {
            // thumbnail needs updating
            return false;
        }
    }

    QImage thumb(file);
    if (thumb.isNull())
        return false;
        
    if (!ThumbDB::instance()->hasThumb(d->curr_url.path(1)))
    {
        QImage thumb(file);
        ThumbDB::instance()->putThumb(d->curr_url.path(1), thumb);
    }
    
    emitThumbnail(thumb);
    return true;
}

void ThumbnailJob::createThumbnail()
{
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
        kdWarning() << k_funcinfo << "Could not read Directory"
                    << d->curr_url.path() << endl;
        processNext();
        return;
    }
    
    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    bool found = false;
    
    while ( (fi = it.current()) != 0 )
    {
        if (QImage::imageFormat(fi->absFilePath()) != 0)
        {
            found = true;
            break;
        }
        ++it;
    }

    // No images found.
    if (!found)
    {
        processNext();
        return;
    }

    d->curr_dirfile = fi->fileName();
        
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
        thumb.setText(QString("Software").latin1(),
                      0, QString("Digikam Thumbnail Generator"));
    }

    KTempFile temp(d->thumbRoot + "digikam-tmp-", ".png");
    if (temp.status() == 0)
    {
        thumb.save(temp.name(), "PNG", 0);
        ::rename(QFile::encodeName(temp.name()),
                 QFile::encodeName(d->thumbRoot + d->curr_thumb));
    }
    
    ThumbDB::instance()->putThumb( d->curr_url.path(1), thumb );
    
    emitThumbnail(thumb);
}

void ThumbnailJob::emitThumbnail(QImage& thumb)
{
    if (thumb.isNull())
    {
        return;
    }

    thumb = thumb.smoothScale(d->size, d->size, QImage::ScaleMin);

    // EXIF rotate the thumbnail if requested so
    if (AlbumSettings::instance()->getExifRotate())
    {
        exifRotate(d->curr_url.path(), thumb);
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
        p.setPen(QPen(QColor(255,255,255),1));
        p.drawRect(1,1,w-2,h-2);
        p.end();
    }

    KFileMetaInfo *metaInfo = 0;
    if (d->metainfo)
    {
        metaInfo = new KFileMetaInfo(d->curr_url.path());
    }

    emit signalThumbnailMetaInfo(d->curr_url, pix, metaInfo);
}

void ThumbnailJob::exifRotate(const QString& filePath, QImage& thumb)
{
    // Rotate thumbnail based on EXIF rotate tag
    QWMatrix matrix;

    KExifData::ImageOrientation orientation
        = getExifOrientation(filePath);
    
    bool doXform = (orientation != KExifData::NORMAL &&
                    orientation != KExifData::UNSPECIFIED);

    switch (orientation) {
       case KExifData::NORMAL:
       case KExifData::UNSPECIFIED:
          break;

       case KExifData::HFLIP:
          matrix.scale(-1,1);
          break;

       case KExifData::ROT_180:
          matrix.rotate(180);
          break;

       case KExifData::VFLIP:
          matrix.scale(1,-1);
          break;

       case KExifData::ROT_90_HFLIP:
          matrix.scale(-1,1);
          matrix.rotate(90);
          break;

       case KExifData::ROT_90:
          matrix.rotate(90);
          break;

       case KExifData::ROT_90_VFLIP:
          matrix.scale(1,-1);
          matrix.rotate(90);
          break;

       case KExifData::ROT_270:
          matrix.rotate(270);
          break;
    }

    //transform accordingly
    if ( doXform )
       thumb = thumb.xForm( matrix );
}

void ThumbnailJob::slotTimeout()
{
    d->running = false;
    
    struct stat stbuf;
    if (::stat(QFile::encodeName(d->curr_url.path()), &stbuf) == -1)
    {
        kdWarning() << k_funcinfo << "Stat failed for url "
                    << d->curr_url.prettyURL() << endl;
        emit signalStatFailed(d->curr_url, d->dir);
        processNext();
        return;
    }

    d->curr_mtime = stbuf.st_mtime;

    if (statThumbnail())
    {
        processNext();
        return;
    }
    
    if (d->dir)
    {
        createFolderThumbnail();
        return;
    }
    else
    {
        createThumbnail();
        return;
    }
}

}

#include "thumbnailjob.moc"

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
#include <kdebug.h>

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
#include <X11/Xlib.h>
#include <Imlib2.h>
}

namespace Digikam
{

class ThumbnailJobPriv
{
public:

    KFileItemList fileList;
    QString       thumbRoot;
    int           size;
    int           cacheSize;
    bool          dir;
    bool          highlight;
    bool          metainfo;

    const KFileItem*  next_item;
    KFileItem*    curr_item;
    time_t        curr_mtime;
    QString       curr_uri;
    QString       curr_thumb;
    QString       curr_dirfile;

    // Shared memory segment Id. The segment is allocated to a size
    // of extent x extent x 4 (32 bit image) on first need.
    int    shmid;
    // And the data area
    uchar *shmaddr;

    Display      *display;
    Visual       *vis;
    GC            gc;
    Colormap      cm;
    Imlib_Context context;
    Imlib_Image   image;

    bool          running;
};

ThumbnailJob::ThumbnailJob(const KFileItem* fileItem, int size,
                           bool dir, bool highlight)
    : KIO::Job(false)
{
    d = new ThumbnailJobPriv;

    d->fileList.append(fileItem);
    d->size = size;
    d->dir  = dir;
    d->highlight = highlight;
    d->metainfo  = false;
    d->running   = false;

    d->curr_item = d->fileList.first();
    d->next_item = d->curr_item;
    
    d->shmid   = -1;
    d->shmaddr = 0;

    d->display = QPaintDevice::x11AppDisplay();
    d->vis     = DefaultVisual(d->display, DefaultScreen(d->display));
    d->cm      = DefaultColormap(d->display, DefaultScreen(d->display));
    d->context = imlib_context_new();
    d->image   = 0;
    imlib_context_push(d->context);
    imlib_set_cache_size(0);
    imlib_set_color_usage(128);
    imlib_context_set_dither(1);
    imlib_context_set_display(d->display);
    imlib_context_set_visual(d->vis);
    imlib_context_set_colormap(d->cm);
    imlib_context_pop();
    
    createThumbnailDirs();

    processNext();
}

ThumbnailJob::ThumbnailJob(const KFileItemList& itemList, int size,
                           bool metainfo, bool highlight)
    : KIO::Job(false)
{
    d = new ThumbnailJobPriv;

    d->fileList  = itemList;
    d->size      = size;
    d->dir       = false;
    d->highlight = highlight;
    d->metainfo  = metainfo;
    d->running   = false;

    d->curr_item = d->fileList.first();
    d->next_item = d->curr_item;
    
    d->shmid = -1;
    d->shmaddr = 0;

    d->display = QPaintDevice::x11AppDisplay();
    d->vis   = DefaultVisual(d->display, DefaultScreen(d->display));
    d->cm    = DefaultColormap(d->display, DefaultScreen(d->display));
    d->context = imlib_context_new();
    d->image   = 0;
    imlib_context_push(d->context);
    imlib_set_cache_size(0);
    imlib_set_color_usage(128);
    imlib_context_set_dither(1);
    imlib_context_set_display(d->display);
    imlib_context_set_visual(d->vis);
    imlib_context_set_colormap(d->cm);
    imlib_context_pop();

    createThumbnailDirs();

    processNext();
}

ThumbnailJob::~ThumbnailJob()
{
    imlib_context_free(d->context);
    
    if (d->shmaddr) {
        shmdt((char*)d->shmaddr);
        shmctl(d->shmid, IPC_RMID, 0);
    }
    delete d;
}

void ThumbnailJob::addItem(const KFileItem* item)
{
    d->fileList.append(item);

    if (!d->running && subjobs.isEmpty())
        processNext();
}

void ThumbnailJob::addItems(const KFileItemList& itemList)
{
    KFileItemListIterator it(itemList);
    while (it.current() != 0) {
        d->fileList.append(it.current());
        ++it;
    }

    if (!d->running && subjobs.isEmpty())
        processNext();
}

bool ThumbnailJob::setNextItemToLoad(const KFileItem* item)
{
    if (d->fileList.containsRef(item))
    {
        d->next_item = item;
        return true;
    }

    return false;
}

void ThumbnailJob::removeItem(const KFileItem *fileItem)
{
    d->fileList.removeRef(fileItem);

    if (d->next_item == fileItem)
        d->next_item = d->fileList.current();
    if (d->curr_item == fileItem)
        d->curr_item = 0;
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
    if (d->fileList.isEmpty()) {
        emit signalCompleted();
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
    
    // First, stat the original file
    d->running = true;
    QTimer::singleShot(0, this, SLOT(slotTimeout()));
}

void ThumbnailJob::slotResult(KIO::Job *job)
{
    subjobs.remove(job);
    Q_ASSERT( subjobs.isEmpty() );
    
    if (job->error() && d->curr_item)
    {
        emit signalFailed(d->curr_item);
    }

    processNext();
}

bool ThumbnailJob::statThumbnail()
{
    if (!d->curr_item)
        return false;
    
    d->curr_uri = "file://" + QDir::cleanDirPath(d->curr_item->url().path(1));

    KMD5 md5( QFile::encodeName( d->curr_uri ) );
    d->curr_thumb = QFile::encodeName( md5.hexDigest() ) + ".png";

    QString file = d->thumbRoot + d->curr_thumb;

    if (!d->dir)
    {
        struct stat stbuf;
        if (::stat(file.latin1(), &stbuf) == -1)
        {
            // stat failed
            return false;
        }

        if (stbuf.st_mtime < d->curr_mtime) {
            // thumbnail needs updating
            return false;
        }
    }
        
    d->image = imlib_load_image_immediately_without_cache(file.latin1());
    if (!d->image)
    {
        return false;
    }

    if (!ThumbDB::instance()->hasThumb(d->curr_item->url().path(1)))
    {
        QImage thumb(file);
        ThumbDB::instance()->putThumb(d->curr_item->url().path(1), thumb);
    }
    
    imlib_context_push(d->context);
    imlib_context_set_image(d->image);
    imlib_context_pop();

    emitThumbnail();
    return true;
}

void ThumbnailJob::createThumbnail()
{
    if (!d->curr_item)
        return;
    
    KURL url(d->curr_item->url());
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
    if (!d->curr_item)
        return;

    QDir dir(d->curr_item->url().path());
    dir.setFilter(QDir::Files);
    const QFileInfoList *list = dir.entryInfoList();
    if (!list) {
        kdWarning() << k_funcinfo << "Could not read Directory"
                    << d->curr_item->url().path() << endl;
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
        kdWarning() << k_funcinfo << "thumbnail is null" << endl;
        if (d->curr_item)
        {
            emit signalFailed(d->curr_item);
        }
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
        
    thumb.save(d->thumbRoot + d->curr_thumb, "PNG", 0);

    ThumbDB::instance()->putThumb( d->curr_item->url().path(1), thumb );

    d->image = imlib_create_image(thumb.width(), thumb.height());
    imlib_context_push(d->context);
    imlib_context_set_image(d->image);
    uint *imgData = imlib_image_get_data();
    memcpy(imgData, thumb.bits(), thumb.numBytes());
    imlib_image_put_back_data(imgData);
    imlib_context_pop();
    
    emitThumbnail();
}

void ThumbnailJob::emitThumbnail()
{
    if (!d->image || !d->curr_item)
        return;

    imlib_context_push(d->context);
    imlib_context_set_image(d->image);

    int w = imlib_image_get_width();
    int h = imlib_image_get_height();

    QSize size(w, h);
    size.scale(d->size, d->size, QSize::ScaleMin);

    w = size.width();
    h = size.height();
    
    QPixmap pix(w, h);
    imlib_context_set_drawable(pix.handle());
    imlib_context_set_anti_alias(1);
    imlib_render_image_on_drawable_at_size(0, 0, w, h);

    imlib_free_image();
    d->image = 0;
    imlib_context_pop();

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
        metaInfo = new KFileMetaInfo(d->curr_item->url());
    }

    emit signalThumbnailMetaInfo(d->curr_item, pix, metaInfo);
}

void ThumbnailJob::slotTimeout()
{
    d->running = false;
    
    if (!d->curr_item)
    {
        processNext();
        return;
    }
        
    struct stat stbuf;
    if (::stat(d->curr_item->url().path().latin1(), &stbuf) == -1)
    {
        kdWarning() << k_funcinfo << "Stat failed for url "
                    << d->curr_item->url().prettyURL() << endl;
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

/* ============================================================
 * File  : thumbnailjob.h
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

#ifndef THUMBNAILJOB_H
#define THUMBNAILJOB_H

#include <kio/job.h>
#include <kurl.h>
#include <kfileitem.h>
#include <qcstring.h>

class QPixmap;
class QImage;
class KFileMetaInfo;

namespace Digikam
{

class ThumbnailJobPriv;

class ThumbnailJob : public KIO::Job
{
    Q_OBJECT

public:

    ThumbnailJob(const KFileItem* item, int size, bool dir=false,
                 bool highlight=true);
    ThumbnailJob(const KFileItemList& fileList, int size, bool metainfo=true,
                 bool highlight=true);
    ~ThumbnailJob();

    void addItem(const  KFileItem* fileItem);
    void addItems(const KFileItemList& fileList);

    bool setNextItemToLoad(const KFileItem *fileItem);
    void removeItem(const KFileItem *fileItem);
    
signals:

    void signalThumbnailMetaInfo(const KFileItem* item, const QPixmap& pix,
                                 const KFileMetaInfo* metaInfo);
    void signalCompleted();
    void signalFailed(const KFileItem* item);

private:

    ThumbnailJobPriv *d;

private:

    void createThumbnailDirs();
    void processNext();
    bool statThumbnail();
    void createThumbnail();
    void createFolderThumbnail();
    void emitThumbnail();
    void createShmSeg();

protected slots:

    void slotResult(KIO::Job *job);
    void slotThumbData(KIO::Job *job, const QByteArray &data);
    void slotTimeout();
};

}

#endif /* THUMBNAILJOB_H */

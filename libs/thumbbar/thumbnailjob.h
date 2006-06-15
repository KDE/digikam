/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2003-10-14
 * Description : digiKam KIO thumbnails generator interface
 *
 * Copyright 2003-2005 by Renchi Raju
 * Copyright      2006 by Gilles Caulier
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

#ifndef THUMBNAILJOB_H
#define THUMBNAILJOB_H

// Qt includes.

#include <qcstring.h>

// KDE includes.

#include <kio/job.h>
#include <kurl.h>

class QPixmap;
class QImage;

namespace Digikam
{

class ThumbnailJobPriv;

class ThumbnailJob : public KIO::Job
{
    Q_OBJECT

public:

    ThumbnailJob(const KURL& url, int size,
                 bool highlight=true, bool exifRotate=false);
    ThumbnailJob(const KURL::List& urlList, int size,
                 bool highlight=true, bool exifRotate=false);
    ~ThumbnailJob();

    void addItem(const  KURL& url);
    void addItems(const KURL::List& urlList);

    bool setNextItemToLoad(const KURL& url);
    void removeItem(const KURL& url);

signals:

    void signalThumbnail(const KURL& url, const QPixmap& pix);
    void signalCompleted();
    void signalFailed(const KURL& url);

private:

    void processNext();
    void emitThumbnail(QImage& thumb);
    void createShmSeg();

protected slots:

    void slotResult(KIO::Job *job);
    void slotThumbData(KIO::Job *job, const QByteArray &data);

private:

    ThumbnailJobPriv *d;
};

}  // namespace Digikam

#endif /* THUMBNAILJOB_H */

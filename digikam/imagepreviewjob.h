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

#ifndef IMAGEPREVIEWJOB_H
#define IMAGEPREVIEWJOB_H

// KDE includes.

#include <kio/job.h>

class QImage;

class KURL;

namespace Digikam
{

class ImagePreviewJobPriv;

class ImagePreviewJob : public KIO::Job
{
    Q_OBJECT

public:

    ImagePreviewJob(const KURL& url, int size, bool exifRotate=false);
    ~ImagePreviewJob();

signals:

    void signalImagePreview(const KURL& url, const QImage& preview);
    void signalCompleted();
    void signalFailed(const KURL& url);

private:

    void getImagePreview();
    void createShmSeg();

protected slots:

    void slotResult(KIO::Job *job);
    void slotImagePreviewData(KIO::Job *job, const QByteArray &data);

private:

    ImagePreviewJobPriv *d;
};

}  // namespace Digikam

#endif /* IMAGEPREVIEWJOB_H */

/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2003-01-15
 * Description : digiKam KIO slave to get image thumbnails.
 *
 * Copyright 2003-2005 by Renchi Raju, Gilles Caulier
 * Copyright 2006      by Gilles Caulier
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

#ifndef _digikamthumbnail_H_
#define _digikamthumbnail_H_

// KDE includes.

#include <kio/slavebase.h>

class QString;
class QImage;
class QApplication;

class KURL;

class kio_digikamthumbnailProtocol : public KIO::SlaveBase
{

public:

    kio_digikamthumbnailProtocol(int argc, char** argv);
    virtual ~kio_digikamthumbnailProtocol();
    virtual void get(const KURL& url);

private:

    bool   loadByExtension(QImage& image, const QString& path);
    bool   loadJPEG(QImage& image, const QString& path);
    void   exifRotate(const QString& filePath, QImage& thumb);
    QImage loadPNG(const QString& path);
    bool   loadDImg(QImage& image, const QString& path);
    bool   loadKDEThumbCreator(QImage& image, const QString& path);
    void   createThumbnailDirs();

private:

    int           cachedSize_;

    int           org_width_;
    int           org_height_;
    int           new_width_;
    int           new_height_;

    int           argc_;
    char        **argv_;

    QString       smallThumbPath_;
    QString       bigThumbPath_;

    QApplication *app_;
};

#endif  // _digikamthumbnail_H_

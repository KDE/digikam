//////////////////////////////////////////////////////////////////////////////
//
//    DIGIKAMTHUMBNAIL.H
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles CAULIER <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _digikamthumbnail_H_
#define _digikamthumbnail_H_

// KDE includes.

#include <kio/slavebase.h>

class KURL;
class QCString;
class QString;
class QImage;

class kio_digikamthumbnailProtocol : public KIO::SlaveBase
{
public:

    kio_digikamthumbnailProtocol(const QCString &pool_socket,
                                 const QCString &app_socket);
    virtual ~kio_digikamthumbnailProtocol();
    virtual void get(const KURL& url);

private:

    bool loadJPEG(QImage& image, const QString& path);
    bool loadImlib2(QImage& image, const QString& path);
    bool loadDCRAW(QImage& image,  const QString& path);
    void createThumbnailDirs();

    int  cachedSize_;

    int org_width_;
    int org_height_;
    int new_width_;
    int new_height_;

    QString smallThumbPath_;
    QString bigThumbPath_;
};

#endif  // _digikamthumbnail_H_

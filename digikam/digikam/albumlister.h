/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-26
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#ifndef ALBUMLISTER_H
#define ALBUMLISTER_H

#include <qobject.h>
#include <qcstring.h>

#include "imageinfo.h"

namespace KIO
{
class Job;
}

class PAlbum;
class AlbumListerPriv;

class AlbumLister : public QObject
{
    Q_OBJECT

public:

    AlbumLister();
    ~AlbumLister();

    void openAlbum(Album *album);
    void stop();

    void setNameFilter(const QString& nameFilter);
    void updateDirectory();

private:

    AlbumListerPriv *d;
    
signals:

    void signalNewItems(const ImageInfoList& items);
    void signalDeleteItem(ImageInfo* item);
    void signalClear();
    void signalCompleted();

private slots:

    void slotClear();

    void slotResult(KIO::Job* job);
    void slotData(KIO::Job* job, const QByteArray& data);
};

#endif /* ALBUMLISTER_H */

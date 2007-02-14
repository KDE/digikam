/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2007-14-02
 * Description : interface to get image info from an albums list.
 *
 * Copyright 2007 by Gilles Caulier
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

#ifndef IMAGEINFOALBUMSJOB_H
#define IMAGEINFOALBUMSJOB_H

// Qt includes.

#include <qobject.h>
#include <qcstring.h>

// Local includes.

#include "albummanager.h"
#include "imageinfo.h"

namespace KIO
{
class Job;
}

namespace Digikam
{

class ImageInfoAlbumsJobPriv;

class ImageInfoAlbumsJob : public QObject
{
    Q_OBJECT

public:

    ImageInfoAlbumsJob();
    ~ImageInfoAlbumsJob();

    void allItemsFromAlbums(const AlbumList& albumsList);
    void stop();
    
signals:

    void signalCompleted(const ImageInfoList& items);

private slots:

    void slotItemsInfo(const ImageInfoList&);
    void slotComplete();

private:

    void parseAlbum();

private:

    ImageInfoAlbumsJobPriv *d;
};

}  // namespace Digikam

#endif /* IMAGEINFOALBUMSJOB_H */

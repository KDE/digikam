/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qstring.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albummanager.h"
#include "imageinfojob.h"
#include "imageinfoalbumsjob.h"
#include "imageinfoalbumsjob.moc"

namespace Digikam
{

class ImageInfoAlbumsJobPriv
{
public:

    ImageInfoAlbumsJobPriv(){}

    AlbumList           albumsList;
    AlbumList::Iterator albumIt;

    ImageInfoList       itemsList;

    ImageInfoJob        imageInfoJob;
};

ImageInfoAlbumsJob::ImageInfoAlbumsJob()
{
    d = new ImageInfoAlbumsJobPriv;

    connect(&d->imageInfoJob, SIGNAL(signalItemsInfo(const ImageInfoList&)),
            this, SLOT(slotItemsInfo(const ImageInfoList&)));
    
    connect(&d->imageInfoJob, SIGNAL(signalCompleted()),
            this, SLOT(slotComplete()));
}

ImageInfoAlbumsJob::~ImageInfoAlbumsJob()
{
    delete d;
}

void ImageInfoAlbumsJob::allItemsFromAlbums(const AlbumList& albumsList)
{
    if (albumsList.isEmpty())
        return;
    
    d->albumsList = albumsList;
    d->albumIt    = d->albumsList.begin();
    parseAlbum();
}

void ImageInfoAlbumsJob::parseAlbum()
{
    d->imageInfoJob.allItemsFromAlbum(*d->albumIt);
}

void ImageInfoAlbumsJob::stop()
{
    d->imageInfoJob.stop();
    d->albumsList.clear();
}

void ImageInfoAlbumsJob::slotItemsInfo(const ImageInfoList& items)
{
    ImageInfo* item;
    for (ImageInfoListIterator it(items); (item = it.current()); ++it)
        d->itemsList.append(item);

   ++d->albumIt; 
   if (d->albumIt == d->albumsList.end())
    {
        stop();
        emit signalCompleted(d->itemsList);
        return;
    }
 
    parseAlbum();
}

void ImageInfoAlbumsJob::slotComplete()
{
    ++d->albumIt;
    if (d->albumIt == d->albumsList.end())
    {
        stop();
        emit signalCompleted(d->itemsList);
        return;
    }
 
    parseAlbum();
}

}  // namespace Digikam

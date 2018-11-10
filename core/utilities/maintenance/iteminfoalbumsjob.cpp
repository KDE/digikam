/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-14-02
 * Description : interface to get item info from an albums list.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "iteminfoalbumsjob.h"

// Qt includes

#include <QString>
#include <QUrl>

// Local includes

#include "iteminfojob.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemInfoAlbumsJob::Private
{
public:

    explicit Private() {}

    AlbumList           albumsList;
    AlbumList::Iterator albumIt;

    ItemInfoList       itemsList;

    ItemInfoJob        imageInfoJob;
};

ItemInfoAlbumsJob::ItemInfoAlbumsJob()
    : d(new Private)
{
    connect(&d->imageInfoJob, SIGNAL(signalItemsInfo(ItemInfoList)),
            this, SLOT(slotItemsInfo(ItemInfoList)));

    connect(&d->imageInfoJob, SIGNAL(signalCompleted()),
            this, SLOT(slotComplete()));
}

ItemInfoAlbumsJob::~ItemInfoAlbumsJob()
{
    delete d;
}

void ItemInfoAlbumsJob::allItemsFromAlbums(const AlbumList& albumsList)
{
    if (albumsList.isEmpty())
    {
        return;
    }

    d->albumsList = albumsList;
    d->albumIt    = d->albumsList.begin();
    parseAlbum();
}

void ItemInfoAlbumsJob::parseAlbum()
{
    d->imageInfoJob.allItemsFromAlbum(*d->albumIt);
}

void ItemInfoAlbumsJob::stop()
{
    d->imageInfoJob.stop();
    d->albumsList.clear();
}

void ItemInfoAlbumsJob::slotItemsInfo(const ItemInfoList& items)
{
    d->itemsList += items;
}

void ItemInfoAlbumsJob::slotComplete()
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

} // namespace Digikam

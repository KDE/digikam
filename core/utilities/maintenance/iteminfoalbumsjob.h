/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-14-02
 * Description : interface to get item info from an albums list.
 *
 * Copyright (C) 2007-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_ITEM_INFO_ALBUMS_JOB_H
#define DIGIKAM_ITEM_INFO_ALBUMS_JOB_H

// Qt includes

#include <QObject>

// Local includes

#include "iteminfo.h"
#include "albummanager.h"

namespace Digikam
{

class ItemInfoAlbumsJob : public QObject
{
    Q_OBJECT

public:

    explicit ItemInfoAlbumsJob();
    ~ItemInfoAlbumsJob();

    void allItemsFromAlbums(const AlbumList& albumsList);
    void stop();

Q_SIGNALS:

    void signalCompleted(const ItemInfoList& items);

private Q_SLOTS:

    void slotItemsInfo(const ItemInfoList&);
    void slotComplete();

private:

    void parseAlbum();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_INFO_ALBUMS_JOB_H

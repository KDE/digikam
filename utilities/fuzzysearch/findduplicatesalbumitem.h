/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-06-17
 * Description : Find Duplicates tree-view search album item.
 *
 * Copyright (C) 2008-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FINDDUPLICATESALBUMITEM_H
#define FINDDUPLICATESALBUMITEM_H

// Qt includes

#include <QTreeWidget>

// KDE includes

#include <kurl.h>

// Local includes

#include "imageinfo.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class SAlbum;

class FindDuplicatesAlbumItem : public QTreeWidgetItem
{

public:

    FindDuplicatesAlbumItem(QTreeWidget* const parent, SAlbum* const album);
    virtual ~FindDuplicatesAlbumItem();

    bool hasValidThumbnail() const;

    SAlbum* album()  const;
    KUrl    refUrl() const;

    void setThumb(const QPixmap& pix, bool hasThumb = true);

    bool operator<(const QTreeWidgetItem& other) const;

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* FINDDUPLICATESALBUMITEM_H */

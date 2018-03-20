/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-06-17
 * Description : Find Duplicates tree-view search album item.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "findduplicatesalbumitem.h"

// Qt includes

#include <QPainter>
#include <QCollator>
#include <QIcon>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "coredbsearchxml.h"

namespace Digikam
{

class FindDuplicatesAlbumItem::Private
{

public:

    Private() :
        hasThumb(false),
        album(0),
        itemCount(0)
    {
    }

    bool      hasThumb;

    SAlbum*   album;
    int       itemCount;

    ImageInfo refImgInfo;
};

FindDuplicatesAlbumItem::FindDuplicatesAlbumItem(QTreeWidget* const parent, SAlbum* const album)
    : QTreeWidgetItem(parent),
      d(new Private)
{
    d->album = album;

    if (d->album)
    {
        qlonglong refImage = d->album->title().toLongLong();
        d->refImgInfo      = ImageInfo(refImage);
        setText(Column::REFERENCE_IMAGE, d->refImgInfo.name());

        calculateInfos();
    }

    setThumb(QIcon::fromTheme(QLatin1String("view-preview")).pixmap(parent->iconSize().width(),
                                                                    QIcon::Disabled), false);
}

FindDuplicatesAlbumItem::~FindDuplicatesAlbumItem()
{
    delete d;
}

bool FindDuplicatesAlbumItem::hasValidThumbnail() const
{
    return d->hasThumb;
}

void FindDuplicatesAlbumItem::calculateInfos(const QList<qlonglong>& deletedImages)
{
    if (d->album)
    {
        qlonglong refImage = d->album->title().toLongLong();

        //qCDebug(DIGIKAM_GENERAL_LOG) << "Calculating info for album " << QString::number(refImage);

        SearchXmlReader reader(d->album->query());
        reader.readToFirstField();

        // Get the defined image ids.
        QList<int> list;
        list << reader.valueToIntList();

        // only images that are not removed/obsolete should be shown.
        double avgSim = 0.00;
        QList<int> filteredList;

        foreach (int imageId, list)
        {
            ImageInfo info(imageId);

            // If image is not deleted in this moment and was also not
            // removed before.
            if (!deletedImages.contains(imageId) && !info.isRemoved())
            {
                filteredList << imageId;

                if (imageId != refImage)
                {
                    avgSim += info.similarityTo(refImage);
                }
            }
        }

        d->itemCount = filteredList.count();

        //qCDebug(DIGIKAM_GENERAL_LOG) << "New Item count: " << QString::number(d->itemCount);

        if (d->itemCount > 1)
        {
            if (!filteredList.contains(refImage))
            {
                avgSim = avgSim / (d->itemCount);
            }
            else
            {
                avgSim = avgSim / (d->itemCount - 1 );
            }
        }
        else
        {
            this->setHidden(true);
        }

        setText(Column::RESULT_COUNT,   QString::number(d->itemCount));
        setText(Column::AVG_SIMILARITY, QString::number(avgSim, 'f', 2));
    }
}

int FindDuplicatesAlbumItem::itemCount() const
{
    return d->itemCount;
}

void FindDuplicatesAlbumItem::setThumb(const QPixmap& pix, bool hasThumb)
{
    int iconSize = treeWidget()->iconSize().width();
    QPixmap pixmap(iconSize + 2, iconSize + 2);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.drawPixmap((pixmap.width()  / 2) - (pix.width()  / 2),
                 (pixmap.height() / 2) - (pix.height() / 2), pix);

    QIcon icon = QIcon(pixmap);
    //  We make sure the preview icon stays the same regardless of the role
    icon.addPixmap(pixmap, QIcon::Selected, QIcon::On);
    icon.addPixmap(pixmap, QIcon::Selected, QIcon::Off);
    icon.addPixmap(pixmap, QIcon::Active,   QIcon::On);
    icon.addPixmap(pixmap, QIcon::Active,   QIcon::Off);
    icon.addPixmap(pixmap, QIcon::Normal,   QIcon::On);
    icon.addPixmap(pixmap, QIcon::Normal,   QIcon::Off);
    setIcon(Column::REFERENCE_IMAGE, icon);

    d->hasThumb = hasThumb;
}

SAlbum* FindDuplicatesAlbumItem::album() const
{
    return d->album;
}

QUrl FindDuplicatesAlbumItem::refUrl() const
{
    return d->refImgInfo.fileUrl();
}

bool FindDuplicatesAlbumItem::operator<(const QTreeWidgetItem& other) const
{
    int column = treeWidget()->sortColumn();
    int result = 0;

    if (column == Column::AVG_SIMILARITY)
    {
        result = ( text(column).toDouble() < other.text(column).toDouble() ) ? -1 : 0;
    }
    else if (column == Column::RESULT_COUNT)
    {
        result = ( text(column).toInt() < other.text(column).toInt() ) ? -1 : 0;
    }
    else
    {
        result = QCollator().compare(text(column), other.text(column));
    }

    if (result < 0)
    {
        return true;
    }

    return false;
}

}  // namespace Digikam

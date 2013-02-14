/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-12
 * Description : Table view column helpers
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef TABLEVIEW_COLUMNS_H
#define TABLEVIEW_COLUMNS_H

// Qt includes

#include <QObject>
#include <QPainter>
#include <QStringList>

// KDE includes

// local includes

#include "tableview_columnfactory.h"
#include <libkgeomap/geocoordinates.h>
#include "thumbnailloadthread.h"

namespace Digikam
{

namespace TableViewColumns
{

class ColumnFilename : public TableViewColumn
{
public:

    explicit ColumnFilename(
            TableViewShared* const tableViewShared,
            const TableViewColumnConfiguration& pConfiguration
        )
      : TableViewColumn(tableViewShared, pConfiguration)
    {
    }
    virtual ~ColumnFilename() { }

    static TableViewColumnDescription getDescription()
    {
        return TableViewColumnDescription(QLatin1String("filename"), QLatin1String("Filename"));
    }
    virtual QString getTitle() { return i18n("Filename"); }

    virtual QVariant data(const QModelIndex& sourceIndex, const int role)
    {
        /// @todo is this correct or does sourceIndex have column!=0?
        return sourceIndex.data(role);
    }

};

class ColumnCoordinates : public TableViewColumn
{
public:

    explicit ColumnCoordinates(
            TableViewShared* const tableViewShared,
            const TableViewColumnConfiguration& pConfiguration
        )
      : TableViewColumn(tableViewShared, pConfiguration)
    {
    }
    virtual ~ColumnCoordinates() { }
    static TableViewColumnDescription getDescription()
    {
        return TableViewColumnDescription(QLatin1String("coordinates"), QLatin1String("Coordinates"));
    }
    virtual QString getTitle() { return i18n("Coordinates"); }

    virtual QVariant data(const QModelIndex& sourceIndex, const int role)
    {
        if (role!=Qt::DisplayRole)
        {
            return QVariant();
        }

        const ImageInfo info = s->imageFilterModel->imageInfo(sourceIndex);

        if (info.isNull() || !info.hasCoordinates())
        {
            return QVariant();
        }

        const KGeoMap::GeoCoordinates coordinates(info.latitudeNumber(), info.longitudeNumber());

        return QString("%1,%2").arg(coordinates.latString()).arg(coordinates.lonString());
    }

};

class ColumnThumbnail : public TableViewColumn
{
public:

    explicit ColumnThumbnail(
            TableViewShared* const tableViewShared,
            const TableViewColumnConfiguration& pConfiguration
        )
      : TableViewColumn(tableViewShared, pConfiguration)
    {
    }
    virtual ~ColumnThumbnail() { }

    static TableViewColumnDescription getDescription()
    {
        return TableViewColumnDescription(QLatin1String("thumbnail"), QLatin1String("Thumbnail"));
    }
    virtual QString getTitle() { return i18n("Thumbnail"); }

    virtual QVariant data(const QModelIndex& sourceIndex, const int role)
    {
        Q_UNUSED(sourceIndex)
        Q_UNUSED(role)

        // we do not return any data, but paint(...) something
        return QVariant();
    }

    virtual bool paint(QPainter* const painter, const QStyleOptionViewItem& option, const QModelIndex& sourceIndex) const
    {
        /// @todo do we have to reset the column?
        const ImageInfo info = s->imageFilterModel->imageInfo(sourceIndex);
        if (!info.isNull())
        {
            QSize size(60, 60);
            const QString path = info.filePath();
            QPixmap thumbnail;

            /// @todo handle unavailable thumbnails -> emit itemChanged(...) later
            if (s->thumbnailLoadThread->find(path, thumbnail, qMax(size.width()+2, size.height()+2)))
            {
                /// @todo remove borders
//                 thumbnail = thumbnail.copy(1, 1, thumbnail.size().width()-2, thumbnail.size().height()-2)
                const QSize availableSize = option.rect.size();
                const QSize pixmapSize    = thumbnail.size().boundedTo(availableSize);
                QPoint startPoint((availableSize.width()-pixmapSize.width())/2,
                                (availableSize.height()-pixmapSize.height())/2);
                startPoint+=option.rect.topLeft();
                painter->drawPixmap(QRect(startPoint, pixmapSize), thumbnail, QRect(QPoint(0, 0), pixmapSize));

                return true;
            }
        }

        // we did not get to paint a thumbnail...
        return false;
    }

    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& sourceIndex) const
    {
        return QSize(60, 60);
    }


};

} /* namespace TableViewColumns */

} /* namespace Digikam */

#endif // TABLEVIEW_COLUMNS_H


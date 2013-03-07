/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-25
 * Description : Table view column helpers: Thumbnail column
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

#ifndef TABLEVIEW_COLUMN_THUMBNAIL_H
#define TABLEVIEW_COLUMN_THUMBNAIL_H

// Qt includes

#include <QObject>
#include <QPainter>
#include <QStringList>

// KDE includes

#include <kglobal.h>
#include <klocale.h>

// local includes

#include "tableview_columnfactory.h"
#include <libkgeomap/geocoordinates.h>
#include "thumbnailloadthread.h"

namespace Digikam
{

namespace TableViewColumns
{

class ColumnThumbnail : public TableViewColumn
{
    Q_OBJECT

public:

    explicit ColumnThumbnail(
            TableViewShared* const tableViewShared,
            const TableViewColumnConfiguration& pConfiguration,
            QObject* const parent = 0
        );

    virtual ~ColumnThumbnail();

    static TableViewColumnDescription getDescription();

    virtual ColumnFlags getColumnFlags() const;

    virtual QString getTitle();

    virtual QVariant data(const QModelIndex& sourceIndex, const int role);

    virtual bool paint(QPainter* const painter, const QStyleOptionViewItem& option, const QModelIndex& sourceIndex) const;

    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& sourceIndex) const;

    virtual void updateThumbnailSize();

private Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb);

private:

    int m_thumbnailSize;
};

} /* namespace TableViewColumns */

} /* namespace Digikam */

#endif // TABLEVIEW_COLUMN_THUMBNAIL_H


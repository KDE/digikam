/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-22
 * Description : A model for the view to display a list of images.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef GPSIMAGEITEMDELEGATE_H
#define GPSIMAGEITEMDELEGATE_H

// Qt includes

#include <QItemDelegate>

// Local includes

#include "gpsimagelist.h"

namespace Digikam
{

class GPSImageItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:

    explicit GPSImageItemDelegate(GPSImageList* const imageList, QObject* const parent = 0);
    virtual ~GPSImageItemDelegate();

    void setThumbnailSize(const int size);
    int  getThumbnailSize() const;

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& sortMappedindex) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& sortMappedindex) const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // GPSIMAGEITEMDELEGATE_H

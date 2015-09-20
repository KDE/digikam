/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-03-21
 * @brief  A model to hold information about images.
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "gpsimagesortproxymodel.h"

// KDE includes

#include <klinkitemselectionmodel.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class GPSImageSortProxyModel::Private
{
public:

    Private()
    {
        imageModel             = 0;
        sourceSelectionModel   = 0;
        linkItemSelectionModel = 0;
    }

    GPSImageModel*           imageModel;
    QItemSelectionModel*     sourceSelectionModel;
    KLinkItemSelectionModel* linkItemSelectionModel;
};

GPSImageSortProxyModel::GPSImageSortProxyModel(GPSImageModel* const imageModel, QItemSelectionModel* const sourceSelectionModel)
    : QSortFilterProxyModel(imageModel),
      d(new Private())
{
    d->imageModel             = imageModel;
    d->sourceSelectionModel   = sourceSelectionModel;
    setSourceModel(imageModel);
    d->linkItemSelectionModel = new KLinkItemSelectionModel(this, d->sourceSelectionModel);
}

GPSImageSortProxyModel::~GPSImageSortProxyModel()
{
    delete d;
}

bool GPSImageSortProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    if ((!left.isValid())||(!right.isValid()))
    {
//      qCDebug(DIGIKAM_GENERAL_LOG) << "INVALID INDICES" << left << right;
        return false;
    }

    const int column                     = left.column();
    const GPSImageItem* const itemLeft  = d->imageModel->itemFromIndex(left);
    const GPSImageItem* const itemRight = d->imageModel->itemFromIndex(right);

//  qCDebug(DIGIKAM_GENERAL_LOG) << itemLeft << itemRight << column << rowCount() << d->imageModel->rowCount();
    return itemLeft->lessThan(itemRight, column);
}

QItemSelectionModel* GPSImageSortProxyModel::mappedSelectionModel() const
{
    return d->linkItemSelectionModel;
}

} /* namespace Digikam */

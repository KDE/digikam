/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
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

#ifndef GPSIMAGEPROXYMODEL_H
#define GPSIMAGEPROXYMODEL_H

// Qt includes

#include <QItemSelectionModel>
#include <QSortFilterProxyModel>

// Local includes

#include "gpsimagemodel.h"

namespace Digikam
{

class GPSImageSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    GPSImageSortProxyModel(GPSImageModel* const imageModel, QItemSelectionModel* const sourceSelectionModel);
    ~GPSImageSortProxyModel();

    QItemSelectionModel* mappedSelectionModel() const;

protected:

    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;

private:

    class Private;
    Private* const d;
};

} /* namespace Digikam */

#endif /* GPSIMAGEPROXYMODEL_H */

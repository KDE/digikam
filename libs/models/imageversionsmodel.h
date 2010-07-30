/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-13
 * Description : Model for image versions
 *
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef IMAGEVERSIONSMODEL_H
#define IMAGEVERSIONSMODEL_H

// Qt includes

#include <QModelIndex>
#include <QPixmap>

// Local includes

#include "digikam_export.h"

namespace Digikam 
{

class WorkingWidget;

class DIGIKAM_EXPORT ImageVersionsModel : public QAbstractListModel
{
    Q_OBJECT

public:

    ImageVersionsModel(QObject* parent = 0);
    ~ImageVersionsModel();

    Qt::ItemFlags flags(const QModelIndex& index) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    void setupModelData(QList<QVariant>* data);
    void clearModelData();

public Q_SLOTS:

    void slotAnimationStep();

private:

    ///Complete paths with filenames
    QList<QVariant>* m_data;
};

} // namespace Digikam

#endif // IMAGEVERSIONSMODEL_H

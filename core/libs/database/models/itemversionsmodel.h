/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-13
 * Description : Model for item versions
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

#ifndef DIGIKAM_ITEM_VERSIONS_MODEL_H
#define DIGIKAM_ITEM_VERSIONS_MODEL_H

// Qt includes

#include <QModelIndex>
#include <QPixmap>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT ItemVersionsModel : public QAbstractListModel
{
    Q_OBJECT

public:

    explicit ItemVersionsModel(QObject* const parent = 0);
    ~ItemVersionsModel();

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant      data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int           rowCount(const QModelIndex& parent = QModelIndex()) const override;

    void setupModelData(QList<QPair<QString, int> >& data);
    void clearModelData();

    QString     currentSelectedImage() const;
    void        setCurrentSelectedImage(const QString& path);
    QModelIndex currentSelectedImageIndex() const;

    bool paintTree() const;
    int  listIndexOf(const QString& item) const;

public Q_SLOTS:

    void slotAnimationStep();
    void setPaintTree(bool paint);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_VERSIONS_MODEL_H

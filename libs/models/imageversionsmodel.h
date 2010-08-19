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
    void setupModelData(QList<QPair<QString, int> >& data);
    void clearModelData();
    QString currentSelectedImage() const;
    void setCurrentSelectedImage(const QString& path);
    QModelIndex currentSelectedImageIndex() const;
    bool paintTree() const;
    int listIndexOf(const QString& item) const;

public Q_SLOTS:

    void slotAnimationStep();
    void setPaintTree(bool paint);

private:

    ///Complete paths with filenames and tree level
    QList<QPair<QString, int> >* m_data;
    ///This is for delegate to paint it as selected
    QString                      m_currentSelectedImage;
    ///If true, the delegate will paint items as a tree
    ///if false, it will be painted as a list
    bool                         m_paintTree;
};

} // namespace Digikam

#endif // IMAGEVERSIONSMODEL_H

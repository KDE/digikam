/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-23
 * Description : model for view with used filters on currently loaded image
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

#ifndef IMAGEFILTERSHISTORY_H
#define IMAGEFILTERSHISTORY_H

// Qt includes

#include <QAbstractListModel>
#include <QList>

// KDE includes

#include <KUrl>

// Local includes

#include "dimagehistory.h"
#include "digikam_export.h"
#include "dimgfiltermanager.h"

namespace Digikam
{

class ImageFiltersHistoryTreeItem;

class DIGIKAM_EXPORT ImageFiltersHistoryModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    ImageFiltersHistoryModel(QObject* parent = 0, const KUrl& url = KUrl());
    ~ImageFiltersHistoryModel();

    Qt::ItemFlags flags(const QModelIndex& index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    void setUrl(const KUrl& url);
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& index) const;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex& parent);

public Q_SLOTS:

    void removeEntry(QModelIndex index);

private:

    void setupModelData(const QList<Digikam::DImageHistory::Entry>& entries, ImageFiltersHistoryTreeItem* parent);

private:

    ImageFiltersHistoryTreeItem *m_rootItem;
    QList<FilterAction>         *m_filterStack;
    DImgFilterManager           *m_filterManager;
};

} //namespace Digikam

#endif // IMAGEFILTERSHISTORY_H

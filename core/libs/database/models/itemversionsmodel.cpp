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

#include "itemversionsmodel.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "workingwidget.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemVersionsModel::Private
{
public:

    explicit Private()
    {
        data      = nullptr;
        paintTree = false;
    }

    /// Complete paths with filenames and tree level
    QList<QPair<QString, int> >* data;

    /// This is for delegate to paint it as selected
    QString                      currentSelectedImage;

    /** If true, the delegate will paint items as a tree
     *  if false, it will be painted as a list
     */
    bool                         paintTree;
};

ItemVersionsModel::ItemVersionsModel(QObject* const parent)
    : QAbstractListModel(parent),
      d(new Private)
{
    d->data = new QList<QPair<QString, int> >;
}

ItemVersionsModel::~ItemVersionsModel()
{
    //qDeleteAll(d->data);
    delete d;
}

Qt::ItemFlags ItemVersionsModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return nullptr;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ItemVersionsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole && !d->data->isEmpty())
    {
        return d->data->at(index.row()).first;
    }
    else if (role == Qt::UserRole && !d->data->isEmpty())
    {
        return d->data->at(index.row()).second;
    }
    else if (role == Qt::DisplayRole && d->data->isEmpty())
    {
        //TODO: make this text Italic
        return QVariant(QString(i18n("No image selected")));
    }

    return QVariant();
}

int ItemVersionsModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return d->data->count();
}

void ItemVersionsModel::setupModelData(QList<QPair<QString, int> >& data)
{
    beginResetModel();

    d->data->clear();

    if (!data.isEmpty())
    {
        d->data->append(data);
    }
    else
    {
        d->data->append(qMakePair(QString(i18n("This is the original image")), 0));
    }

    endResetModel();
}

void ItemVersionsModel::clearModelData()
{
    beginResetModel();

    if (!d->data->isEmpty())
    {
        d->data->clear();
    }

    endResetModel();
}

void ItemVersionsModel::slotAnimationStep()
{
    emit dataChanged(createIndex(0, 0), createIndex(rowCount()-1, 1));
}

QString ItemVersionsModel::currentSelectedImage() const
{
    return d->currentSelectedImage;
}

void ItemVersionsModel::setCurrentSelectedImage(const QString& path)
{
    d->currentSelectedImage = path;
}

QModelIndex ItemVersionsModel::currentSelectedImageIndex() const
{
    return index(listIndexOf(d->currentSelectedImage), 0);
}

bool ItemVersionsModel::paintTree() const
{
    return d->paintTree;
}

void ItemVersionsModel::setPaintTree(bool paint)
{
    d->paintTree = paint;
}

int ItemVersionsModel::listIndexOf(const QString& item) const
{
    for (int i = 0 ; i < d->data->size() ; ++i)
    {
        if (d->data->at(i).first == item)
        {
            return i;
        }
    }

    return -1;
}

} // namespace Digikam

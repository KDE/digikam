/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011-2012 by Lukasz Spas <lukasz dot spas at gmail dot com>
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

#include "TemplatesModel.h"

#include <QMessageBox>

using namespace PhotoLayoutsEditor;

TemplatesModel::TemplatesModel(QObject *parent) :
    QAbstractItemModel(parent)
{
}

QModelIndex TemplatesModel::index(int row, int column, const QModelIndex & parent) const
{
    if (!hasIndex(row, column, parent) || parent.isValid())
        return QModelIndex();

    return createIndex(row, column, templates[row]);
}

int TemplatesModel::columnCount(const QModelIndex & parent) const
{
    if (parent.isValid())
        return 0;
    return 1;
}

int TemplatesModel::rowCount(const QModelIndex & parent) const
{
    if (parent.isValid())
        return 0;
    return templates.count();
}

bool TemplatesModel::insertRows(int row, int count, const QModelIndex & parent)
{
    if (count < 0 || row > this->rowCount())
        return false;

    beginInsertRows(parent, row, row + count - 1);
    while (count--)
        templates.insert(row, 0);
    endInsertRows();
    return true;
}

bool TemplatesModel::removeRows(int row, int count, const QModelIndex & /*parent*/)
{
    while (count--)
        templates[row]->deleteLater();
    return true;
}

QVariant TemplatesModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TemplateItem* item = templates[index.row()];

    if (!item)
        return QVariant();

    switch (role)
    {
        case Qt::DisplayRole:
            return item->name();
            break;
        case Qt::DecorationRole:
            return item->icon();
            break;
        default:
            break;
    }

    return QVariant();
}

QModelIndex TemplatesModel::parent(const QModelIndex& /*child*/) const
{
    return QModelIndex();
}

void TemplatesModel::addTemplate(const QString & path, const QString & name)
{
    insertRows(rowCount(), 1);
    templates.last() = new TemplateItem(path, name);
}

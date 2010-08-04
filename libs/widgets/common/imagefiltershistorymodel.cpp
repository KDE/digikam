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

#include "imagefiltershistorymodel.moc"

// Qt includes

#include <QHashIterator>

// KDE includes

#include <KUrl>
#include <KDebug>

// Local includes

#include "dimgfiltermanager.h"
#include "dmetadata.h"
#include "imagefiltershistorytreeitem.h"

namespace Digikam
{

class ImageFiltersHistoryModel::ImageFiltersHistoryModelPriv
{
public:

    ImageFiltersHistoryModelPriv()
    {
        rootItem      = 0;
        filterStack   = 0;
        filterManager = 0;
        lastUrl       = 0;
    }

    ImageFiltersHistoryTreeItem* rootItem;
    QList<FilterAction>*         filterStack;
    DImgFilterManager*           filterManager;
    KUrl                         lastUrl;
};

ImageFiltersHistoryModel::ImageFiltersHistoryModel(QObject* parent, const KUrl& url)
                        : QAbstractItemModel(parent), d(new ImageFiltersHistoryModelPriv)
{
    d->filterStack = new QList<FilterAction>();
//    d->filterManager = new DImgFilterManager();

    if(!url.isEmpty())
    {
        kDebug() << "Creating model with url" << url.toLocalFile();
        d->rootItem = new ImageFiltersHistoryTreeItem(url.fileName());
        d->lastUrl = url;

        DMetadata metadata(url.toLocalFile());
        QString xml     = metadata.getImageHistory();
        DImageHistory h = DImageHistory::fromXml(xml);
        setupModelData(h.entries(), d->rootItem);
    }
    else
    {
        //kDebug() << "Creating empty model";
        d->rootItem = new ImageFiltersHistoryTreeItem("Generic");
    }
}

ImageFiltersHistoryModel::~ImageFiltersHistoryModel()
{
    delete d->rootItem;
    delete d;
}

void ImageFiltersHistoryModel::setUrl(const KUrl& url)
{
    if(!url.isEmpty())
    {
        //delete the current model data
        delete d->rootItem;

        d->rootItem = new ImageFiltersHistoryTreeItem(url.fileName());
        d->lastUrl = url;
        //kDebug() << "Updating model data with url" << rootData.first();
        DMetadata metadata(url.toLocalFile());
        setupModelData(DImageHistory::fromXml(metadata.getImageHistory()).entries(), d->rootItem);
    }
/*
    else
    {
        kDebug() << "Model not updated; url is" << url.pathOrUrl();
    }
*/
}

int ImageFiltersHistoryModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
/*
    if (parent.isValid())
        return static_cast<ImageFiltersHistoryTreeItem*>(parent.internalPointer())->columnCount();
    else
        return d->rootItem->columnCount();
*/
}

QVariant ImageFiltersHistoryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    ImageFiltersHistoryTreeItem* item;

    if(role == Qt::DecorationRole)
    {
        item = static_cast<ImageFiltersHistoryTreeItem*>(index.internalPointer());
        return item->data(1);
    }
    else if (role == Qt::DisplayRole)
    {
        item = static_cast<ImageFiltersHistoryTreeItem*>(index.internalPointer());
        return item->data(index.column());
    }

    return QVariant();
}

Qt::ItemFlags ImageFiltersHistoryModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ImageFiltersHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return d->rootItem->data(section);

    return QVariant();
}

QModelIndex ImageFiltersHistoryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ImageFiltersHistoryTreeItem* parentItem;

    if (!parent.isValid())
        parentItem = d->rootItem;
    else
        parentItem = static_cast<ImageFiltersHistoryTreeItem*>(parent.internalPointer());

    ImageFiltersHistoryTreeItem* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex ImageFiltersHistoryModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    ImageFiltersHistoryTreeItem* childItem  = static_cast<ImageFiltersHistoryTreeItem*>(index.internalPointer());
    ImageFiltersHistoryTreeItem* parentItem = childItem->parent();

    if (parentItem == d->rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int ImageFiltersHistoryModel::rowCount(const QModelIndex& parent) const
{
    ImageFiltersHistoryTreeItem* parentItem = 0;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = d->rootItem;
    else
        parentItem = static_cast<ImageFiltersHistoryTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void ImageFiltersHistoryModel::setupModelData(const QList<DImageHistory::Entry>& entries, 
                                              ImageFiltersHistoryTreeItem* parent)
{
    beginResetModel();
    if(!parent)
    {
        delete d->rootItem;
        d->rootItem = new ImageFiltersHistoryTreeItem(d->lastUrl.fileName());
        parent = d->rootItem;
    }

    kDebug() << "Initializing model data, got" << entries.count() << "entries";
    QList<ImageFiltersHistoryTreeItem*> parents;
    QList<ImageFiltersHistoryTreeItem*> filters;
    parents << parent;

    QList<QVariant> itemData;
    d->filterStack->clear();

    for(int i = 0; i < entries.count(); i++)
    {
        // the first entry, for the original, may not have an Action
        if (entries.at(i).action.isNull())
            continue;

        d->filterStack->append(entries.at(i).action);

        if (entries.at(i).action.displayableName().isEmpty() && entries.at(i).action.identifier().isEmpty())
        {
            itemData.append("Unknown filter");
        }
        else
        {
            itemData.append(entries.at(i).action.displayableName());
            itemData.append(entries.at(i).action.identifier());
        }
        kDebug() << "Adding an entry: " << itemData;
        parents.first()->appendChild(new ImageFiltersHistoryTreeItem(itemData, parents.first()));
        filters << parents.last()->child(parents.last()->childCount()-1);

/*        QHashIterator<QString, QVariant> iter(entries.at(i).action.parameters());
        while (iter.hasNext())
        {
            QList<QVariant> columnData;
            iter.next();
            columnData << iter.key() << iter.value();
            filters.last()->appendChild(new ImageFiltersHistoryTreeItem(columnData, filters.last()));
        }
*/

        itemData.clear();
    }

    endResetModel();
}

void ImageFiltersHistoryModel::removeEntry(QModelIndex index)
{
    removeRow(index.row(), index.parent());
}

bool ImageFiltersHistoryModel::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    if(!parent.isValid())
    {
        beginResetModel();
        d->rootItem->removeChild(row);
        d->filterStack->removeAt(row);
        endResetModel();
        //TODO: emit signal starting FilterManager
        return true;
    }
    return false;
}

void ImageFiltersHistoryModel::disableEntry(QModelIndex index, bool disable)
{
    if(index.isValid())
    {
        d->rootItem->child(index.row())->setDisabled(disable);
    }
}

} // namespace Digikam
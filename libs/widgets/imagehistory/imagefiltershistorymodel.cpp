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

#include "imagefiltershistorymodel.h"

// Qt includes

#include <QHashIterator>
#include <QPixmap>
#include <QUrl>
#include <QIcon>

// Local includes

#include "digikam_debug.h"
#include "dimgfiltermanager.h"
#include "dmetadata.h"
#include "imagefiltershistorytreeitem.h"

namespace Digikam
{

class ImageFiltersHistoryModel::Private
{
public:

    Private()
        : rootItem(0),
          disabledEntries(0)
    {
    }

    ImageFiltersHistoryTreeItem* rootItem;
    QList<FilterAction>          filterStack;
    QUrl                         lastUrl;

    int                          disabledEntries;
};

ImageFiltersHistoryModel::ImageFiltersHistoryModel(QObject* const parent, const QUrl& url)
    : QAbstractItemModel(parent),
      d(new Private)
{
    if (!url.isEmpty())
    {
        //qCDebug(DIGIKAM_WIDGETS_LOG) << "Creating model with url" << url.toLocalFile();
        d->rootItem = new ImageFiltersHistoryTreeItem(url.fileName());
        d->lastUrl  = url;

        DMetadata metadata(url.toLocalFile());
        QString xml     = metadata.getImageHistory();
        DImageHistory h = DImageHistory::fromXml(xml);
        setupModelData(h.entries(), d->rootItem);
    }
    else
    {
        //qCDebug(DIGIKAM_WIDGETS_LOG) << "Creating empty model";
        d->rootItem = new ImageFiltersHistoryTreeItem(QLatin1String("Generic"));
    }
}

ImageFiltersHistoryModel::~ImageFiltersHistoryModel()
{
    delete d->rootItem;
    delete d;
}

void ImageFiltersHistoryModel::setUrl(const QUrl& url)
{
    if (!url.isEmpty())
    {
        //delete the current model data
        delete d->rootItem;

        d->rootItem = new ImageFiltersHistoryTreeItem(url.fileName());
        d->lastUrl  = url;
        //qCDebug(DIGIKAM_WIDGETS_LOG) << "Updating model data with url" << rootData.first();
        DMetadata metadata(url.toLocalFile());
        setupModelData(DImageHistory::fromXml(metadata.getImageHistory()).entries(), d->rootItem);
    }
/*
    else
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "Model not updated; url is" << url.pathOrUrl();
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
    {
        return QVariant();
    }

    ImageFiltersHistoryTreeItem* item = 0;

    if (role == Qt::DecorationRole)
    {
        item = static_cast<ImageFiltersHistoryTreeItem*>(index.internalPointer());
        return item->data(1);
    }
    else if (role == Qt::DisplayRole)
    {
        item = static_cast<ImageFiltersHistoryTreeItem*>(index.internalPointer());
        return item->data(0);
    }

    return QVariant();
}

Qt::ItemFlags ImageFiltersHistoryModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return 0;
    }

    Qt::ItemFlags flags = Qt::ItemIsSelectable;

    if (!static_cast<ImageFiltersHistoryTreeItem*>(index.internalPointer())->isDisabled())
    {
        flags |= Qt::ItemIsEnabled;
    }

    return flags;
}

QVariant ImageFiltersHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return d->rootItem->data(section);
    }

    return QVariant();
}

QModelIndex ImageFiltersHistoryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    ImageFiltersHistoryTreeItem* parentItem = 0;

    if (!parent.isValid())
    {
        parentItem = d->rootItem;
    }
    else
    {
        parentItem = static_cast<ImageFiltersHistoryTreeItem*>(parent.internalPointer());
    }

    ImageFiltersHistoryTreeItem* const childItem = parentItem->child(row);

    if (childItem)
    {
        return createIndex(row, column, childItem);
    }
    else
    {
        return QModelIndex();
    }
}

QModelIndex ImageFiltersHistoryModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    ImageFiltersHistoryTreeItem* const childItem  = static_cast<ImageFiltersHistoryTreeItem*>(index.internalPointer());
    ImageFiltersHistoryTreeItem* const parentItem = childItem->parent();

    if (parentItem == d->rootItem)
    {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int ImageFiltersHistoryModel::rowCount(const QModelIndex& parent) const
{
    ImageFiltersHistoryTreeItem* parentItem = 0;

    if (parent.column() > 0)
    {
        return 0;
    }

    if (!parent.isValid())
    {
        parentItem = d->rootItem;
    }
    else
    {
        parentItem = static_cast<ImageFiltersHistoryTreeItem*>(parent.internalPointer());
    }

    return parentItem->childCount();
}

void ImageFiltersHistoryModel::setupModelData(const QList<DImageHistory::Entry>& entries, ImageFiltersHistoryTreeItem* parent)
{
    beginResetModel();

    if (!parent || entries.isEmpty())
    {
        delete d->rootItem;
        d->rootItem = new ImageFiltersHistoryTreeItem(d->lastUrl.fileName());
        parent      = d->rootItem;

        if (entries.isEmpty())
        {
            endResetModel();
            return;
        }
    }

    //qCDebug(DIGIKAM_WIDGETS_LOG) << "Initializing model data, got" << entries.count() << "entries";
    QList<ImageFiltersHistoryTreeItem*> parents;
    QList<ImageFiltersHistoryTreeItem*> filters;
    parents << parent;

    QList<QVariant> itemData;
    d->filterStack.clear();

    for (int i = 0; i < entries.count(); ++i)
    {
        // the first entry, for the original, may not have an Action
        if (entries.at(i).action.isNull())
        {
            continue;
        }

        d->filterStack.append(entries.at(i).action);

        itemData.append(DImgFilterManager::instance()->i18nDisplayableName(entries.at(i).action));

        QString iconName = DImgFilterManager::instance()->filterIcon(entries.at(i).action);
        QPixmap icon     = QIcon::fromTheme(iconName).pixmap(22);
        itemData.append(icon);

        //qCDebug(DIGIKAM_WIDGETS_LOG) << "Adding an entry: " << itemData;
        parents.first()->appendChild(new ImageFiltersHistoryTreeItem(itemData, parents.first()));
        filters << parents.last()->child(parents.last()->childCount()-1);

/*
        QHashIterator<QString, QVariant> iter(entries.at(i).action.parameters());
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

    d->disabledEntries = 0;

    endResetModel();
}

void ImageFiltersHistoryModel::removeEntry(const QModelIndex& index)
{
    removeRow(index.row(), index.parent());
}

bool ImageFiltersHistoryModel::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    if (!parent.isValid())
    {
        beginResetModel();
        d->rootItem->removeChild(row);
        d->filterStack.removeAt(row);
        endResetModel();
        //TODO: emit signal starting FilterManager
        return true;
    }

    return false;
}

void ImageFiltersHistoryModel::setEnabledEntries(int count)
{
    for (int i=0; i<d->rootItem->childCount(); ++i)
    {
        d->rootItem->child(i)->setDisabled(i >= count);
    }

    d->disabledEntries = qMax(rowCount() - count, 0);

    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
}

void ImageFiltersHistoryModel::disableEntries(int count)
{
    if (count > rowCount())
    {
        count = rowCount();
    }

    d->disabledEntries += count;

    while (count > 0)
    {
        d->rootItem->child(rowCount() - d->disabledEntries - 1 + count)->setDisabled(true);
        --count;
    }

    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
}

void ImageFiltersHistoryModel::enableEntries(int count)
{
    if (count > rowCount())
    {
        count = rowCount();
    }

    int tmp = count;

    while (count > 0)
    {
        d->rootItem->child(rowCount()-d->disabledEntries-1+count)->setDisabled(false);
        --count;
    }

    d->disabledEntries -= tmp;

    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
}

} // namespace Digikam

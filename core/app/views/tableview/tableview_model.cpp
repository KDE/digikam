/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-12
 * Description : Wrapper model for table view
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "tableview_model.h"

// C++ includes

#include <functional>
#include <valarray>

// Qt includes

#include <QTimer>

// Local includes

#include "digikam_debug.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "coredbchangesets.h"
#include "coredbfields.h"
#include "coredbwatch.h"
#include "imagefiltermodel.h"
#include "imagefiltersettings.h"
#include "imageinfo.h"
#include "tableview_columnfactory.h"
#include "tableview_selection_model_syncer.h"

#ifdef DIGIKAM_ENABLE_MODELTEST
#   include "../../../tests/modeltest/modeltest.h"
#endif // DIGIKAM_ENABLE_MODELTEST

#define ASSERT_MODEL(index, modelPointer) if (index.isValid()) { Q_ASSERT(index.model()==modelPointer); }

namespace Digikam
{

TableViewModel::Item::Item()
  : imageId(0),
    parent(0),
    children()
{
}

TableViewModel::Item::~Item()
{
    qDeleteAll(children);
}

void TableViewModel::Item::addChild(TableViewModel::Item* const newChild)
{
    newChild->parent = this;

    children << newChild;
}

void TableViewModel::Item::insertChild(const int pos, TableViewModel::Item* const newChild)
{
    newChild->parent = this;

    children.insert(pos, newChild);
}

void TableViewModel::Item::takeChild(TableViewModel::Item* const oldChild)
{
    children.removeOne(oldChild);
}

TableViewModel::Item* TableViewModel::Item::findChildWithImageId(const qlonglong searchImageId)
{
    if (imageId == searchImageId)
    {
        return this;
    }

    foreach(Item* const item, children)
    {
        Item* const iItem = item->findChildWithImageId(searchImageId);

        if (iItem)
        {
            return iItem;
        }
    }

    return 0;
}

// ----------------------------------------------------------------------------------------------

class TableViewModel::Private
{
public:

    Private()
      : columnObjects(),
        rootItem(0),
        imageFilterSettings(),
        sortColumn(0),
        sortOrder(Qt::AscendingOrder),
        sortRequired(false),
        groupingMode(GroupingShowSubItems),
        cachedImageInfos(),
        outdated(true)
    {
    }

    QList<TableViewColumn*>     columnObjects;
    TableViewModel::Item*       rootItem;
    ImageFilterSettings         imageFilterSettings;
    int                         sortColumn;
    Qt::SortOrder               sortOrder;
    bool                        sortRequired;
    GroupingMode                groupingMode;
    QHash<qlonglong, ImageInfo> cachedImageInfos;
    bool                        outdated;
};

TableViewModel::TableViewModel(TableViewShared* const sharedObject, QObject* parent)
    : QAbstractItemModel(parent),
      s(sharedObject),
      d(new Private())
{
    d->rootItem            = new Item();
    d->imageFilterSettings = s->imageFilterModel->imageFilterSettings();

    connect(s->imageModel, SIGNAL(modelAboutToBeReset()),
            this, SLOT(slotSourceModelAboutToBeReset()));

    connect(s->imageModel, SIGNAL(modelReset()),
            this, SLOT(slotSourceModelReset()));

    connect(s->imageModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(slotSourceRowsAboutToBeInserted(QModelIndex,int,int)));

    connect(s->imageModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotSourceRowsInserted(QModelIndex,int,int)));

    connect(s->imageModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(slotSourceRowsAboutToBeRemoved(QModelIndex,int,int)));

    connect(s->imageModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(slotSourceRowsRemoved(QModelIndex,int,int)));

    connect(s->imageModel, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(slotSourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));

    connect(s->imageModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(slotSourceRowsMoved(QModelIndex,int,int,QModelIndex,int)));

    connect(s->imageModel, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(slotSourceLayoutAboutToBeChanged()));

    connect(s->imageModel, SIGNAL(layoutChanged()),
            this, SLOT(slotSourceLayoutChanged()));

    connect(s->imageFilterModel, SIGNAL(filterSettingsChanged(ImageFilterSettings)),
            this, SLOT(slotFilterSettingsChanged(ImageFilterSettings)));

    // We do not connect to ImageFilterModel::dataChanged, because we monitor changes directly from the database.

    connect(CoreDbAccess::databaseWatch(), SIGNAL(imageChange(ImageChangeset)),
            this, SLOT(slotDatabaseImageChanged(ImageChangeset)), Qt::QueuedConnection);

#ifdef DIGIKAM_ENABLE_MODELTEST
    new ModelTest(this, this);
#endif // DIGIKAM_ENABLE_MODELTEST

    // We only have to trigger population of the model if data is in the source model,
    // otherwise the source model will tell us about any new data.
    const int itemsInImageModel = s->imageModel->rowCount();

    if (itemsInImageModel > 0)
    {
        // populate the model once later, not now
        QTimer::singleShot(0, this, SLOT(slotPopulateModelWithNotifications()));
    }
}

TableViewModel::~TableViewModel()
{
    delete d->rootItem;
}

int TableViewModel::columnCount(const QModelIndex& i) const
{
    if (i.column() > 0)
    {
        return 0;
    }

    return d->columnObjects.count();
}

QModelIndex TableViewModel::toImageFilterModelIndex(const QModelIndex& i) const
{
    Item* const item = itemFromIndex(i);

    if (!item)
    {
        return QModelIndex();
    }

    return s->imageFilterModel->indexForImageId(item->imageId);
}

QModelIndex TableViewModel::toImageModelIndex(const QModelIndex& i) const
{
    Item* const item = itemFromIndex(i);

    if (!item)
    {
        return QModelIndex();
    }

    return s->imageModel->indexForImageId(item->imageId);
}

QVariant TableViewModel::data(const QModelIndex& i, int role) const
{
    Item* const item = itemFromIndex(i);

    if (!item)
    {
        return QVariant();
    }

    const int columnNumber          = i.column();
    TableViewColumn* const myColumn = d->columnObjects.at(columnNumber);

    return myColumn->data(item, role);
}

QModelIndex TableViewModel::index(int row, int column, const QModelIndex& parent) const
{
    Item* parentItem = d->rootItem;

    if (parent.isValid())
    {
        if (parent.column() > 0)
        {
            // only column 0 can have children, the other columns can not
            return QModelIndex();
        }

        parentItem = itemFromIndex(parent);
    }

    // test for valid row/column values
    if ((row < 0) || (column < 0)            ||
        (column >= d->columnObjects.count()) ||
        (row >= parentItem->children.count()))
    {
        return QModelIndex();
    }

    Item* const itemPointer = parentItem->children.at(row);

    return createIndex(row, column, itemPointer);
}

QModelIndex TableViewModel::parent(const QModelIndex& childIndex) const
{
    if (!childIndex.isValid())
    {
        return QModelIndex();
    }

    Item* const childItem  = itemFromIndex(childIndex);
    Item* const parentItem = childItem->parent;

    if (parentItem == d->rootItem)
    {
        return QModelIndex();
    }

    Item* const grandParentItem = parentItem->parent;
    const int rowIndex          = grandParentItem->children.indexOf(parentItem);

    /// @todo What should be the column number?
    return createIndex(rowIndex, 0, parentItem);
}

int TableViewModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
    {
        return 0;
    }

    Item* parentItem = d->rootItem;

    if (parent.isValid())
    {
        parentItem = itemFromIndex(parent);
    }

    return parentItem->children.count();
}

QVariant TableViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // test for valid input ranges
    if ((section < 0) || (section >= d->columnObjects.count()))
    {
        return QVariant();
    }

    if ((orientation != Qt::Horizontal) || (role != Qt::DisplayRole))
    {
        return QVariant();
    }

    TableViewColumn* const myColumn = d->columnObjects.at(section);

    return myColumn->getTitle();
}

void TableViewModel::addColumnAt(const TableViewColumnDescription& description, const int targetColumn)
{
    /// @todo take additional configuration data of the column into account
    TableViewColumnConfiguration newConfiguration = description.toConfiguration();

    addColumnAt(newConfiguration, targetColumn);
}

void TableViewModel::addColumnAt(const TableViewColumnConfiguration& configuration, const int targetColumn)
{
    TableViewColumn* const newColumn = s->columnFactory->getColumn(configuration);

    if (!newColumn)
    {
        return;
    }

    int newColumnIndex = targetColumn;

    if (targetColumn < 0)
    {
        // a negative column index means "append after last column"
        newColumnIndex = d->columnObjects.count();
    }

    beginInsertColumns(QModelIndex(), newColumnIndex, newColumnIndex);

    if (newColumnIndex >= d->columnObjects.count())
    {
        d->columnObjects.append(newColumn);
    }
    else
    {
        d->columnObjects.insert(newColumnIndex, newColumn);
    }

    endInsertColumns();

    connect(newColumn, SIGNAL(signalDataChanged(qlonglong)),
            this, SLOT(slotColumnDataChanged(qlonglong)));

    connect(newColumn, SIGNAL(signalAllDataChanged()),
            this, SLOT(slotColumnAllDataChanged()));
}

void TableViewModel::slotColumnDataChanged(const qlonglong imageId)
{
    TableViewColumn* const senderColumn = qobject_cast<TableViewColumn*>(sender());

    /// @todo find a faster way to find the column number
    const int iColumn                   = d->columnObjects.indexOf(senderColumn);

    if (iColumn < 0)
    {
        return;
    }

    const QModelIndex changedIndex      = indexFromImageId(imageId, iColumn);
    emit(dataChanged(changedIndex, changedIndex));
}

void TableViewModel::slotColumnAllDataChanged()
{
    TableViewColumn* const senderColumn = qobject_cast<TableViewColumn*>(sender());

    /// @todo find a faster way to find the column number
    const int iColumn                   = d->columnObjects.indexOf(senderColumn);

    if (iColumn < 0)
    {
        return;
    }

    const QModelIndex changedIndexTopLeft     = index(0, iColumn, QModelIndex());
    const QModelIndex changedIndexBottomRight = index(rowCount(QModelIndex())-1, iColumn, QModelIndex());

    emit(dataChanged(changedIndexTopLeft, changedIndexBottomRight));
}

void TableViewModel::removeColumnAt(const int columnIndex)
{
    beginRemoveColumns(QModelIndex(), columnIndex, columnIndex);
    TableViewColumn* const removedColumn = d->columnObjects.takeAt(columnIndex);
    endRemoveColumns();

    delete removedColumn;
}

TableViewColumn* TableViewModel::getColumnObject(const int columnIndex)
{
    return d->columnObjects.at(columnIndex);
}

TableViewColumnProfile TableViewModel::getColumnProfile() const
{
    TableViewColumnProfile profile;

    for (int i = 0 ; i < d->columnObjects.count() ; ++i)
    {
        TableViewColumnConfiguration ic = d->columnObjects.at(i)->getConfiguration();
        profile.columnConfigurationList << ic;
    }

    return profile;
}

void TableViewModel::loadColumnProfile(const TableViewColumnProfile& columnProfile)
{
    while (!d->columnObjects.isEmpty())
    {
        removeColumnAt(0);
    }

    /// @todo disable updating of the model while this happens
    for (int i = 0 ; i < columnProfile.columnConfigurationList.count() ; ++i)
    {
        addColumnAt(columnProfile.columnConfigurationList.at(i), -1);
    }
}

void TableViewModel::slotSourceModelAboutToBeReset()
{
    if (!s->isActive)
    {
        slotClearModel(true);
        return;
    }

    // the source model is about to be reset. Propagate that change:
    beginResetModel();
}

void TableViewModel::slotSourceModelReset()
{
    if (!s->isActive)
    {
        return;
    }

    // the source model is done resetting.
    slotPopulateModel(false);
    endResetModel();
}

void TableViewModel::slotSourceRowsAboutToBeInserted(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
}

void TableViewModel::slotSourceRowsInserted(const QModelIndex& parent, int start, int end)
{
    if (!s->isActive)
    {
        slotClearModel(true);
        return;
    }

    for (int i = start ; i <= end ; ++i)
    {
        const QModelIndex sourceIndex = s->imageModel->index(i, 0, parent);

        addSourceModelIndex(sourceIndex, true);
    }
}

void TableViewModel::slotSourceRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    if (!s->isActive)
    {
        slotClearModel(true);
        return;
    }

    for (int i = start ; i <= end ; ++i)
    {
        const QModelIndex imageModelIndex = s->imageModel->index(i, 0, parent);
        const qlonglong imageId           = s->imageModel->imageId(imageModelIndex);
        d->cachedImageInfos.remove(imageId);

        const QModelIndex tableViewIndex  = indexFromImageId(imageId, 0);

        if (!tableViewIndex.isValid())
        {
            continue;
        }

        Item* const item                  = itemFromIndex(tableViewIndex);

        if (!item)
        {
            continue;
        }

        beginRemoveRows(tableViewIndex.parent(), tableViewIndex.row(), tableViewIndex.row());
        item->parent->takeChild(item);
        // delete image info of children from cache
        QList<Item*> itemsToRemoveFromCache = item->children;

        while (!itemsToRemoveFromCache.isEmpty())
        {
            Item* const itemToRemove = itemsToRemoveFromCache.takeFirst();
            itemsToRemoveFromCache << itemToRemove->children;

            d->cachedImageInfos.remove(itemToRemove->imageId);

            // child items will be deleted when item is deleted
        }

        delete item;
        endRemoveRows();
    }
}

void TableViewModel::slotSourceRowsRemoved(const QModelIndex& parent, int start, int end)
{
    /// @todo Do we need to do anything here?

    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
}

void TableViewModel::slotSourceRowsAboutToBeMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd,
                                                  const QModelIndex& destinationParent, int destinationRow)
{
//     beginMoveRows(sourceParent, sourceStart, sourceEnd, destinationParent, destinationRow);

    /// @todo For our items, moving stuff around does not matter --> remove this slot
    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceStart)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destinationParent)
    Q_UNUSED(destinationRow)
}

void TableViewModel::slotSourceRowsMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd,
                                         const QModelIndex& destinationParent, int destinationRow)
{
    /// @todo For our items, moving stuff around does not matter --> remove this slot

    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceStart)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destinationParent)
    Q_UNUSED(destinationRow)

//     endMoveRows();
}

void TableViewModel::slotSourceLayoutAboutToBeChanged()
{
    if (!s->isActive)
    {
        slotClearModel(true);
        return;
    }

    /// @todo Emitting layoutAboutToBeChanged and layoutChanged is tricky,
    ///       because we do not know what will change.
    ///       It looks like ImageFilterModel emits layoutAboutToBeChanged and layoutChanged
    ///       even when the resulting dataset will be empty, and ModelTest does not like that.
    ///       For now, the easiest workaround is resetting the model
//     emit(layoutAboutToBeChanged());
    beginResetModel();
}

void TableViewModel::slotSourceLayoutChanged()
{
    if (!s->isActive)
    {
        return;
    }

    /// @todo See note in TableViewModel#slotSourceLayoutAboutToBeChanged

    slotPopulateModel(false);

    endResetModel();
}

void TableViewModel::slotDatabaseImageChanged(const ImageChangeset& imageChangeset)
{
    if (!s->isActive)
    {
        slotClearModel(true);
        return;
    }

//     const DatabaseFields::Set changes = imageChangeset.changes();

    /// @todo Decide which changes are relevant here or
    ///       let the TableViewColumn object decide which are relevant
    /// @todo Re-population of the model is also triggered, thus making this
    ///       check irrelevant. Needs to be fixed.
    /// @todo If the user has never set which column should define the sorting,
    ///       the sortColumn is invalid. We should set a useful default.
    bool needToResort = false;

    if ((d->sortColumn >= 0) && (d->sortColumn < d->columnObjects.count()))
    {
        TableViewColumn* const sortColumnObject = d->columnObjects.at(d->sortColumn);
        needToResort                            = sortColumnObject->columnAffectedByChangeset(imageChangeset);
    }

    foreach(const qlonglong& id, imageChangeset.ids())
    {
        // first clear the item's cached values
        /// @todo Clear only the fields which were changed
        Item* const item = itemFromImageId(id);

        if (!item)
        {
            // Item is not in this model. If it is in the ImageModel,
            // it has been filtered out and we have to re-check the filtering.
            const QModelIndex& imageModelIndex = s->imageModel->indexForImageId(id);

            if (!imageModelIndex.isValid())
            {
                continue;
            }

            const ImageInfo imageInfo          = s->imageModel->imageInfo(imageModelIndex);

            if (d->imageFilterSettings.matches(imageInfo))
            {
                // need to add the item
                addSourceModelIndex(imageModelIndex, true);
            }

            continue;
        }

        // remove cached info and re-insert it
        if (d->cachedImageInfos.contains(item->imageId))
        {
            const ImageInfo itemInfo(item->imageId);
            d->cachedImageInfos.remove(item->imageId);
            d->cachedImageInfos.insert(item->imageId, itemInfo);
        }

        // Re-check filtering for this item.
        const QModelIndex changedIndexTopLeft = indexFromImageId(id, 0);

        if (!changedIndexTopLeft.isValid())
        {
            continue;
        }

        const ImageInfo myImageInfo = imageInfo(changedIndexTopLeft);

        if (!d->imageFilterSettings.matches(myImageInfo))
        {
            // Filter does not match, remove the item.
            beginRemoveRows(changedIndexTopLeft.parent(), changedIndexTopLeft.row(), changedIndexTopLeft.row());
            item->parent->takeChild(item);
            delete item;
            endRemoveRows();

            continue;
        }

        /// @todo Re-check grouping for this item

        // only update now if we do not resort later anyway
        if (!needToResort)
        {
            const QModelIndex changedIndexBottomRight = index(changedIndexTopLeft.row(),
                                                              columnCount(changedIndexTopLeft.parent())-1,
                                                              changedIndexTopLeft.parent());

            if (changedIndexBottomRight.isValid())
            {
                emit(dataChanged(changedIndexTopLeft, changedIndexBottomRight));
            }
        }
    }

    if (needToResort)
    {
        scheduleResort();
    }
}

Qt::ItemFlags TableViewModel::flags(const QModelIndex& index) const
{
    const Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    /// @todo Handle read-only files etc. which can not be moved
    if (index.isValid())
    {
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    }

    return Qt::ItemIsDropEnabled | defaultFlags;
}

QList<TableViewColumn*> TableViewModel::getColumnObjects()
{
    return d->columnObjects;
}

void TableViewModel::slotPopulateModelWithNotifications()
{
    slotPopulateModel(true);
}

void TableViewModel::slotClearModel(const bool sendNotifications)
{
    if (d->outdated)
    {
        return;
    }

    d->outdated = true;

    if (sendNotifications)
    {
        beginResetModel();
    }

    if (d->rootItem)
    {
        delete d->rootItem;
    }

    d->rootItem = new Item();
    d->cachedImageInfos.clear();

    if (sendNotifications)
    {
        endResetModel();
    }
}

void TableViewModel::slotPopulateModel(const bool sendNotifications)
{
    if (!s->isActive)
    {
        slotClearModel(sendNotifications);
        return;
    }

    if (sendNotifications)
    {
        beginResetModel();
    }

    if (d->rootItem)
    {
        delete d->rootItem;
    }

    d->rootItem     = new Item();
    d->cachedImageInfos.clear();
    d->outdated     = false;
    d->sortRequired = false;

    const int sourceRowCount = s->imageModel->rowCount(QModelIndex());

    for (int i = 0 ; i < sourceRowCount ; ++i)
    {
        const QModelIndex sourceModelIndex = s->imageModel->index(i, 0);
        // do not send notifications in addSourceModelIndex, because this function here
        // already started a model reset
        addSourceModelIndex(sourceModelIndex, false);
    }

    if (sendNotifications)
    {
        endResetModel();
    }
}

TableViewModel::Item* TableViewModel::createItemFromSourceIndex(const QModelIndex& imageModelIndex)
{
    ASSERT_MODEL(imageModelIndex, s->imageModel);

    Item* const item = new Item();
    item->imageId    = s->imageModel->imageId(imageModelIndex);

    return item;
}

void TableViewModel::addSourceModelIndex(const QModelIndex& imageModelIndex, const bool sendNotifications)
{
    ASSERT_MODEL(imageModelIndex, s->imageModel);

    const ImageInfo imageInfo = s->imageModel->imageInfo(imageModelIndex);
    const bool passedFilter   = d->imageFilterSettings.matches(imageInfo);

    if (!passedFilter)
    {
        return;
    }

    /// @todo Implement Grouping and sorting
    Item* const parentItem = d->rootItem;

    if (imageInfo.isGrouped())
    {
        switch (d->groupingMode)
        {
        case GroupingHideGrouped:
            // we do not show grouped items at all
            return;

        case GroupingIgnoreGrouping:
            // nothing to do, we just add it to the root item
            break;

        case GroupingShowSubItems:
            // we do not add this subitem, because it has been automatically added to the group leader
            return;
        }
    }

    Item* item      = createItemFromSourceIndex(imageModelIndex);

    // Normally we do the sorting of items here on insertion.
    // However, if the sorting is currently outdated, we just
    // append the items because the model will be resorted later.
    int newRowIndex = parentItem->children.count();

    if (!d->sortRequired)
    {
        newRowIndex = findChildSortedPosition(parentItem, item);
    }

    if (sendNotifications)
    {
        const QModelIndex parentIndex = itemIndex(parentItem);
        beginInsertRows(parentIndex, newRowIndex, newRowIndex);
    }

    parentItem->insertChild(newRowIndex, item);

    if (sendNotifications)
    {
        endInsertRows();
    }

    if ((d->groupingMode == GroupingShowSubItems) && imageInfo.hasGroupedImages())
    {
        // the item was a group leader, add its subitems
        const QList<ImageInfo> groupedImages = imageInfo.groupedImages();

        if (sendNotifications)
        {
            const QModelIndex groupLeaderIndex = itemIndex(item);
            beginInsertRows(groupLeaderIndex, 0, groupedImages.count()-1);
        }

        foreach(const ImageInfo& groupedInfo, groupedImages)
        {
            d->cachedImageInfos.insert(groupedInfo.id(), groupedInfo);

            /// @todo Grouped items are currently not filtered. Should they?
            Item* const groupedItem = new Item();
            groupedItem->imageId    = groupedInfo.id();

            // Normally we do the sorting of items here on insertion.
            // However, if the sorting is currently outdated, we just
            // append the items because the model will be resorted later.
            int newRowIndex = item->children.count();

            if (!d->sortRequired)
            {
                newRowIndex = findChildSortedPosition(item, groupedItem);
            }

            item->insertChild(newRowIndex, groupedItem);
        }

        if (sendNotifications)
        {
            endInsertRows();
        }
    }
}

TableViewModel::Item* TableViewModel::itemFromImageId(const qlonglong imageId) const
{
    return d->rootItem->findChildWithImageId(imageId);
}

TableViewModel::Item* TableViewModel::itemFromIndex(const QModelIndex& i) const
{
    if (!i.isValid())
    {
        return 0;
    }

    Q_ASSERT(i.model() == this);

    Item* const item = static_cast<Item*>(i.internalPointer());

    return item;
}

QModelIndex TableViewModel::fromImageFilterModelIndex(const QModelIndex& imageFilterModelIndex)
{
    ASSERT_MODEL(imageFilterModelIndex, s->imageFilterModel);

    const qlonglong imageId = s->imageFilterModel->imageId(imageFilterModelIndex);

    if (!imageId)
    {
        return QModelIndex();
    }

    return indexFromImageId(imageId, 0);
}

QModelIndex TableViewModel::fromImageModelIndex(const QModelIndex& imageModelIndex)
{
    ASSERT_MODEL(imageModelIndex, s->imageModel);

    const qlonglong imageId = s->imageModel->imageId(imageModelIndex);

    if (!imageId)
    {
        return QModelIndex();
    }

    return indexFromImageId(imageId, 0);
}

ImageInfo TableViewModel::infoFromItem(TableViewModel::Item* const item) const
{
    /// @todo Is there a way to do it without first looking up the index in the ImageModel?
    const QModelIndex imageModelIndex = s->imageModel->indexForImageId(item->imageId);

    if (!imageModelIndex.isValid())
    {
        const ImageInfo fromCache = d->cachedImageInfos.value(item->imageId);

        return fromCache;
    }

    const ImageInfo info = s->imageModel->imageInfo(imageModelIndex);

    return info;
}

ImageInfoList TableViewModel::infosFromItems(QList<TableViewModel::Item*> const items) const
{
    ImageInfoList infos;

    foreach(TableViewModel::Item* const item, items)
    {
        infos << infoFromItem(item);
    }

    return infos;
}

TableViewModel::DatabaseFieldsHashRaw TableViewModel::itemDatabaseFieldsRaw(TableViewModel::Item* const item, const DatabaseFields::Set requestedSet)
{
    const ImageInfo itemImageInfo = infoFromItem(item);

    return itemImageInfo.getDatabaseFieldsRaw(requestedSet);
}

QVariant TableViewModel::itemDatabaseFieldRaw(TableViewModel::Item* const item, const DatabaseFields::Set requestedField)
{
    const TableViewModel::DatabaseFieldsHashRaw rawHash = itemDatabaseFieldsRaw(item, requestedField);

    if (requestedField.hasFieldsFromImageMetadata())
    {
        const DatabaseFields::ImageMetadata requestedFieldFlag = requestedField;
        const QVariant value                                   = rawHash.value(requestedFieldFlag);

        return value;
    }

    if (requestedField.hasFieldsFromVideoMetadata())
    {
        const DatabaseFields::VideoMetadata requestedFieldFlag = requestedField;
        const QVariant value                                   = rawHash.value(requestedFieldFlag);

        return value;
    }

    return QVariant();
}

QModelIndex TableViewModel::indexFromImageId(const qlonglong imageId, const int columnIndex) const
{
    Item* const item = itemFromImageId(imageId);

    if (!item)
    {
        return QModelIndex();
    }

    Item* const parentItem = item->parent;

    /// @todo This is a waste of time because itemFromImageId already did this search.
    ///       We should modify it to also give the row index.
    const int rowIndex = parentItem->children.indexOf(item);

    return createIndex(rowIndex, columnIndex, item);
}

QList<qlonglong> TableViewModel::imageIds(const QModelIndexList& indexList) const
{
    QList<qlonglong> idList;

    foreach(const QModelIndex& index, indexList)
    {
        ASSERT_MODEL(index, this);

        if (index.column() > 0)
        {
            continue;
        }

        const Item* const item = itemFromIndex(index);

        if (!item)
        {
            continue;
        }

        idList << item->imageId;
    }

    return idList;
}

QList<ImageInfo> TableViewModel::imageInfos(const QModelIndexList& indexList) const
{
    QList<ImageInfo> infoList;

    foreach(const QModelIndex& index, indexList)
    {
        ASSERT_MODEL(index, this);

        if (index.column() > 0)
        {
            continue;
        }

        Item* const item = itemFromIndex(index);

        if (!item)
        {
            continue;
        }

        infoList << infoFromItem(item);
    }

    return infoList;
}

ImageInfo TableViewModel::imageInfo(const QModelIndex& index) const
{
    ASSERT_MODEL(index, this);

    Item* const item = itemFromIndex(index);

    if (!item)
    {
        return ImageInfo();
    }

    return infoFromItem(item);
}

void TableViewModel::slotFilterSettingsChanged(const ImageFilterSettings& settings)
{
    d->imageFilterSettings = settings;

    slotPopulateModel(true);
}

class TableViewModel::LessThan
{
public:

    explicit LessThan(TableViewModel* const model)
      : m(model)
    {
    }

    bool operator()(const TableViewModel::Item* const itemA, const TableViewModel::Item* const itemB)
    {
        const bool compareResult = m->lessThan(const_cast<Item*>(itemA), const_cast<Item*>(itemB));

        if (m->d->sortOrder == Qt::DescendingOrder)
        {
            return !compareResult;
        }

        return compareResult;
    }

public:

    TableViewModel* m;
};

QList<TableViewModel::Item*> TableViewModel::sortItems(const QList<TableViewModel::Item*> itemList)
{
    QList<Item*> sortedList = itemList;

    std::sort(sortedList.begin(),
              sortedList.end(),
              LessThan(this));

    return sortedList;
}

void TableViewModel::sort(int column, Qt::SortOrder order)
{
    d->sortColumn = column;
    d->sortOrder  = order;

    /// @todo re-sort items
    QList<Item*> itemsRequiringSorting;
    itemsRequiringSorting << d->rootItem;

    beginResetModel();

    while (!itemsRequiringSorting.isEmpty())
    {
        Item* const itemToSort = itemsRequiringSorting.takeFirst();

        foreach(Item* const itemToCheck, itemToSort->children)
        {
            if (!itemToCheck->children.isEmpty())
            {
                itemsRequiringSorting << itemToCheck;
            }
        }

        itemToSort->children = sortItems(itemToSort->children);
    }

    endResetModel();
}

bool TableViewModel::lessThan(TableViewModel::Item* const itemA, TableViewModel::Item* const itemB)
{
    if ((d->sortColumn < 0) || (d->sortColumn >= d->columnObjects.count()))
    {
        return itemA->imageId < itemB->imageId;
    }

    const TableViewColumn* columnObject = s->tableViewModel->getColumnObject(d->sortColumn);

    if (!columnObject->getColumnFlags().testFlag(TableViewColumn::ColumnCustomSorting))
    {
        const QString stringA = columnObject->data(itemA, Qt::DisplayRole).toString();
        const QString stringB = columnObject->data(itemB, Qt::DisplayRole).toString();

        if ((stringA == stringB) || (stringA.isEmpty() && stringB.isEmpty()))
        {
            return itemA->imageId < itemB->imageId;
        }

        return stringA < stringB;
    }

    TableViewColumn::ColumnCompareResult cmpResult = columnObject->compare(itemA, itemB);

    if (cmpResult == TableViewColumn::CmpEqual)
    {
        // compared items are equal, use the image id to enforce a repeatable sorting
        const qlonglong imageIdA = itemA->imageId;
        const qlonglong imageIdB = itemB->imageId;

        return imageIdA < imageIdB;
    }

    return cmpResult == TableViewColumn::CmpALessB;
}

QMimeData* TableViewModel::mimeData(const QModelIndexList& indexes) const
{
    // we pack the mime data via ImageModel's drag-drop handler
    AbstractItemDragDropHandler* const ddHandler = s->imageModel->dragDropHandler();

    QModelIndexList imageModelIndexList;

    foreach(const QModelIndex& i, indexes)
    {
        if (i.column() > 0)
        {
            continue;
        }

        const QModelIndex imageModelIndex = toImageModelIndex(i);

        if (imageModelIndex.isValid())
        {
            imageModelIndexList << imageModelIndex;
        }
    }

    QMimeData* const imageModelMimeData   = ddHandler->createMimeData(imageModelIndexList);

    return imageModelMimeData;
}

Qt::DropActions TableViewModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList TableViewModel::mimeTypes() const
{
    AbstractItemDragDropHandler* const ddHandler = s->imageModel->dragDropHandler();

    if (ddHandler)
    {
        return ddHandler->mimeTypes();
    }

    return QStringList();
}

bool TableViewModel::dropMimeData(const QMimeData* data,
                                  Qt::DropAction action,
                                  int row, int column,
                                  const QModelIndex& parent)
{
    Q_UNUSED(data)
    Q_UNUSED(action)
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)

    // the drop is handled by the drag-drop handler, therefore we return false here
    return false;
}

void TableViewModel::slotResortModel()
{
    if (!d->sortRequired)
    {
        return;
    }

    beginResetModel();
    sort(d->sortColumn, d->sortOrder);
    endResetModel();

    d->sortRequired = false;
}

void TableViewModel::scheduleResort()
{
    if (d->sortRequired)
    {
        return;
    }

    d->sortRequired = true;

    QTimer::singleShot(100, this, SLOT(slotResortModel()));
}

QModelIndex TableViewModel::itemIndex(TableViewModel::Item* const item) const
{
    if ((!item) || (item==d->rootItem))
    {
        return QModelIndex();
    }

    const int rowIndex = item->parent->children.indexOf(item);

    return createIndex(rowIndex, 0, item);
}

bool TableViewModel::hasChildren(const QModelIndex& parent) const
{
    Item* parentItem = d->rootItem;

    if (parent.isValid())
    {
        if (parent.column() > 0)
        {
            // only column 0 can have children
            return false;
        }

        parentItem = itemFromIndex(parent);
    }

    return !parentItem->children.isEmpty();
}

qlonglong TableViewModel::imageId(const QModelIndex& anIndex) const
{
    const Item* const anItem = itemFromIndex(anIndex);

    if (!anItem)
    {
        return -1;
    }

    return anItem->imageId;
}

QList<ImageInfo> TableViewModel::allImageInfo() const
{
    return infosFromItems(d->rootItem->children);
}

TableViewModel::GroupingMode TableViewModel::groupingMode() const
{
    return d->groupingMode;
}

void TableViewModel::setGroupingMode(const TableViewModel::GroupingMode newGroupingMode)
{
    if (d->groupingMode != newGroupingMode)
    {
        d->groupingMode = newGroupingMode;
        QTimer::singleShot(100, this, SLOT(slotPopulateModelWithNotifications()));

        emit(signalGroupingModeChanged());
    }
}

QModelIndex TableViewModel::deepRowIndex(const int rowNumber) const
{
    int targetRowNumber = rowNumber;

    if (rowNumber < 0)
    {
        targetRowNumber += deepRowCount();
    }

    QModelIndex cIndex = index(0, 0);

    for (int i = 0 ; i < targetRowNumber ; ++i)
    {
        if (hasChildren(cIndex))
        {
            cIndex = cIndex.child(0, 0);
        }
        else
        {
            QModelIndex candidateIndex = cIndex.sibling(cIndex.row() + 1, 0);

            if (!candidateIndex.isValid())
            {
                QModelIndex parentIndex = cIndex.parent();

                if (!parentIndex.isValid())
                {
                    return QModelIndex();
                }

                candidateIndex = parentIndex.sibling(parentIndex.row() + 1, 0);
            }

            cIndex = candidateIndex;
        }
    }

    return cIndex;
}

int TableViewModel::indexToDeepRowNumber(const QModelIndex& rowIndex) const
{
    const QModelIndex column0Index = toCol0(rowIndex);
    int deepRowNumber              = 0;
    QModelIndex cIndex             = index(0, 0);

    while (cIndex.isValid())
    {
        if (cIndex == column0Index)
        {
            break;
        }

        ++deepRowNumber;

        if (hasChildren(cIndex))
        {
            cIndex = cIndex.child(0, 0);
        }
        else
        {
            QModelIndex candidateIndex = cIndex.sibling(cIndex.row() + 1, 0);

            if (!candidateIndex.isValid())
            {
                QModelIndex parentIndex = cIndex.parent();

                if (!parentIndex.isValid())
                {
                    break;
                }

                candidateIndex          = parentIndex.sibling(parentIndex.row() + 1, 0);
            }

            cIndex = candidateIndex;
        }
    }

    if (!cIndex.isValid())
    {
        return -1;
    }

    return deepRowNumber;
}

int TableViewModel::deepRowCount() const
{
    int deepRowNumber  = 0;
    QModelIndex cIndex = index(0, 0);

    while (cIndex.isValid())
    {
        ++deepRowNumber;

        if (hasChildren(cIndex))
        {
            cIndex = cIndex.child(0, 0);
        }
        else
        {
            QModelIndex candidateIndex = cIndex.sibling(cIndex.row() + 1, 0);

            if (!candidateIndex.isValid())
            {
                QModelIndex parentIndex = cIndex.parent();

                if (!parentIndex.isValid())
                {
                    break;
                }

                candidateIndex          = parentIndex.sibling(parentIndex.row() + 1, 0);
            }

            cIndex = candidateIndex;
        }
    }

    return deepRowNumber;
}

QModelIndex TableViewModel::toCol0(const QModelIndex& anIndex) const
{
    return anIndex.sibling(anIndex.row(), 0);
}

int TableViewModel::firstDeepRowNotInList(const QList<QModelIndex>& needleList)
{
    int currentNeedlePos           = 0;
    QModelIndex currentNeedleIndex = toCol0(needleList.first());
    int deepRowNumber              = 0;
    QModelIndex cIndex             = index(0, 0);

    while (cIndex.isValid())
    {
        if (cIndex!=currentNeedleIndex)
        {
            return deepRowNumber;
        }

        if (hasChildren(cIndex))
        {
            cIndex = cIndex.child(0, 0);
        }
        else
        {
            QModelIndex candidateIndex = cIndex.sibling(cIndex.row() + 1, 0);

            if (!candidateIndex.isValid())
            {
                QModelIndex parentIndex = cIndex.parent();

                if (!parentIndex.isValid())
                {
                    break;
                }

                candidateIndex = parentIndex.sibling(parentIndex.row() + 1, 0);
            }

            cIndex = candidateIndex;
        }

        if (cIndex.isValid())
        {
            ++deepRowNumber;
            ++currentNeedlePos;

            if (currentNeedlePos >= needleList.count())
            {
                return deepRowNumber;
            }

            currentNeedleIndex = toCol0(needleList.at(currentNeedlePos));
        }
    }

    return -1;
}

void TableViewModel::slotSetActive(const bool isActive)
{
    if (isActive)
    {
        if (d->outdated)
        {
            // populate the model once later, not now
            QTimer::singleShot(0, this, SLOT(slotPopulateModelWithNotifications()));
        }
    }
}

int TableViewModel::findChildSortedPosition(TableViewModel::Item* const parentItem, TableViewModel::Item* const childItem)
{
    if (parentItem->children.isEmpty())
    {
        return 0;
    }

    // nChildren is guaranteed to be >=1
    const int nChildren = parentItem->children.count();
    int stepSize        = nChildren / 2;
    // make sure pos is at least 0 if there is only one item
    int pos             = qMin(nChildren - 1, stepSize);

    while (true)
    {
        stepSize = stepSize / 2;

        if (stepSize == 0)
        {
            stepSize = 1;
        }

        bool isLessThanUpper = lessThan(childItem, parentItem->children.at(pos));

        if (d->sortOrder == Qt::DescendingOrder)
        {
            isLessThanUpper = !isLessThanUpper;
        }

        if (!isLessThanUpper)
        {
            // need to jump up, quit if we can not jump up by 1
            if (pos + 1 >= nChildren)
            {
                pos = nChildren;
                break;
            }

            // jump up by stepSize and make sure we do not jump over the end
            pos += stepSize;

            if (pos >= nChildren)
            {
                pos = nChildren - 1;
            }

            continue;
        }

        // can we go lower?
        const bool lowerThere = (pos > 0);

        if (!lowerThere)
        {
            // no, stop
            pos = 0;
            break;
        }

        bool isLessThanLower = lessThan(childItem, parentItem->children.at(pos-1));

        if (d->sortOrder == Qt::DescendingOrder)
        {
            isLessThanLower = !isLessThanLower;
        }

        if (isLessThanLower)
        {
            // go lower and make sure we do not jump too low
            pos -= stepSize;

            if (pos < 0)
            {
                pos = 0;
            }

            continue;
        }

        break;
    }

    return pos;
}

} // namespace Digikam

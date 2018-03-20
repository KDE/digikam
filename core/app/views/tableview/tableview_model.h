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

#ifndef TABLEVIEW_MODEL_H
#define TABLEVIEW_MODEL_H

// Qt includes

#include <QAbstractItemModel>
#include <QUrl>

// Local includes

#include "coredbchangesets.h"
#include "tableview_shared.h"

class QMimeData;

namespace Digikam
{

class ImageChangeset;
class ImageFilterModel;
class ImageFilterSettings;
class ImageInfo;
class ImageInfoList;
class TableViewColumn;
class TableViewColumnConfiguration;
class TableViewColumnDescription;
class TableViewColumnFactory;
class TableViewColumnProfile;

class TableViewModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    enum GroupingMode
    {
        GroupingHideGrouped    = 0,
        GroupingIgnoreGrouping = 1,
        GroupingShowSubItems   = 2
    };

    typedef DatabaseFields::Hash<QVariant> DatabaseFieldsHashRaw;

public:

    class Item
    {
    public:

        explicit Item();
        virtual ~Item();

        void  addChild(Item* const newChild);
        void  insertChild(const int pos, Item* const newChild);
        void  takeChild(Item* const oldChild);
        Item* findChildWithImageId(const qlonglong searchImageId);

    public:

        qlonglong    imageId;
        Item*        parent;
        QList<Item*> children;
    };

public:

    explicit TableViewModel(TableViewShared* const sharedObject, QObject* parent = 0);
    virtual ~TableViewModel();

    void                    addColumnAt(const TableViewColumnDescription& description, const int targetColumn = -1);
    void                    addColumnAt(const TableViewColumnConfiguration& cpp, const int targetColumn = -1);
    void                    removeColumnAt(const int columnIndex);
    TableViewColumn*        getColumnObject(const int columnIndex);
    QList<TableViewColumn*> getColumnObjects();
    QModelIndex             fromImageFilterModelIndex(const QModelIndex& imageFilterModelIndex);
    QModelIndex             fromImageModelIndex(const QModelIndex& imageModelIndex);
    QModelIndex             toImageFilterModelIndex(const QModelIndex& i) const;
    QModelIndex             toImageModelIndex(const QModelIndex& i) const;
    void                    loadColumnProfile(const TableViewColumnProfile& columnProfile);
    TableViewColumnProfile  getColumnProfile() const;

    QModelIndex deepRowIndex(const int rowNumber) const;
    int indexToDeepRowNumber(const QModelIndex& index) const;
    int deepRowCount() const;
    int firstDeepRowNotInList(const QList<QModelIndex>& needleList);
    QModelIndex toCol0(const QModelIndex& anIndex) const;

    QModelIndex   itemIndex(Item* const item) const;
    QModelIndex   indexFromImageId(const qlonglong imageId, const int columnIndex) const;
    Item*         itemFromImageId(const qlonglong imageId) const;
    Item*         itemFromIndex(const QModelIndex& i) const;
    ImageInfo     infoFromItem(Item* const item) const;
    ImageInfoList infosFromItems(QList<Item*> const items) const;

    QVariant              itemDatabaseFieldRaw(Item* const item, const DatabaseFields::Set requestedField);
    DatabaseFieldsHashRaw itemDatabaseFieldsRaw(Item* const item, const DatabaseFields::Set requestedSet);

    qlonglong        imageId(const QModelIndex& anIndex) const;
    QList<qlonglong> imageIds(const QModelIndexList& indexList) const;
    QList<ImageInfo> imageInfos(const QModelIndexList& indexList) const;
    ImageInfo        imageInfo(const QModelIndex& index) const;
    QList<ImageInfo> allImageInfo() const;

    QList<Item*> sortItems(const QList<Item*> itemList);
    class LessThan;
    bool lessThan(Item* const itemA, Item* const itemB);
    int findChildSortedPosition(Item* const parentItem, Item* const childItem);

    void scheduleResort();
    GroupingMode groupingMode() const;
    void setGroupingMode(const GroupingMode newGroupingMode);

public:

    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& childIndex) const;
    virtual int rowCount(const QModelIndex& parent) const;
    virtual int columnCount(const QModelIndex& i) const;
    virtual QVariant data(const QModelIndex& i, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;

    // drag-and-drop related functions
    virtual Qt::DropActions supportedDropActions() const;
    virtual QStringList mimeTypes() const;
    virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
    virtual QMimeData* mimeData(const QModelIndexList& indexes) const;

protected:

    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

private Q_SLOTS:

    void slotPopulateModelWithNotifications();
    void slotPopulateModel(const bool sendNotifications);

    void slotColumnDataChanged(const qlonglong imageId);
    void slotColumnAllDataChanged();

    void slotSourceModelAboutToBeReset();
    void slotSourceModelReset();
    void slotSourceRowsAboutToBeInserted(const QModelIndex& parent, int start, int end);
    void slotSourceRowsInserted(const QModelIndex& parent, int start, int end);
    void slotSourceRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void slotSourceRowsRemoved(const QModelIndex& parent, int start, int end);
    void slotSourceRowsAboutToBeMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd,
                                      const QModelIndex& destinationParent, int destinationRow);
    void slotSourceRowsMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd,
                             const QModelIndex& destinationParent, int destinationRow);
    void slotSourceLayoutAboutToBeChanged();
    void slotSourceLayoutChanged();

    void slotDatabaseImageChanged(const ImageChangeset& imageChangeset);

    void slotFilterSettingsChanged(const ImageFilterSettings& settings);
    void slotResortModel();
    void slotClearModel(const bool sendNotifications);

public Q_SLOTS:

    void slotSetActive(const bool isActive);

Q_SIGNALS:

    void signalGroupingModeChanged();

private:

    Item* createItemFromSourceIndex(const QModelIndex& imageFilterModelIndex);
    void addSourceModelIndex(const QModelIndex& imageModelIndex, const bool sendNotifications);

private:

    TableViewShared* const s;
    class Private;
    const QScopedPointer<Private> d;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::TableViewModel::GroupingMode)

#endif // TABLEVIEW_MODEL_H

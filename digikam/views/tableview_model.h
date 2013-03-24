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
#include <QSortFilterProxyModel>

// KDE includes

// local includes

#include "databasechangesets.h"
#include "tableview_shared.h"

class QMimeData;

namespace Digikam
{

class ImageChangeset;
class ImageFilterModel;
class ImageFilterSettings;
class ImageInfo;
class TableViewColumn;
class TableViewColumnConfiguration;
class TableViewColumnDescription;
class TableViewColumnFactory;
class TableViewColumnProfile;

class TableViewModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    typedef DatabaseFields::Hash<QVariant> DatabaseFieldsHashRaw;

    class Item
    {
    public:

        qlonglong imageId;
        DatabaseFields::Set cachedDatabaseFields;
        DatabaseFieldsHashRaw databaseFields;
        Item* parent;
        QList<Item*> children;

    public:

        explicit Item();
        ~Item();

        void addChild(Item* const newChild);
        void takeChild(Item* const oldChild);
        Item* findChildWithImageId(const qlonglong searchImageId);

    };

    explicit TableViewModel(TableViewShared* const sharedObject, QObject* parent = 0);
    virtual ~TableViewModel();

    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& parent) const;
    virtual int rowCount(const QModelIndex& parent) const;
    virtual int columnCount(const QModelIndex& i) const;
    virtual QVariant data(const QModelIndex& i, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    void addColumnAt(const TableViewColumnDescription& description, const int targetColumn = -1);
    void addColumnAt(const TableViewColumnConfiguration& configuration, const int targetColumn = -1);
    void removeColumnAt(const int columnIndex);
    TableViewColumn* getColumnObject(const int columnIndex);
    QList<TableViewColumn*> getColumnObjects();
    QModelIndex fromImageFilterModelIndex(const QModelIndex& imageFilterModelIndex);
    QModelIndex fromImageModelIndex(const QModelIndex& imageModelIndex);
    QModelIndex toImageFilterModelIndex(const QModelIndex& i) const;
    QModelIndex toImageModelIndex(const QModelIndex& i) const;
    void loadColumnProfile(const TableViewColumnProfile& columnProfile);
    TableViewColumnProfile getColumnProfile() const;

    QModelIndex indexFromImageId(const qlonglong imageId, const int columnIndex) const;
    Item* itemFromImageId(const qlonglong imageId) const;
    Item* itemFromIndex(const QModelIndex& i) const;
    ImageInfo infoFromItem(Item* const item) const;
    QVariant itemDatabaseFieldRaw(Item* const item, const DatabaseFields::Set requestedField);
    DatabaseFieldsHashRaw itemDatabaseFieldsRaw(Item* const item, const DatabaseFields::Set requestedSet);
    QList<qlonglong> imageIds(const QModelIndexList& indexList) const;
    QList<ImageInfo> imageInfos(const QModelIndexList& indexList) const;
    ImageInfo imageInfo(const QModelIndex& index) const;

private Q_SLOTS:

    void slotPopulateModel();

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

private:

    Item* createItemFromSourceIndex(const QModelIndex& imageFilterModelIndex);
    void addSourceModelIndex(const QModelIndex& imageFilterModelIndex);

    TableViewShared* const s;
    class Private;
    const QScopedPointer<Private> d;
};

} /* namespace Digikam */

#endif // TABLEVIEW_MODEL_H


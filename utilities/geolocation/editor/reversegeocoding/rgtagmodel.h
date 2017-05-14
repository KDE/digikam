/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-12
 * Description : A model to hold information about image tags.
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
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

#ifndef RGTAGMODEL_H
#define RGTAGMODEL_H

// Qt includes

#include <QAbstractItemModel>
#include <QItemSelectionModel>

// Local includes

#include "gpsimageitem.h"
#include "treebranch.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT RGTagModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    explicit RGTagModel(QAbstractItemModel* const externalTagModel, QObject* const parent = 0);
    ~RGTagModel();

    // QAbstractItemModel:
    virtual int columnCount(const QModelIndex& parent = QModelIndex() ) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex() ) const;
    virtual QModelIndex parent(const QModelIndex& index) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    //Local functions:
    QModelIndex fromSourceIndex(const QModelIndex& externalTagModelIndex) const;
    QModelIndex toSourceIndex(const QModelIndex& tagModelIndex) const;
    void addSpacerTag(const QModelIndex& parent, const QString& spacerName);
    QPersistentModelIndex addNewTag(const QModelIndex& parent, const QString& newTagName);
    QList<QList<TagData> > addNewData(QStringList& elements, QStringList& resultedData);
    void addDataInTree(TreeBranch* currentBranch, int currentRow,const QStringList& addressElements,const QStringList& elementsData);
    QList<TagData> getTagAddress();
    void findAndDeleteSpacersOrNewTags(TreeBranch* currentBranch, int currentRow, Type whatShouldRemove);
    void deleteAllSpacersOrNewTags(const QModelIndex& currentIndex, Type whatShouldRemove);
    void readdTag(TreeBranch*& currentBranch, int currentRow,const QList<TagData> tagAddressElements, int currentAddressElementIndex);
    void readdNewTags(const QList<QList<TagData> >& tagAddressList);
    void deleteTag(const QModelIndex& currentIndex);
    QList<QList<TagData> > getSpacers();
    void climbTreeAndGetSpacers(const TreeBranch* currentBranch);
    QList<TagData> getSpacerAddress(TreeBranch* currentBranch);
    void addExternalTags(TreeBranch* parentBranch, int currentRow);
    void addAllExternalTagsToTreeView();
    void addAllSpacersToTag(const QModelIndex currentIndex, const QStringList spacerList, int spacerListIndex);
    Type getTagType(const QModelIndex& index) const;
    TreeBranch* branchFromIndex(const QModelIndex& index) const;

public Q_SLOTS:

    void slotSourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void slotSourceHeaderDataChanged(const Qt::Orientation orientation, int first, int last);
    void slotColumnsAboutToBeInserted ( const QModelIndex & parent, int start, int end);
    void slotColumnsAboutToBeMoved ( const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationColumn);
    void slotColumnsAboutToBeRemoved ( const QModelIndex & parent, int start, int end );
    void slotColumnsInserted ();
    void slotColumnsMoved ();
    void slotColumnsRemoved ();
    void slotLayoutAboutToBeChanged ();
    void slotLayoutChanged();
    void slotModelAboutToBeReset();
    void slotModelReset();
    void slotRowsAboutToBeInserted (const QModelIndex & parent, int start, int end);
    void slotRowsAboutToBeMoved (const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);
    void slotRowsAboutToBeRemoved (const QModelIndex & parent, int start, int end);
    void slotRowsInserted ();
    void slotRowsMoved ();
    void slotRowsRemoved ();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // RGTAGMODEL_H

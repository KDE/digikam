/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 30-07-2013
 * Description : Qt filter model for showfoto items
 *
 * Copyright (C) 2013 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "showfotofiltermodel.h"

// Local includes

#include "showfotoiteminfo.h"

namespace ShowFoto
{

ShowfotoSortFilterModel::ShowfotoSortFilterModel(QObject* const parent)
    : DCategorizedSortFilterProxyModel(parent),
      m_chainedModel(0)
{
}

ShowfotoSortFilterModel::~ShowfotoSortFilterModel()
{
}

void ShowfotoSortFilterModel::setSourceShowfotoModel(ShowfotoImageModel* const sourceModel)
{
    if (m_chainedModel)
    {
        m_chainedModel->setSourceShowfotoModel(sourceModel);
    }
    else
    {
        setDirectSourceShowfotoModel(sourceModel);
    }
}

ShowfotoImageModel* ShowfotoSortFilterModel::sourceShowfotoModel() const
{
    if (m_chainedModel)
    {
        return m_chainedModel->sourceShowfotoModel();
    }

    return static_cast<ShowfotoImageModel*>(sourceModel());
}

void ShowfotoSortFilterModel::setSourceFilterModel(ShowfotoSortFilterModel* const sourceModel)
{
    if (sourceModel)
    {
        ShowfotoImageModel* const model = sourceShowfotoModel();

        if (model)
        {
            sourceModel->setSourceShowfotoModel(model);
        }
    }

    m_chainedModel = sourceModel;
    setSourceModel(sourceModel);
}

ShowfotoSortFilterModel* ShowfotoSortFilterModel::sourceFilterModel() const
{
    return m_chainedModel;
}

QModelIndex ShowfotoSortFilterModel::mapToSourceShowfotoModel(const QModelIndex& proxyIndex) const
{
    if (m_chainedModel)
    {
        return m_chainedModel->mapToSourceShowfotoModel(mapToSource(proxyIndex));
    }

    return mapToSource(proxyIndex);
}

QModelIndex ShowfotoSortFilterModel::mapFromSourceShowfotoModel(const QModelIndex& ShowfotoModelIndex) const
{
    if (m_chainedModel)
    {
        return mapFromSource(m_chainedModel->mapFromSourceShowfotoModel(ShowfotoModelIndex));
    }

    return mapFromSource(ShowfotoModelIndex);
}

QModelIndex ShowfotoSortFilterModel::mapFromDirectSourceToSourceShowfotoModel(const QModelIndex& sourceModelIndex) const
{
    if (m_chainedModel)
    {
        return m_chainedModel->mapToSourceShowfotoModel(sourceModelIndex);
    }

    return sourceModelIndex;
}

QList<QModelIndex> ShowfotoSortFilterModel::mapListToSource(const QList<QModelIndex>& indexes) const
{
    QList<QModelIndex> sourceIndexes;

    foreach (const QModelIndex& index, indexes)
    {
        sourceIndexes << mapToSourceShowfotoModel(index);
    }

    return sourceIndexes;
}

QList<QModelIndex> ShowfotoSortFilterModel::mapListFromSource(const QList<QModelIndex>& sourceIndexes) const
{
    QList<QModelIndex> indexes;

    foreach (const QModelIndex& index, sourceIndexes)
    {
        indexes << mapFromSourceShowfotoModel(index);
    }

    return indexes;
}

ShowfotoItemInfo ShowfotoSortFilterModel::showfotoItemInfo(const QModelIndex& index) const
{
    return sourceShowfotoModel()->showfotoItemInfo(mapToSourceShowfotoModel(index));
}

QList<ShowfotoItemInfo> ShowfotoSortFilterModel::showfotoItemInfos(const QList<QModelIndex>& indexes) const
{
    QList<ShowfotoItemInfo> infos;

    foreach (const QModelIndex& index, indexes)
    {
        infos << showfotoItemInfo(index);
    }

    return infos;
}

QModelIndex ShowfotoSortFilterModel::indexForUrl(const QUrl& fileUrl) const
{
    return mapFromSourceShowfotoModel(sourceShowfotoModel()->indexForUrl(fileUrl));
}

QModelIndex ShowfotoSortFilterModel::indexForShowfotoItemInfo(const ShowfotoItemInfo& info) const
{
    return mapFromSourceShowfotoModel(sourceShowfotoModel()->indexForShowfotoItemInfo(info));
}

QModelIndex ShowfotoSortFilterModel::indexForShowfotoItemId(qlonglong id) const
{
    return mapFromSourceShowfotoModel(sourceShowfotoModel()->indexForShowfotoItemId(id));
}

QList<ShowfotoItemInfo> ShowfotoSortFilterModel::showfotoItemInfosSorted() const
{
    QList<ShowfotoItemInfo> infos;
    const int size = rowCount();

    for (int i = 0; i < size; i++)
    {
        infos << showfotoItemInfo(index(i, 0));
    }

    return infos;
}

ShowfotoFilterModel* ShowfotoSortFilterModel::showfotoFilterModel() const
{
    if (m_chainedModel)
    {
        return m_chainedModel->showfotoFilterModel();
    }

    return 0;
}

void ShowfotoSortFilterModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    DCategorizedSortFilterProxyModel::setSourceModel(sourceModel);
}

void ShowfotoSortFilterModel::setDirectSourceShowfotoModel(ShowfotoImageModel* const sourceModel)
{
    setSourceModel(sourceModel);
}

//--- ShowfotoFilterModel methods ---------------------------------

class ShowfotoFilterModel::ShowfotoFilterModelPrivate : public QObject
{

public:

    ShowfotoFilterModelPrivate()
    {
        q                  = 0;
        showfotoImageModel = 0;
    }

    void init(ShowfotoFilterModel* const _q);

Q_SIGNALS:

    void reAddShowfotoItemInfos(const QList<ShowfotoItemInfo>&);
    void reAddingFinished();

public:

    ShowfotoFilterModel*     q;
    ShowfotoImageModel*      showfotoImageModel;
    ShowfotoItemSortSettings sorter;
};

void ShowfotoFilterModel::ShowfotoFilterModelPrivate::init(ShowfotoFilterModel* const _q)
{
    q = _q;
}

ShowfotoFilterModel::ShowfotoFilterModel(QObject* const parent)
    : ShowfotoSortFilterModel(parent),
      d_ptr(new ShowfotoFilterModelPrivate)
{
    d_ptr->init(this);
}

ShowfotoFilterModel::~ShowfotoFilterModel()
{
    Q_D(ShowfotoFilterModel);
    delete d;
}

QVariant ShowfotoFilterModel::data(const QModelIndex& index, int role) const
{
    Q_D(const ShowfotoFilterModel);

    if (!index.isValid())
    {
        return QVariant();
    }

    switch (role)
    {
        case DCategorizedSortFilterProxyModel::CategoryDisplayRole:
            return categoryIdentifier(d->showfotoImageModel->showfotoItemInfoRef(mapToSource(index)));

        case CategorizationModeRole:
            return d->sorter.categorizationMode;

        case SortOrderRole:
            return d->sorter.sortRole;

        case CategoryFormatRole:
            return d->showfotoImageModel->showfotoItemInfoRef(mapToSource(index)).mime;

        case ShowfotoFilterModelPointerRole:
            return QVariant::fromValue(const_cast<ShowfotoFilterModel*>(this));
    }

    return DCategorizedSortFilterProxyModel::data(index, role);
}

ShowfotoFilterModel* ShowfotoFilterModel::showfotoFilterModel() const
{
    return const_cast<ShowfotoFilterModel*>(this);
}

// --- Sorting and Categorization ----------------------------------------------

void ShowfotoFilterModel::setShowfotoItemSortSettings(const ShowfotoItemSortSettings& sorter)
{
    Q_D(ShowfotoFilterModel);
    d->sorter = sorter;
    setCategorizedModel(d->sorter.categorizationMode != ShowfotoItemSortSettings::NoCategories);
    invalidate();
}

void ShowfotoFilterModel::setCategorizationMode(ShowfotoItemSortSettings::CategorizationMode mode)
{
    Q_D(ShowfotoFilterModel);
    d->sorter.setCategorizationMode(mode);
    setShowfotoItemSortSettings(d->sorter);
}

void ShowfotoFilterModel::setSortRole(ShowfotoItemSortSettings::SortRole role)
{
    Q_D(ShowfotoFilterModel);
    d->sorter.setSortRole(role);
    setShowfotoItemSortSettings(d->sorter);
}

void ShowfotoFilterModel::setSortOrder(ShowfotoItemSortSettings::SortOrder order)
{
    Q_D(ShowfotoFilterModel);
    d->sorter.setSortOrder(order);
    setShowfotoItemSortSettings(d->sorter);
}

void ShowfotoFilterModel::setSendShowfotoItemInfoSignals(bool sendSignals)
{
    if (sendSignals)
    {
        connect(this, &ShowfotoFilterModel::rowsInserted, this, &ShowfotoFilterModel::slotRowsInserted);

        connect(this, &ShowfotoFilterModel::rowsAboutToBeRemoved, this, &ShowfotoFilterModel::slotRowsAboutToBeRemoved);
    }
    else
    {
        disconnect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SLOT(slotRowsInserted(QModelIndex,int,int)));

        disconnect(this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SLOT(slotRowsAboutToBeRemoved(QModelIndex,int,int)));
    }
}

void ShowfotoFilterModel::slotRowsInserted(const QModelIndex& /*parent*/, int start, int end)
{
    QList<ShowfotoItemInfo> infos;

    for (int i = start; i < end; i++)
    {
        infos << showfotoItemInfo(index(i, 0));
    }

    emit showfotoItemInfosAdded(infos);
}

void ShowfotoFilterModel::slotRowsAboutToBeRemoved(const QModelIndex& /*parent*/, int start, int end)
{
    QList<ShowfotoItemInfo> infos;

    for (int i = start; i < end; i++)
    {
        infos << showfotoItemInfo(index(i, 0));
    }

    emit showfotoItemInfosAboutToBeRemoved(infos);
}

void ShowfotoFilterModel::setDirectSourceShowfotoModel(ShowfotoImageModel* const sourceModel)
{
    Q_D(ShowfotoFilterModel);

    if (d->showfotoImageModel)
    {
        //disconnect(d->ShowfotoImageModel, SIGNAL(modelReset()),
                           //this, SLOT(slotModelReset()));
        //TODO: slotModelReset(); will be added when implementing filtering options
    }

    d->showfotoImageModel = sourceModel;

    if (d->showfotoImageModel)
    {
        //connect(d, SIGNAL(reAddShowfotoItemInfos(QList<ShowfotoItemInfo>)),
                //d->showfotoImageModel, SLOT(reAddShowfotoItemInfos(QList<ShowfotoItemInfo>)));

        //connect(d, SIGNAL(reAddingFinished()),
                //d->showfotoImageModel, SLOT(reAddingFinished()));

        //TODO: connect(d->showfotoImageModel, SIGNAL(modelReset()), this, SLOT(slotModelReset()));
    }

    setSourceModel(d->showfotoImageModel);
}

int ShowfotoFilterModel::compareCategories(const QModelIndex& left, const QModelIndex& right) const
{
    Q_D(const ShowfotoFilterModel);

    if (!d->sorter.isCategorized())
    {
        return 0;
    }

    if (!left.isValid() || !right.isValid())
    {
        return -1;
    }

    return compareInfosCategories(d->showfotoImageModel->showfotoItemInfoRef(left), d->showfotoImageModel->showfotoItemInfoRef(right));
}

bool ShowfotoFilterModel::subSortLessThan(const QModelIndex& left, const QModelIndex& right) const
{
    Q_D(const ShowfotoFilterModel);

    if (!left.isValid() || !right.isValid())
    {
        return true;
    }

    if (left == right)
    {
        return false;
    }

    const ShowfotoItemInfo& leftInfo  = d->showfotoImageModel->showfotoItemInfoRef(left);
    const ShowfotoItemInfo& rightInfo = d->showfotoImageModel->showfotoItemInfoRef(right);

    if (leftInfo == rightInfo)
    {
        return d->sorter.lessThan(left.data(ShowfotoImageModel::ExtraDataRole), right.data(ShowfotoImageModel::ExtraDataRole));
    }

    return infosLessThan(leftInfo, rightInfo);
}

int ShowfotoFilterModel::compareInfosCategories(const ShowfotoItemInfo& left, const ShowfotoItemInfo& right) const
{
    Q_D(const ShowfotoFilterModel);
    return d->sorter.compareCategories(left, right);
}

bool ShowfotoFilterModel::infosLessThan(const ShowfotoItemInfo& left, const ShowfotoItemInfo& right) const
{
    Q_D(const ShowfotoFilterModel);
    return d->sorter.lessThan(left, right);
}

QString ShowfotoFilterModel::categoryIdentifier(const ShowfotoItemInfo& info) const
{
    Q_D(const ShowfotoFilterModel);

    switch (d->sorter.categorizationMode)
    {
        case ShowfotoItemSortSettings::NoCategories:
            return QString();
        case ShowfotoItemSortSettings::CategoryByFolder:
            return info.folder;
        case ShowfotoItemSortSettings::CategoryByFormat:
            return info.mime;
        default:
            return QString();
    }
}

// -------------------------------------------------------------------------------------------------------

NoDuplicatesShowfotoFilterModel::NoDuplicatesShowfotoFilterModel(QObject* const parent)
    : ShowfotoSortFilterModel(parent)
{
}

bool NoDuplicatesShowfotoFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    if (index.data(ShowfotoImageModel::ExtraDataDuplicateCount).toInt() <= 1)
    {
        return true;
    }

    QModelIndex previousIndex = sourceModel()->index(source_row - 1, 0, source_parent);

    if (!previousIndex.isValid())
    {
        return true;
    }

    return true;
}

} // namespace ShowFoto

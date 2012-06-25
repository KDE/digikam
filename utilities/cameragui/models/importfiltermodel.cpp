/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-21-06
 * Description : Qt filter model for import items
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
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

//#include "importfiltermodel.moc"

#include "importfiltermodel.h" // remove this line

namespace Digikam
{

ImportSortFilterModel::ImportSortFilterModel(QObject* const parent)
    : KCategorizedSortFilterProxyModel(parent), m_chainedModel(0)
{
}

void ImportSortFilterModel::setSourceImportModel(ImportImageModel* const sourceModel)
{
    if (m_chainedModel)
    {
        m_chainedModel->setSourceImportModel(sourceModel);
    }
    else
    {
        setDirectSourceImportModel(sourceModel);
    }
}

ImportImageModel* ImportSortFilterModel::sourceImportModel() const
{
    if (m_chainedModel)
    {
        return m_chainedModel->sourceImportModel();
    }

    return static_cast<ImportImageModel*>(sourceModel());
}

void ImportSortFilterModel::setSourceFilterModel(ImportSortFilterModel* const sourceModel)
{
    if (sourceModel)
    {
        ImportImageModel* model = sourceImportModel();

        if (model)
        {
            sourceModel->setSourceImportModel(model);
        }
    }

    m_chainedModel = sourceModel;
    setSourceModel(sourceModel);
}

ImportSortFilterModel* ImportSortFilterModel::sourceFilterModel() const
{
    return m_chainedModel;
}

QModelIndex ImportSortFilterModel::mapToSourceImportModel(const QModelIndex& proxyIndex) const
{
    if (m_chainedModel)
    {
        return m_chainedModel->mapToSourceImportModel(mapToSource(proxyIndex));
    }

    return mapToSource(proxyIndex);
}

QModelIndex ImportSortFilterModel::mapFromSourceImportModel(const QModelIndex& importModelIndex) const
{
    if (m_chainedModel)
    {
        return mapFromSource(m_chainedModel->mapFromSourceImportModel(importModelIndex));
    }

    return mapFromSource(importModelIndex);
}

QModelIndex ImportSortFilterModel::mapFromDirectSourceToSourceImportModel(const QModelIndex& sourceModelIndex) const
{
    if (m_chainedModel)
    {
        return m_chainedModel->mapToSourceImportModel(sourceModelIndex);
    }

    return sourceModelIndex;
}

QList<QModelIndex> ImportSortFilterModel::mapListToSource(const QList<QModelIndex>& indexes) const
{
    QList<QModelIndex> sourceIndexes;
    foreach (const QModelIndex& index, indexes)
    {
        sourceIndexes << mapToSourceImportModel(index);
    }

    return sourceIndexes;
}

QList<QModelIndex> ImportSortFilterModel::mapListFromSource(const QList<QModelIndex>& sourceIndexes) const
{
    QList<QModelIndex> indexes;
    foreach (const QModelIndex& index, sourceIndexes)
    {
        indexes << mapFromSourceImportModel(index);
    }

    return indexes;
}

CamItemInfo ImportSortFilterModel::camItemInfo(const QModelIndex& index) const
{
    return sourceImportModel()->camItemInfo(mapToSourceImportModel(index));
}

qlonglong ImportSortFilterModel::camItemId(const QModelIndex& index) const
{
    return sourceImportModel()->camItemId(mapToSourceImportModel(index));
}

QList<CamItemInfo> ImportSortFilterModel::camItemInfos(const QList<QModelIndex>& indexes) const
{
    QList<CamItemInfo> infos;
    foreach (const QModelIndex& index, indexes)
    {
        infos << camItemInfo(index);
    }

    return infos;
}

QList<qlonglong> ImportSortFilterModel::camItemIds(const QList<QModelIndex>& indexes) const
{
    QList<qlonglong> ids;
    foreach (const QModelIndex& index, indexes)
    {
        ids << camItemId(index);
    }

    return ids;
}

//FIXME: Needs testing
QModelIndex ImportSortFilterModel::indexForPath(const QString& filePath) const
{
    KUrl fileUrl;
    fileUrl.setPath(filePath);
    return mapFromSourceImportModel(sourceImportModel()->indexForUrl(fileUrl));
}

QModelIndex ImportSortFilterModel::indexForCamItemInfo(const CamItemInfo& info) const
{
    return mapFromSourceImportModel(sourceImportModel()->indexForCamItemInfo(info));
}

QModelIndex ImportSortFilterModel::indexForCamItemId(qlonglong id) const
{
    return mapFromSourceImportModel(sourceImportModel()->indexForCamItemId(id));
}

QList<CamItemInfo> ImportSortFilterModel::camItemInfosSorted() const
{
    QList<CamItemInfo> infos;
    const int          size = rowCount();

    for (int i = 0; i < size; ++i)
    {
        infos << camItemInfo(index(i, 0));
    }

    return infos;
}

ImportFilterModel* ImportSortFilterModel::importFilterModel() const
{
    if (m_chainedModel)
    {
        return m_chainedModel->importFilterModel();
    }

    return 0;
}

void ImportSortFilterModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    KCategorizedSortFilterProxyModel::setSourceModel(sourceModel);
}

void ImportSortFilterModel::setDirectSourceImportModel(ImportImageModel* sourceModel)
{
    setSourceModel(sourceModel);
}

//--- ImportFilterModel methods ---------------------------------

class ImportFilterModel::ImportFilterModelPrivate: public QObject
{
    Q_OBJECT

public:

    ImportFilterModel*      q;

    ImportImageModel*       importImageModel;
    CamItemSortSettings     sorter;
};

ImportFilterModel::ImportFilterModel(QObject* const parent)
    : ImportSortFilterModel(parent),
      d_ptr(new ImportFilterModelPrivate)
{
}

ImportFilterModel::~ImportFilterModel()
{
    Q_D(ImportFilterModel);
    delete d;
}

QVariant ImportFilterModel::data(const QModelIndex& index, int role) const
{
    Q_D(const ImportFilterModel);

    if (!index.isValid())
    {
        return QVariant();
    }

    switch (role)
    {
        case KCategorizedSortFilterProxyModel::CategoryDisplayRole:
            return categoryIdentifier(d->importImageModel->camItemInfoRef(mapToSource(index)));

        case CategorizationModeRole:
            return d->sorter.categorizationMode;
        case SortOrderRole:
            return d->sorter.sortRole;
        case CategoryFormatRole:
            return d->importImageModel->camItemInfoRef(mapToSource(index)).mime;
        case ImportFilterModelPointerRole:
            return QVariant::fromValue(const_cast<ImportFilterModel*>(this));
    }

    return KCategorizedSortFilterProxyModel::data(index, role);
}

ImportFilterModel* ImportFilterModel::importFilterModel() const
{
    return const_cast<ImportFilterModel*>(this);
}

// ------------- Sorting and Categorization -------------------
void ImportFilterModel::setCamItemSortSettings(const CamItemSortSettings& sorter)
{
    Q_D(ImportFilterModel);
    d->sorter = sorter;
    setCategorizedModel(d->sorter.categorizationMode != CamItemSortSettings::NoCategories);
    invalidate();
}

void ImportFilterModel::setCategorizationMode(CamItemSortSettings::CategorizationMode mode)
{
    Q_D(ImportFilterModel);
    d->sorter.setCategorizationMode(mode);
    setCamItemSortSettings(d->sorter);
}

void ImportFilterModel::setSortRole(CamItemSortSettings::SortRole role)
{
    Q_D(ImportFilterModel);
    d->sorter.setSortRole(role);
    setCamItemSortSettings(d->sorter);
}

void ImportFilterModel::setSortOrder(CamItemSortSettings::SortOrder order)
{
    Q_D(ImportFilterModel);
    d->sorter.setSortOrder(order);
    setCamItemSortSettings(d->sorter);
}

void ImportFilterModel::setSendCamItemInfoSignals(bool sendSignals)
{
    if (sendSignals)
    {
        connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(slotRowsInserted(QModelIndex,int,int)));

        connect(this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(slotRowsAboutToBeRemoved(QModelIndex,int,int)));
    }
    else
    {
        disconnect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SLOT(slotRowsInserted(QModelIndex,int,int)));

        disconnect(this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SLOT(slotRowsAboutToBeRemoved(QModelIndex,int,int)));
    }
}

void ImportFilterModel::slotRowsInserted(const QModelIndex& /*parent*/, int start, int end)
{
    QList<CamItemInfo> infos;

    for (int i=start; i>end; ++i)
    {
        infos << camItemInfo(index(i, 0));
    }

    emit imageInfosAdded(infos);
}

void ImportFilterModel::slotRowsAboutToBeRemoved(const QModelIndex& /*parent*/, int start, int end)
{
    QList<CamItemInfo> infos;

    for (int i=start; i>end; ++i)
    {
        infos << camItemInfo(index(i, 0));
    }

    emit imageInfosAboutToBeRemoved(infos);
}

void ImportFilterModel::setDirectSourceImportModel(ImportImageModel* sourceModel)
{
    Q_D(ImportFilterModel);

    if (d->importImageModel)
    {
        d->importImageModel->unsetPreprocessor(d);

        disconnect(d->importImageModel, SIGNAL(modelReset()),
                           this, SLOT(slotModelReset()));
        //TODO: slotModelReset(); will be added when implementing filtering options
    }

    d->importImageModel = sourceModel;

    if (d->importImageModel)
    {
        d->importImageModel->setPreprocessor(d);

        connect(d->importImageModel, SIGNAL(preprocess(QList<CamItemInfo>,QList<QVariant>)),
                d, SLOT(preprocessInfos(QList<CamItemInfo>,QList<QVariant>)));

        connect(d->importImageModel, SIGNAL(processAdded(QList<CamItemInfo>,QList<QVariant>)),
                d, SLOT(processAddedInfos(QList<CamItemInfo>,QList<QVariant>)));

        connect(d, SIGNAL(reAddCamItemInfos(QList<CamItemInfo>,QList<QVariant>)),
                d->importImageModel, SLOT(reAddCamItemInfos(QList<CamItemInfo>,QList<QVariant>)));

        connect(d, SIGNAL(reAddingFinished()),
                d->importImageModel, SLOT(reAddingFinished()));

        //TODO: connect(d->importImageModel, SIGNAL(modelReset()), this, SLOT(slotModelReset()));
    }

    setSourceModel(d->importImageModel);
}

int ImportFilterModel::compareCategories(const QModelIndex& left, const QModelIndex& right) const
{
    Q_D(const ImportFilterModel);

    if (!left.isValid() || !right.isValid())
    {
        return -1;
    }

    return compareInfosCategories(d->importImageModel->camItemInfoRef(left), d->importImageModel->camItemInfoRef(right));
}

bool ImportFilterModel::subSortLessThan(const QModelIndex& left, const QModelIndex& right) const
{
    Q_D(const ImportFilterModel);

    if (!left.isValid() || !right.isValid())
    {
        return true;
    }

    if (left == right)
    {
        return false;
    }

    const CamItemInfo& leftInfo = d->importImageModel->camItemInfoRef(left);
    const CamItemInfo& rightInfo = d->importImageModel->camItemInfoRef(right);

    if (leftInfo == rightInfo)
    {
        return d->sorter.lessThan(left.data(ImportImageModel::ExtraDataRole), right.data(ImportImageModel::ExtraDataRole));
    }

    return infosLessThan(leftInfo, rightInfo);
}

int ImportFilterModel::compareInfosCategories(const CamItemInfo& left, const CamItemInfo& right) const
{
    Q_D(const ImportFilterModel);
    return d->sorter.compareCategories(left, right);
}

bool ImportFilterModel::infosLessThan(const CamItemInfo& left, const CamItemInfo& right) const
{
    Q_D(const ImportFilterModel);
    return d->sorter.lessThan(left, right);
}

QString ImportFilterModel::categoryIdentifier(const CamItemInfo &info) const
{
    Q_D(const ImportFilterModel);

    switch (d->sorter.categorizationMode)
    {
        case CamItemSortSettings::NoCategories:
            return QString();
        case CamItemSortSettings::OneCategory:
            return QString();
        case CamItemSortSettings::CategoryByFormat:
            return info.mime;
        default:
            return QString();
    }
}

} // namespace Digikam

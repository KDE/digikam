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

#ifndef IMPORTFILTERMODEL_H
#define IMPORTFILTERMODEL_H

// Qt includes

#include <QObject>

// Local includes

#include "dcategorizedsortfilterproxymodel.h"
#include "camitemsortsettings.h"
#include "importimagemodel.h"

namespace Digikam
{
class Filter;
class ImportFilterModel;

class ImportSortFilterModel : public DCategorizedSortFilterProxyModel
{
    Q_OBJECT

public:

    explicit ImportSortFilterModel(QObject* const parent = 0);
    ~ImportSortFilterModel();

    void setSourceImportModel(ImportImageModel* const sourceModel);
    ImportImageModel* sourceImportModel() const;

    void setSourceFilterModel(ImportSortFilterModel* const sourceModel);
    ImportSortFilterModel* sourceFilterModel() const;

    /// Convenience methods mapped to ImportImageModel.
    /// Mentioned indexes returned come from the source import image model.
    QModelIndex mapToSourceImportModel(const QModelIndex& proxyIndex)                       const;
    QModelIndex mapFromSourceImportModel(const QModelIndex& importModelIndex)               const;
    QModelIndex mapFromDirectSourceToSourceImportModel(const QModelIndex& sourceModelIndex) const;

    QList<QModelIndex> mapListToSource(const QList<QModelIndex>& indexes)         const;
    QList<QModelIndex> mapListFromSource(const QList<QModelIndex>& sourceIndexes) const;

    CamItemInfo        camItemInfo(const QModelIndex& index)           const;
    qlonglong          camItemId(const QModelIndex& index)             const;
    QList<CamItemInfo> camItemInfos(const QList<QModelIndex>& indexes) const;
    QList<qlonglong>   camItemIds(const QList<QModelIndex>& indexes)   const;

    QModelIndex indexForPath(const QString& filePath)        const;
    QModelIndex indexForCamItemInfo(const CamItemInfo& info) const;
    QModelIndex indexForCamItemId(qlonglong id)              const;

    /** Returns a list of all camera infos, sorted according to this model.
     *  If you do not need a sorted list, use ImportImageModel's camItemInfo() method.
     */
    QList<CamItemInfo> camItemInfosSorted() const;

    /// Returns this, any chained ImportFilterModel, or 0.
    virtual ImportFilterModel* importFilterModel() const;

protected:

    virtual void setSourceModel(QAbstractItemModel* sourceModel);

    /// Reimplement if needed. Called only when model shall be set as (direct) sourceModel.
    virtual void setDirectSourceImportModel(ImportImageModel* const sourceModel);

protected:

    ImportSortFilterModel* m_chainedModel;
};

// ------------------------------------------------------------------------------------------

class ImportFilterModel : public ImportSortFilterModel
{
    Q_OBJECT

public:

    enum ImportFilterModelRoles
    {
        /// Returns the current categorization mode.
        CategorizationModeRole       = ImportImageModel::FilterModelRoles + 1,

        /// Returns the current sort order.
        SortOrderRole                = ImportImageModel::FilterModelRoles + 2,

        /// Returns the format of the index which is used for category.
        CategoryFormatRole           = ImportImageModel::FilterModelRoles + 3,

        /// Returns the date of the index which is used for category.
        CategoryDateRole             = ImportImageModel::FilterModelRoles + 4,

        /// Returns true if the given camera item is a group leader, and the group is opened.
        //TODO: GroupIsOpenRole            = ImportImageModel::FilterModelRoles + 5
        ImportFilterModelPointerRole = ImportImageModel::FilterModelRoles + 50
    };

public:

    explicit ImportFilterModel(QObject* const parent = 0);
    ~ImportFilterModel();

    CamItemSortSettings camItemSortSettings() const;

    void setCamItemSortSettings(const CamItemSortSettings& sorter);

    /// Enables sending camItemInfosAdded and camItemInfosAboutToBeRemoved.
    void setSendCamItemInfoSignals(bool sendSignals);

    //TODO: Implement grouping in import tool.
    //bool isGroupOpen(qlonglong group) const;
    //bool isAllGroupsOpen() const;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual ImportFilterModel* importFilterModel()                              const;

public Q_SLOTS:

    void setCategorizationMode(CamItemSortSettings::CategorizationMode mode);
    void setSortRole(CamItemSortSettings::SortRole role);
    void setSortOrder(CamItemSortSettings::SortOrder order);
    void setStringTypeNatural(bool natural);
    void setFilter(Filter*);
    void setCameraThumbsController(CameraThumbsCtrl* const thumbsCtrl);

    //TODO: Implement grouping in import tool.
    //void setGroupOpen(qlonglong group, bool open);
    //void toggleGroupOpen(qlonglong group);
    //void setAllGroupsOpen(bool open);

    /** Changes the current image filter settings and refilters. */
    //TODO: Implement filtering in import tool.
    //virtual void setImageFilterSettings(const ImageFilterSettings& settings);

    /** Changes the current image sort settings and resorts. */
    //TODO: virtual void setImageSortSettings(const ImageSortSettings& settings);

Q_SIGNALS:

    /** These signals need to be explicitly enabled with setSendImageInfoSignals().
     */
    void camItemInfosAdded(const QList<CamItemInfo>& infos);
    void camItemInfosAboutToBeRemoved(const QList<CamItemInfo>& infos);

protected Q_SLOTS:

    void slotRowsInserted(const QModelIndex& parent, int start, int end);
    void slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void slotProcessAdded(const QList<CamItemInfo>&);

public:

    // Declared as public because of use in sub-classes.
    class ImportFilterModelPrivate;

protected:

    ImportFilterModelPrivate* const d_ptr;

protected:

    virtual void setDirectSourceImportModel(ImportImageModel* const sourceModel);

    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

    virtual int compareCategories(const QModelIndex& left, const QModelIndex& right) const;
    virtual bool subSortLessThan(const QModelIndex& left, const QModelIndex& right) const;

    /** Reimplement to customize category sorting,
     *  Return negative if category of left < category right,
     *  Return 0 if left and right are in the same category, else return positive.
     */
    virtual int compareInfosCategories(const CamItemInfo& left, const CamItemInfo& right) const;

    /** Reimplement to customize sorting. Do not take categories into account here.
     */
    virtual bool infosLessThan(const CamItemInfo& left, const CamItemInfo& right) const;

    /** Returns a unique identifier for the category if info. The string need not be for user display.
     */
    virtual QString categoryIdentifier(const CamItemInfo& info) const;

private:

    Q_DECLARE_PRIVATE(ImportFilterModel)
};

// -----------------------------------------------------------------------------------------------------

class NoDuplicatesImportFilterModel : public ImportSortFilterModel
{
    Q_OBJECT

public:

    explicit NoDuplicatesImportFilterModel(QObject* const parent = 0);

protected:

    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ImportFilterModel*)

#endif // IMPORTFILTERMODEL_H

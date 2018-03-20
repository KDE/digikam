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

#ifndef SHOWFOTOFILTERMODEL_H
#define SHOWFOTOFILTERMODEL_H

// Qt includes

#include <QObject>

// Local includes

#include "dcategorizedsortfilterproxymodel.h"
#include "showfotoimagemodel.h"
#include "showfotothumbnailmodel.h"
#include "showfotoitemsortsettings.h"

namespace ShowFoto
{

class ShowfotoFilterModel;

class ShowfotoSortFilterModel : public DCategorizedSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit ShowfotoSortFilterModel(QObject* const parent = 0);
    ~ShowfotoSortFilterModel();

    void setSourceShowfotoModel(ShowfotoImageModel* const sourceModel);
    ShowfotoImageModel* sourceShowfotoModel() const;

    void setSourceFilterModel(ShowfotoSortFilterModel* const sourceModel);
    ShowfotoSortFilterModel* sourceFilterModel() const;

    /// Convenience methods mapped to ShowfotoImageModel.
    /// Mentioned indexes returned come from the source Showfoto image model.
    QModelIndex mapToSourceShowfotoModel(const QModelIndex& proxyIndex)                       const;
    QModelIndex mapFromSourceShowfotoModel(const QModelIndex& ShowfotoModelIndex)             const;
    QModelIndex mapFromDirectSourceToSourceShowfotoModel(const QModelIndex& sourceModelIndex) const;

    QList<QModelIndex> mapListToSource(const QList<QModelIndex>& indexes)         const;
    QList<QModelIndex> mapListFromSource(const QList<QModelIndex>& sourceIndexes) const;

    ShowfotoItemInfo        showfotoItemInfo(const QModelIndex& index)           const;
    qlonglong          showfotoItemId(const QModelIndex& index)                  const;
    QList<ShowfotoItemInfo> showfotoItemInfos(const QList<QModelIndex>& indexes) const;
    QList<qlonglong>   showfotoItemIds(const QList<QModelIndex>& indexes)        const;

    QModelIndex indexForUrl(const QUrl& fileUrl)                       const;
    QModelIndex indexForShowfotoItemInfo(const ShowfotoItemInfo& info) const;
    QModelIndex indexForShowfotoItemId(qlonglong id)                   const;

    /** Returns a list of all showfoto infos, sorted according to this model.
     *  If you do not need a sorted list, use ShowfotoImageModel's showfotoItemInfo() method.
     */
    QList<ShowfotoItemInfo> showfotoItemInfosSorted() const;

    /// Returns this, any chained ShowfotoFilterModel, or 0.
    virtual ShowfotoFilterModel* showfotoFilterModel() const;

protected:

    virtual void setSourceModel(QAbstractItemModel* sourceModel);

    /// Reimplement if needed. Called only when model shall be set as (direct) sourceModel.
    virtual void setDirectSourceShowfotoModel(ShowfotoImageModel* const sourceModel);

protected:

    ShowfotoSortFilterModel* m_chainedModel;
};

// ------------------------------------------------------------------------------------------

class ShowfotoFilterModel : public ShowfotoSortFilterModel
{
    Q_OBJECT

public:

    enum ShowfotoFilterModelRoles
    {
        /// Returns the current categorization mode.
        CategorizationModeRole         = ShowfotoImageModel::FilterModelRoles + 1,

        /// Returns the current sort order.
        SortOrderRole                  = ShowfotoImageModel::FilterModelRoles + 2,

        /// Returns the format of the index which is used for category.
        CategoryFormatRole             = ShowfotoImageModel::FilterModelRoles + 3,

        /// Returns true if the given showfoto item is a group leader, and the group is opened.
        //TODO: GroupIsOpenRole        = ShowfotoImageModel::FilterModelRoles + 4
        ShowfotoFilterModelPointerRole = ShowfotoImageModel::FilterModelRoles + 50
    };

public:

    explicit ShowfotoFilterModel(QObject* const parent = 0);
    ~ShowfotoFilterModel();

    ShowfotoItemSortSettings showfotoItemSortSettings() const;

    void setShowfotoItemSortSettings(const ShowfotoItemSortSettings& sorter);

    /// Enables sending ShowfotoItemInfosAdded and ShowfotoItemInfosAboutToBeRemoved.
    void setSendShowfotoItemInfoSignals(bool sendSignals);

    //TODO: Implement grouping in Showfoto tool.
    //bool isGroupOpen(qlonglong group) const;
    //bool isAllGroupsOpen() const;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual ShowfotoFilterModel* showfotoFilterModel()                          const;

public Q_SLOTS:

    void setCategorizationMode(ShowfotoItemSortSettings::CategorizationMode mode);
    void setSortRole(ShowfotoItemSortSettings::SortRole role);
    void setSortOrder(ShowfotoItemSortSettings::SortOrder order);

    //TODO: Implement grouping in Showfoto tool.
    //void setGroupOpen(qlonglong group, bool open);
    //void toggleGroupOpen(qlonglong group);
    //void setAllGroupsOpen(bool open);

    /** Changes the current image filter settings and refilters. */
    //TODO: Implement filtering in Showfoto tool.
    //virtual void setImageFilterSettings(const ImageFilterSettings& settings);

    /** Changes the current image sort settings and resorts. */
    //TODO: virtual void setImageSortSettings(const ImageSortSettings& settings);

Q_SIGNALS:

    /** These signals need to be explicitly enabled with setSendImageInfoSignals().
     */
    void showfotoItemInfosAdded(const QList<ShowfotoItemInfo>& infos);
    void showfotoItemInfosAboutToBeRemoved(const QList<ShowfotoItemInfo>& infos);

protected Q_SLOTS:

    void slotRowsInserted(const QModelIndex& parent, int start, int end);
    void slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);

public:

    // Declared as public because of use in sub-classes.
    class ShowfotoFilterModelPrivate;

protected:

    ShowfotoFilterModelPrivate* const d_ptr;

protected:

    virtual void setDirectSourceShowfotoModel(ShowfotoImageModel* const sourceModel);

    //TODO: virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

    virtual int compareCategories(const QModelIndex& left, const QModelIndex& right) const;
    virtual bool subSortLessThan(const QModelIndex& left, const QModelIndex& right) const;

    /** Reimplement to customize category sorting,
     *  Return negative if category of left < category right,
     *  Return 0 if left and right are in the same category, else return positive.
     */
    virtual int compareInfosCategories(const ShowfotoItemInfo& left, const ShowfotoItemInfo& right) const;

    /** Reimplement to customize sorting. Do not take categories into account here.
     */
    virtual bool infosLessThan(const ShowfotoItemInfo& left, const ShowfotoItemInfo& right) const;

    /** Returns a unique identifier for the category if info. The string need not be for user display.
     */
    virtual QString categoryIdentifier(const ShowfotoItemInfo& info) const;

private:

    Q_DECLARE_PRIVATE(ShowfotoFilterModel)
};

// -----------------------------------------------------------------------------------------------------

class NoDuplicatesShowfotoFilterModel : public ShowfotoSortFilterModel
{
    Q_OBJECT

public:

    explicit NoDuplicatesShowfotoFilterModel(QObject* const parent = 0);

protected:

    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
};

} // namespace ShowFoto

Q_DECLARE_METATYPE(ShowFoto::ShowfotoFilterModel*)

#endif // SHOWFOTOFILTERMODEL_H

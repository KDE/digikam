/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Qt item model for database entries
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C)      2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C)      2011 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C)      2014 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#ifndef IMAGEFILTERMODEL_H
#define IMAGEFILTERMODEL_H

// Local includes

#include "dcategorizedsortfilterproxymodel.h"
#include "textfilter.h"
#include "imagefiltersettings.h"
#include "imagemodel.h"
#include "imagesortsettings.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageChangeset;
class ImageFilterModel;
class ImageTagChangeset;

class DIGIKAM_DATABASE_EXPORT ImageFilterModelPrepareHook
{
public:

    virtual ~ImageFilterModelPrepareHook() {};
    virtual void prepare(const QVector<ImageInfo>& infos) = 0;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ImageSortFilterModel : public DCategorizedSortFilterProxyModel
{
    Q_OBJECT

public:

    explicit ImageSortFilterModel(QObject* parent = 0);

    void        setSourceImageModel(ImageModel* model);
    ImageModel* sourceImageModel() const;

    void                  setSourceFilterModel(ImageSortFilterModel* model);
    ImageSortFilterModel* sourceFilterModel() const;

    QModelIndex mapToSourceImageModel(const QModelIndex& index) const;
    QModelIndex mapFromSourceImageModel(const QModelIndex& imagemodel_index) const;
    QModelIndex mapFromDirectSourceToSourceImageModel(const QModelIndex& sourceModel_index) const;

    /// Convenience methods mapped to ImageModel.
    /// Mentioned indexes returned come from the source image model.
    QList<QModelIndex> mapListToSource(const QList<QModelIndex>& indexes) const;
    QList<QModelIndex> mapListFromSource(const QList<QModelIndex>& sourceIndexes) const;

    ImageInfo        imageInfo(const QModelIndex& index) const;
    qlonglong        imageId(const QModelIndex& index) const;
    QList<ImageInfo> imageInfos(const QList<QModelIndex>& indexes) const;
    QList<qlonglong> imageIds(const QList<QModelIndex>& indexes) const;

    QModelIndex indexForPath(const QString& filePath) const;
    QModelIndex indexForImageInfo(const ImageInfo& info) const;
    QModelIndex indexForImageId(qlonglong id) const;

    /** Returns a list of all image infos, sorted according to this model.
     *  If you do not need a sorted list, use ImageModel's imageInfos() method.
     */
    QList<ImageInfo> imageInfosSorted() const;

    /// Returns this, any chained ImageFilterModel, or 0.
    virtual ImageFilterModel* imageFilterModel() const;

protected:

    /// Reimplement if needed. Called only when model shall be set as (direct) sourceModel.
    virtual void setDirectSourceImageModel(ImageModel* model);

    // made protected
    virtual void setSourceModel(QAbstractItemModel* model);

protected:

    ImageSortFilterModel* m_chainedModel;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ImageFilterModel : public ImageSortFilterModel
{
    Q_OBJECT

public:

    enum ImageFilterModelRoles
    {
        /// Returns the current categorization mode
        CategorizationModeRole      = ImageModel::FilterModelRoles + 1,
        /// Returns the current sort order
        SortOrderRole               = ImageModel::FilterModelRoles + 2,
        // / Returns the number of items in the index' category
        //CategoryCountRole         = ImageModel::FilterModelRoles + 3,
        /// Returns the id of the PAlbum of the index which is used for category
        CategoryAlbumIdRole         = ImageModel::FilterModelRoles + 3,
        /// Returns the format of the index which is used for category
        CategoryFormatRole          = ImageModel::FilterModelRoles + 4,
        /// Returns true if the given image is a group leader, and the group is opened
        GroupIsOpenRole             = ImageModel::FilterModelRoles + 5,
        ImageFilterModelPointerRole = ImageModel::FilterModelRoles + 50
    };

public:

    explicit ImageFilterModel(QObject* parent = 0);
    ~ImageFilterModel();

    /** Add a hook to get added images for preparation tasks before they are added in the model */
    void addPrepareHook(ImageFilterModelPrepareHook* hook);
    void removePrepareHook(ImageFilterModelPrepareHook* hook);

    /** Returns a set of DatabaseFields suggested to set as watch flags on the source ImageModel.
     *  The contained flags will be those that this model can sort or filter by. */
    DatabaseFields::Set suggestedWatchFlags() const;

    ImageFilterSettings        imageFilterSettings() const;
    VersionImageFilterSettings versionImageFilterSettings() const;
    GroupImageFilterSettings   groupImageFilterSettings() const;
    ImageSortSettings          imageSortSettings() const;

    // group is identified by the id of its group leader
    bool isGroupOpen(qlonglong group) const;
    bool isAllGroupsOpen() const;

    /// Enables sending imageInfosAdded and imageInfosAboutToBeRemoved
    void setSendImageInfoSignals(bool sendSignals);

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual ImageFilterModel* imageFilterModel() const;

public Q_SLOTS:

    /** Changes the current version image filter settings and refilters. */
    void setVersionImageFilterSettings(const VersionImageFilterSettings& settings);

    /** Changes the current version image filter settings and refilters. */
    void setGroupImageFilterSettings(const GroupImageFilterSettings& settings);

    /** Adjust the current ImageFilterSettings.
     *  Equivalent to retrieving the current filter settings, adjusting the parameter
     *  and calling setImageFilterSettings.
     *  Provided for convenience.
     *  It is encouraged to use setImageFilterSettings if you change more than one
     *  parameter at a time.
     */
    void setDayFilter(const QList<QDateTime>& days);
    void setTagFilter(const QList<int>& includedTags, const QList<int>& excludedTags,
                      ImageFilterSettings::MatchingCondition matchingCond, bool showUnTagged,
                      const QList<int>& clTagIds, const QList<int>& plTagIds);
    void setRatingFilter(int rating, ImageFilterSettings::RatingCondition ratingCond, bool isUnratedExcluded);
    void setMimeTypeFilter(int mimeTypeFilter);
    void setGeolocationFilter(const ImageFilterSettings::GeolocationCondition& condition);
    void setTextFilter(const SearchTextFilterSettings& settings);

    void setCategorizationMode(ImageSortSettings::CategorizationMode mode);
    void setCategorizationSortOrder(ImageSortSettings::SortOrder order);
    void setSortRole(ImageSortSettings::SortRole role);
    void setSortOrder(ImageSortSettings::SortOrder order);
    void setStringTypeNatural(bool natural);
    void setUrlWhitelist(const QList<QUrl> urlList, const QString& id);
    void setIdWhitelist(const QList<qlonglong>& idList, const QString& id);

    void setVersionManagerSettings(const VersionManagerSettings& settings);
    void setExceptionList(const QList<qlonglong>& idlist, const QString& id);

    void setGroupOpen(qlonglong group, bool open);
    void toggleGroupOpen(qlonglong group);
    void setAllGroupsOpen(bool open);

    /** Changes the current image filter settings and refilters. */
    virtual void setImageFilterSettings(const ImageFilterSettings& settings);

    /** Changes the current image sort settings and resorts. */
    virtual void setImageSortSettings(const ImageSortSettings& settings);

Q_SIGNALS:

    /// Signals that the set filter matches at least one index
    void filterMatches(bool matches);

    /** Signals that the set text filter matches at least one entry.
        If no text filter is set, this signal is emitted
        with 'false' when filterMatches() is emitted.
     */
    void filterMatchesForText(bool matchesByText);

    /** Emitted when the filter settings have been changed
        (the model may not yet have been updated)
     */
    void filterSettingsChanged(const ImageFilterSettings& settings);

    /** These signals need to be explicitly enabled with setSendImageInfoSignals()
     */
    void imageInfosAdded(const QList<ImageInfo>& infos);
    void imageInfosAboutToBeRemoved(const QList<ImageInfo>& infos);

public:

    // Declared as public because of use in sub-classes.
    class ImageFilterModelPrivate;

protected:

    ImageFilterModelPrivate* const d_ptr;

protected:

    ImageFilterModel(ImageFilterModelPrivate& dd, QObject* parent);

    virtual void setDirectSourceImageModel(ImageModel* model);

    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

    virtual int  compareCategories(const QModelIndex& left, const QModelIndex& right) const;
    virtual bool subSortLessThan(const QModelIndex& left, const QModelIndex& right) const;
    //virtual int  categoryCount(const ImageInfo& info) const;

    /** Reimplement to customize category sorting,
     *  Return negative if category of left < category right,
     *  Return 0 if left and right are in the same category, else return positive.
     */
    virtual int compareInfosCategories(const ImageInfo& left, const ImageInfo& right) const;

    /** Reimplement to customize sorting. Do not take categories into account here.
     */
    virtual bool infosLessThan(const ImageInfo& left, const ImageInfo& right) const;

    /** Returns a unique identifier for the category if info. The string need not be for user display.
     */
    virtual QString categoryIdentifier(const ImageInfo& info) const;

protected Q_SLOTS:

    void slotModelReset();
    void slotUpdateFilter();

    void slotImageTagChange(const ImageTagChangeset& changeset);
    void slotImageChange(const ImageChangeset& changeset);

    void slotRowsInserted(const QModelIndex& parent, int start, int end);
    void slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);

private:

    Q_DECLARE_PRIVATE(ImageFilterModel)
};

// -----------------------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT NoDuplicatesImageFilterModel : public ImageSortFilterModel
{
    Q_OBJECT

public:

    explicit NoDuplicatesImageFilterModel(QObject* parent = 0);

protected:

    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ImageFilterModel*)

#endif // IMAGEMODEL_H

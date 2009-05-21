/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Qt item model for database entries
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

// KDE includes

#include "kcategorizedsortfilterproxymodel.h"

// Local includes

#include "imagefiltersettings.h"
#include "imagemodel.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageChangeset;
class ImageTagChangeset;
class ImageFilterModelPrivate;

class DIGIKAM_DATABASE_EXPORT ImageFilterModel : public KCategorizedSortFilterProxyModel
{
    Q_OBJECT

public:

    enum ImageFilterModelRoles
    {
        /// Returns the current categorization mode
        CategorizationModeRole = ImageModel::FilterModelRoles + 1,
        /// Returns the current sort order
        SortOrderRole          = ImageModel::FilterModelRoles + 2,
        /// Returns the number of items in the index' category
        CategoryCountRole      = ImageModel::FilterModelRoles + 3,
        /// Returns the id of the PAlbum of the index which is used for category
        CategoryAlbumIdRole    = ImageModel::FilterModelRoles + 4,
        /// Returns the format of the PAlbum of the index which is used for category
        CategoryFormatRole     = ImageModel::FilterModelRoles + 5,
        ImageFilterModelPointerRole  = ImageModel::FilterModelRoles + 50
    };

    ImageFilterModel(QObject *parent = 0);
    ~ImageFilterModel();

    /** This filter model is for use with ImageModel source models only. */
    void setSourceImageModel(ImageModel* model);

    ImageModel *sourceModel() const;

    /// Convenience methods mapped to ImageModel
    QList<QModelIndex> mapListToSource(const QList<QModelIndex>& indexes) const;
    QList<QModelIndex> mapListFromSource(const QList<QModelIndex>& sourceIndexes) const;
    ImageInfo imageInfo(const QModelIndex& index) const;
    qlonglong imageId(const QModelIndex& index) const;
    QList<ImageInfo> imageInfos(const QList<QModelIndex>& indexes) const;
    QList<qlonglong> imageIds(const QList<QModelIndex>& indexes) const;
    QModelIndex indexForPath(const QString& filePath) const;
    QModelIndex indexForImageInfo(const ImageInfo& info) const;
    QModelIndex indexForImageId(qlonglong id) const;
    /** Returns a list of all image infos, sorted according to this model.
     *  If you do not need a sorted list, use ImageModel's imageInfos() method. */
    QList<ImageInfo> imageInfosSorted() const;
    /** Returns a set of DatabaseFields suggested to set as watch flags on the source ImageModel.
     *  The contained flags will be those that this model can sort or filter by. */
    DatabaseFields::Set suggestedWatchFlags() const;

    ImageFilterSettings imageFilterSettings() const;

    enum CategorizationMode
    {
        NoCategories, /// categorization switched off
        OneCategory, /// all items in one global category
        CategoryByAlbum,
        CategoryByFormat
    };

    enum SortOrder
    {
        SortByFileName,
        SortByFilePath,
        SortByFileSize,
        SortByModificationDate,
        SortByCreationDate,
        SortByRating,
        SortByImageSize // pixel number
    };

    CategorizationMode categorizationMode() const;
    SortOrder sortOrder() const;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

public Q_SLOTS:

    /** Changes the current image filter settings and refilters. */
    virtual void setImageFilterSettings(const ImageFilterSettings& settings);

    /** Adjust the current ImageFilterSettings.
     *  Equivalent to retrieving the current filter settings, adjusting the parameter
     *  and calling setImageFilterSettings.
     *  Provided for convenience.
     *  It is encouraged to use setImageFilterSettings if you change more than one
     *  parameter at a time.
     */
    void setDayFilter(const QList<QDateTime>& days);
    void setTagFilter(const QList<int>& tags, ImageFilterSettings::MatchingCondition matchingCond,
                      bool showUnTagged=false);
    void setRatingFilter(int rating, ImageFilterSettings::RatingCondition ratingCond);
    void setMimeTypeFilter(int mimeTypeFilter);
    void setTextFilter(const SearchTextSettings& settings);

    void setCategorizationMode(CategorizationMode mode);
    void setSortOrder(SortOrder order);

Q_SIGNALS:

    /// Signals that the set filter matches at least one index
    void filterMatches(bool matches);
    /** Signals that the set text filter matches at least one entry.
        If no text filter is set, this signal is emitted
        with 'false' when filterMatches() is emitted. */
    void filterMatchesForText(bool matchesByText);

    /** Emitted when the filter settings have been changed
        (the model may not yet have been updated) */
    void filterSettingsChanged(const ImageFilterSettings &settings);

protected:

    virtual void setSourceModel(QAbstractItemModel* model);

    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

    virtual int  compareCategories(const QModelIndex& left, const QModelIndex& right) const;
    virtual bool subSortLessThan(const QModelIndex& left, const QModelIndex& right) const;
    virtual int  categoryCount(const ImageInfo& info) const;

    /** Reimplement to customize category sorting,
     *  Return negative if category of left < category right,
     *  Return 0 if left and right are in the same category, else return positive. */
    virtual int compareInfosCategories(const ImageInfo& left, const ImageInfo& right) const;
    /** Reimplement to customize sorting. Do not take categories into account here. */
    virtual bool infosLessThan(const ImageInfo& left, const ImageInfo& right) const;
    /** Returns a unique identifier for the category if info. The string need not be for user display. */
    virtual QString categoryIdentifier(const ImageInfo& info) const;

    ImageFilterModelPrivate *const d_ptr;
    ImageFilterModel(ImageFilterModelPrivate& dd, QObject *parent);

protected Q_SLOTS:

    void slotModelReset();
    void slotUpdateFilter();

    void slotImageTagChange(const ImageTagChangeset& changeset);
    void slotImageChange(const ImageChangeset& changeset);

private:

    Q_DECLARE_PRIVATE(ImageFilterModel)
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ImageFilterModel*)

#endif // IMAGEMODEL_H

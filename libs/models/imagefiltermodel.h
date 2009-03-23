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

// Qt includes.

#include <QSortFilterProxyModel>

// Local includes.

#include "imagefiltersettings.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageModel;
class ImageChangeset;
class ImageTagChangeset;
class ImageFilterModelPrivate;

class DIGIKAM_EXPORT ImageFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    ImageFilterModel(QObject *parent = 0);
    ~ImageFilterModel();

    /** This filter model is for use with ImageModel source models only. */
    void setSourceImageModel(ImageModel* model);

    /** Changes the current image filter settings and refilters. */
    virtual void setImageFilterSettings(const ImageFilterSettings &settings);
    ImageFilterSettings imageFilterSettings() const;

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

protected:

    virtual void setSourceModel(QAbstractItemModel* model);

    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

    ImageFilterModelPrivate *const d_ptr;
    ImageFilterModel(ImageFilterModelPrivate &dd, QObject *parent);

protected Q_SLOTS:

    void slotModelReset();
    void slotUpdateFilter();

    void slotImageTagChange(const ImageTagChangeset &changeset);
    void slotImageChange(const ImageChangeset &changeset);

private:

    Q_DECLARE_PRIVATE(ImageFilterModel)
};


}

#endif // IMAGEMODEL_H

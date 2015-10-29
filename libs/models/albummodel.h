/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-22
 * Description : Qt Model for Albums
 *
 * Copyright (C) 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef ALBUMMODEL_H
#define ALBUMMODEL_H

// Qt includes

#include <QDateTime>

// Local includes

#include "abstractalbummodel.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT AlbumModel : public AbstractCheckableAlbumModel
{
public:

    /// Create a model containing all physical albums
    explicit AlbumModel(RootAlbumBehavior rootBehavior = IncludeRootAlbum, QObject* const parent = 0);
    virtual ~AlbumModel();

    PAlbum* albumForIndex(const QModelIndex& index) const;

protected:

    virtual QVariant decorationRoleData(Album* a) const;
    virtual Album*   albumForId(int id) const;
};

// ------------------------------------------------------------------

class DIGIKAM_EXPORT TagModel : public AbstractCheckableAlbumModel
{
public:

    /**
     * If setShowCount() is enabled:
     * Per default, normal tag counts are shown, setUseTagCount().
     * You can switch to showing face count.
     */
    enum TagCountMode
    {
        NormalTagCount,
        FaceTagCount
    };

public:

    /// Create a model containing all tags
    explicit TagModel(RootAlbumBehavior rootBehavior = IncludeRootAlbum, QObject* const parent = 0);

    TAlbum* albumForIndex(const QModelIndex& index) const;
    void    setColumnHeader(const QString& header);
    void    setTagCount(TagCountMode mode);

protected:

    virtual QVariant decorationRoleData(Album* a) const;
    virtual Album*   albumForId(int id) const;
};

// ------------------------------------------------------------------

class DIGIKAM_EXPORT SearchModel : public AbstractCheckableAlbumModel
{
    Q_OBJECT

public:

    /// Create a model containing searches
    explicit SearchModel(QObject* const parent = 0);

    SAlbum* albumForIndex(const QModelIndex& index) const;

    /** Set a hash of internal names (key) that shall be replaced by a user-visible string (value).
     *  This affects Qt::DisplayRole and AlbumTitleRole. */
    void setReplaceNames(const QHash<QString, QString>& replaceNames);
    void addReplaceName(const QString& technicalName, const QString& userVisibleName);

    /** Set pixmaps for the DecorationRole */
    void setPixmapForNormalSearches(const QPixmap& pix);
    void setDefaultPixmap(const QPixmap& pix);
    void setPixmapForTimelineSearches(const QPixmap& pix);
    void setPixmapForHaarSearches(const QPixmap& pix);
    void setPixmapForMapSearches(const QPixmap& pix);
    void setPixmapForDuplicatesSearches(const QPixmap& pix);

protected:

    virtual QVariant albumData(Album* a, int role) const;
    virtual Album*   albumForId(int id) const;

private Q_SLOTS:

    void albumSettingsChanged();

protected:

    QHash<int, QPixmap>     m_pixmaps;
    QHash<QString, QString> m_replaceNames;
};

// ------------------------------------------------------------------

/**
 * A model for date based albums.
 */
class DIGIKAM_EXPORT DateAlbumModel : public AbstractCountingAlbumModel
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent parent for Qt's parent child mechanism
     */
    explicit DateAlbumModel(QObject* const parent = 0);

    DAlbum* albumForIndex(const QModelIndex& index) const;

    /**
     * Finds an album index based on a date. The given date is therefore
     * normalized to year-month-form. The day is ignored. This means the
     * returned index always points to a month DAlbum.
     *
     * @param date date to search for (year and month)
     * @return model index corresponding to the album with the given date or an
     *         empty index if not found
     */
    QModelIndex monthIndexForDate(const QDate& date) const;

    /** Set pixmaps for the DecorationRole */
    void setPixmaps(const QPixmap& forYearAlbums, const QPixmap& forMonthAlbums);

public Q_SLOTS:

    void setYearMonthMap(const QMap<YearMonth, int>& yearMonthMap);

protected:

    virtual QString  albumName(Album* a) const;
    virtual QVariant decorationRoleData(Album* a) const;
    virtual QVariant sortRoleData(Album* a) const;
    virtual Album*   albumForId(int id) const;

protected:

    QPixmap m_yearPixmap;
    QPixmap m_monthPixmap;
};

} // namespace Digikam

#endif // ALBUMMODEL_H

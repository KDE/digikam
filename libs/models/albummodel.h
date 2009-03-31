/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-22
 * Description : Qt Model for Albums
 *
 * Copyright (C) 2008-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Local includes.

#include "abstractalbummodel.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_ALBUMMODELS_EXPORT AlbumModel : public AbstractCheckableAlbumModel
{
public:

    /// Create a model containing all physical albums
    AlbumModel(RootAlbumBehavior rootBehavior = IncludeRootAlbum, QObject *parent = 0);

    PAlbum *albumForIndex(const QModelIndex &index) const;

protected:

    virtual QVariant decorationRole(Album *a) const;
    virtual Album* albumForId(int id) const;
};

// ------------------------------------------------------------------

class DIGIKAM_ALBUMMODELS_EXPORT TagModel : public AbstractCheckableAlbumModel
{
public:

    /// Create a model containing all tags
    TagModel(RootAlbumBehavior rootBehavior = IncludeRootAlbum, QObject *parent = 0);

    TAlbum *albumForIndex(const QModelIndex &index) const;

protected:

    virtual QVariant decorationRole(Album *a) const;
    virtual Album* albumForId(int id) const;
};

// ------------------------------------------------------------------

class DIGIKAM_ALBUMMODELS_EXPORT SearchModel : public AbstractSpecificAlbumModel
{
public:

    /// Create a model containing searches
    SearchModel(QObject *parent = 0);

    SAlbum *albumForIndex(const QModelIndex &index) const;

    /** Set the DatabaseSearch::Type. */
    void setSearchType(DatabaseSearch::Type type);
    void listNormalSearches();
    void listAllSearches();

    /** Set a hash of internal names (key) that shall be replaced by a user-visible string (value) */
    void setReplaceNames(QHash<QString, QString> replaceNames);

    /** Set a pixmap for the DecorationRole */
    void setPixmap(const QPixmap &pix);

protected:

    virtual QVariant albumData(Album *a, int role) const;
    virtual bool filterAlbum(Album *album) const;

protected:

    QPixmap m_pixmap;
    int     m_searchType;
    QHash<QString, QString> m_replaceNames;
};

// ------------------------------------------------------------------

class DIGIKAM_ALBUMMODELS_EXPORT DateAlbumModel : public AbstractCountingAlbumModel
{
    Q_OBJECT

public:

    /// A model for date based albums
    DateAlbumModel(QObject *parent = 0);

    DAlbum *albumForIndex(const QModelIndex &index) const;

    /** Set pixmaps for the DecorationRole */
    void setPixmaps(const QPixmap &forYearAlbums, const QPixmap &forMonthAlbums);

public Q_SLOTS:

    void setYearMonthMap(const QMap<YearMonth, int>& yearMonthMap);

protected:

    virtual QString  albumName(Album *a) const;
    virtual QVariant decorationRole(Album *a) const;
    virtual Album* albumForId(int id) const;

protected:

    QPixmap m_yearPixmap;
    QPixmap m_monthPixmap;
};

} // namespace Digikam

#endif // ALBUMMODEL_H

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-19
 * Description : Handling of database specific URLs
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DATABASEURL_H
#define DATABASEURL_H

// Qt includes

#include <QDateTime>
#include <QList>

// KDE includes

#include <kurl.h>

// Local includes

#include "digikam_export.h"
#include "databaseparameters.h"
#include "databaseaccess.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT DatabaseUrl : public KUrl
{
public:

    /**
      * This class shall facilitate the usage of
      * digikamalbums:/, digikamtags:/, digikamdates:/ and digikamsearch: URLs.
      * It provides functions to set and get the parameters stored in such a URL.
      * (with the exception of the search parameters in a search URL, which
      *  are out of the scope of this class.)
      */

    /**
      * Create a digikamalbums:/ URL from a file:// URL.
      * The file URL can point to a file or a directory (an album in this case).
      * The additional information stored in the URL need to be supplied as well:
      * - The album root in which the entity pointed to is stored.
      *   This is the left part of the file URL.
      *   (if the file is "/media/fotos/Summer 2007/001.jpg", the album root may be "/media/fotos")
      * - The parameters of the database that is referenced
      */
    static DatabaseUrl fromFileUrl(const KUrl& fileUrl,
                                   const KUrl& albumRoot,
                                   int   albumRootId,
                                   const DatabaseParameters& parameters = DatabaseAccess::parameters());

    static DatabaseUrl fromFileUrl(const KUrl& fileUrl,
                                   const KUrl& albumRoot,
                                   const DatabaseParameters& parameters = DatabaseAccess::parameters());

    /**
     * Create a digikamalbums:/ url from an album name and an image in this album.
     * If name is empty, the album is referenced.
     * Other parameters as above.
     */
    static DatabaseUrl fromAlbumAndName(const QString& name,
                                        const QString& album,
                                        const KUrl& albumRoot,
                                        int   albumRootId,
                                        const DatabaseParameters& parameters = DatabaseAccess::parameters());

    static DatabaseUrl fromAlbumAndName(const QString& name,
                                        const QString& album,
                                        const KUrl& albumRoot,
                                        const DatabaseParameters& parameters = DatabaseAccess::parameters());

    /**
     * Create an empty digikamalbums:/ url
     */
    static DatabaseUrl albumUrl(const DatabaseParameters& parameters = DatabaseAccess::parameters());

    /**
     * Create a digikamtags:/ url from a list of tag IDs, where this list is the tag hierarchy
     * of the referenced tag, with the topmost parent first, and the tag last in the list.
     * An empty list references the root tag.
     */
    static DatabaseUrl fromTagIds(const QList<int>& tagIds,
                                  const DatabaseParameters& parameters = DatabaseAccess::parameters());

    /**
     * Create an empty digikamdates:/ url
     */
    static DatabaseUrl dateUrl(const DatabaseParameters& parameters = DatabaseAccess::parameters());

    /**
     * Create a digikamdates:/ url for the month of the given date.
     * (The whole month of the given date will included in the referenced time span)
     */
    static DatabaseUrl fromDateForMonth(const QDate& date,
                                        const DatabaseParameters& parameters = DatabaseAccess::parameters());

    /**
     * Create a digikamdates:/ url for the year of the given date.
     * (The whole year of the given date will included in the referenced time span)
     */
    static DatabaseUrl fromDateForYear(const QDate& date,
                                       const DatabaseParameters& parameters = DatabaseAccess::parameters());

    /**
     * Create a digikamdates:/ url for a specified time span which begin with the
     * start date (inclusive) and ends before the end date (exclusive).
     * To cover the whole year of 1984, you would pass 1/1/1984 and 1/1/1985.
     */
    static DatabaseUrl fromDateRange(const QDate& startDate, const QDate& endDate,
                                     const DatabaseParameters& parameters = DatabaseAccess::parameters());

    /**
     * Create an empty digikammapimages:/ url
     */

    static DatabaseUrl mapImagesUrl(const DatabaseParameters& parameters = DatabaseAccess::parameters());
    static DatabaseUrl fromAreaRange(const qreal lat1, const qreal lng1, const qreal lat2, const qreal lng2, const DatabaseParameters& parameters = DatabaseAccess::parameters());
    /**
     * Create a digikamsearch: URL for the search with the given id.
     */
    static DatabaseUrl searchUrl(int searchId,
                                 const DatabaseParameters& parameters = DatabaseAccess::parameters());
    /**
      * Create a DatabaseUrl object from a KUrl, to retrieve the information stored
      */
    DatabaseUrl(const KUrl& digikamUrl);

    /**
     * Create an invalid database URL
     */
    DatabaseUrl();

    /**
     * These test for the protocol of this URL.
     * The protocol string is of course available via protocol().
     */
    bool isAlbumUrl() const;
    bool isTagUrl() const;
    bool isDateUrl() const;
    bool isSearchUrl() const;
    bool isMapImagesUrl() const;
    /**
     * Returns the DatabaseParameters stored in this URL.
     * Applicable to all protocols.
     */
    DatabaseParameters parameters() const;
    /**
     * Change the database parameters stored in this URL
     * Applicable to all protocols.
     */
    void setParameters(const DatabaseParameters& parameters);

    /**
     * The following methods are only applicable for a certain protocol each.
     * If the URL has another protocol, the return value of these methods is undefined.
     */

    /// Album URL

    /** Returns the album root URL of the file or album referenced by this URL
     *  In the example above, this is "file://media/fotos"
     */
    KUrl albumRoot() const;

    /** Returns the album root path of the file or album referenced by this URL
     *  In the example above, this is "/media/fotos"
     */
    QString albumRootPath() const;

    /** Returns the album root id */
    int albumRootId() const;

    /** Returns the album: This is the directory hierarchy below the album root.
     *  In the example above, the album is "/Summer 2007"
     */
    QString album() const;

    /**
     * Returns the file name. In the example above, this is "001.jpg"
     */
    QString name() const;

    /**
     * Converts this digikamalbums:// URL to a file:// URL
     */
    KUrl fileUrl() const;

    /// Tag URL

    /**
     * Returns the tag ID, or -1 if the root tag is referenced
     */
    int tagId() const;

    /**
     * Returns the tag ids of all tags in the tag path of this tag,
     * the topmost tag in the hierarchy first.
     */
    QList<int> tagIds() const;

    /// Date URL

    /**
     * Return the referenced start date (included in the referenced span)
     */
    QDate startDate() const;

    /**
     * Return the referenced end date (excluded from the referenced span)
     */
    QDate endDate() const;

    /// MapImages URL

    /**
     * Returns the coordinates surrounding the map area.
     * Returns true if the string to number conversion was ok.
     */

    bool areaCoordinates(double* lat1, double* lat2, double* lon1, double* lon2) const;

    /// Search URL

    /**
     * Return the id of the search.
     */
    int searchId() const;

    DatabaseUrl(const DatabaseUrl& url);

    DatabaseUrl& operator=(const KUrl& digikamalbumsUrl);
    DatabaseUrl& operator=(const DatabaseUrl& url);

    bool operator==(const KUrl& digikamalbumsUrl) const;
};

}  // namespace Digikam

#endif // DATABASEURL_H

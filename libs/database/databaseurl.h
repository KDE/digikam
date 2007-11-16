/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-19
 * Description : Handling of database specific URLs
 *
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes.

#include <QDateTime>
#include <QList>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "digikam_export.h"
#include "databaseparameters.h"
#include "databaseaccess.h"

namespace Digikam
{

class DIGIKAM_EXPORT DatabaseUrl : public KUrl
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
    static DatabaseUrl fromFileUrl(const KUrl &fileUrl,
                                   const KUrl &albumRoot,
                                   int   albumRootId,
                                   const DatabaseParameters &parameters = DatabaseAccess::parameters());

    static DatabaseUrl fromFileUrl(const KUrl &fileUrl,
                                   const KUrl &albumRoot,
                                   const DatabaseParameters &parameters = DatabaseAccess::parameters());

    /**
     * Create a digikamalbums:/ url from an album name and an image in this album.
     * If name is empty, the album is referenced.
     * Other parameters as above.
     */
    static DatabaseUrl fromAlbumAndName(const QString &name,
                                        const QString &album,
                                        const KUrl &albumRoot,
                                        int   albumRootId,
                                        const DatabaseParameters &parameters = DatabaseAccess::parameters());

    static DatabaseUrl fromAlbumAndName(const QString &name,
                                        const QString &album,
                                        const KUrl &albumRoot,
                                        const DatabaseParameters &parameters = DatabaseAccess::parameters());

    /**
     * Create a digikamtags:/ url from a list of tag IDs, where this list is the tag hierarchy
     * of the referenced tag, with the topmost parent first, and the tag last in the list.
     * An empty list references the root tag.
     */
    static DatabaseUrl fromTagIds(const QList<int> &tagIds,
                                  const DatabaseParameters &parameters = DatabaseAccess::parameters());

    /**
     * Create a digikamdates:/ url from the given date.
     */
    static DatabaseUrl fromDate(const QDate &date,
                                const DatabaseParameters &parameters = DatabaseAccess::parameters());

    /**
     * Create a digikamsearch: URL.
     * It is assumed that the URL provided is a digikamsearch: URL
     * The database parameters will be added to the provided URL.
     * Populating the query items is out of the scope of this class.
     */
    static DatabaseUrl fromSearchUrl(const KUrl &searchURL,
                                     const DatabaseParameters &parameters = DatabaseAccess::parameters());
    /**
      * Create a DatabaseUrl object from a KUrl, to retrieve the information stored
      */
    DatabaseUrl(const KUrl &digikamUrl);

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

    /**
     * Returns the DatabaseParameters stored in this URL.
     * Applicable to all protocols.
     */
    DatabaseParameters parameters() const;
    /**
     * Change the database parameters stored in this URL
     * Applicable to all protocols.
     */
    void setParameters(const DatabaseParameters &parameters);

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
     * Return the referenced date
     */
    QDate date() const;

    /// Search URL

    /**
     * Return the search URL as provided to fromSearchUrl().
     * The returned URL has database information removed.
     */
    KUrl searchUrl() const;

    DatabaseUrl(const DatabaseUrl &url);

    DatabaseUrl &operator=(const KUrl &digikamalbumsUrl);
    DatabaseUrl &operator=(const DatabaseUrl &url);

    bool operator==(const KUrl &digikamalbumsUrl);

};

}  // namespace Digikam

#endif // DATABASEURL_H

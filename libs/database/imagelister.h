/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Listing information from database.
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2007-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGELISTER_H
#define IMAGELISTER_H

// Qt includes

#include <QString>

// KDE includes

#include <kio/job.h>

// Local includes

#include "digikam_export.h"
#include "imageinfo.h"
#include "imageinfolist.h"
#include "databaseaccess.h"
#include "databaseurl.h"
#include "imagelisterrecord.h"
#include "imagelisterreceiver.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT ImageLister
{
public:

    /**
     * Create a TransferJob for the "special" method of one of the database ioslaves,
     * referenced by the URL.
     * @param url url referencing the database ioslave
     * @param extraValue If -1, nothing is sent. If it takes another value,
     *                   this value will be sent as a second parameter.
     */
    static KIO::TransferJob* startListJob(const DatabaseUrl& url, int extraValue = -1);

public:

    ImageLister();
    ~ImageLister();

    /**
     * Adjust the setting if album or tags will be listed recursively (i.e. including subalbums / subtags)
     */
    void setRecursive(bool recursive);

    /**
     * Adjust the setting if images from collections that are currently not in the state "available"
     * will be included in the listing. Default: true.
     */
    void setListOnlyAvailable(bool listOnlyAvailable);

    /**
     * Allow the binary protocol of ImageListerRecord using an extra value.
     * Currently used for AreaRand and TagPropertySearch.
     */
    void setAllowExtraValues(bool useExtraValue);

    /**
     * Convenience method for Album, Tag and Date URLs, _not_ for Search URLs.
     */
    void list(ImageListerReceiver* const receiver, const DatabaseUrl& url);

    /**
      * List images in the Album (physical album) specified by albumRoot, album.
      * The results will be fed to the specified receiver.
      */
    void listAlbum(ImageListerReceiver* const receiver, int albumRootId, const QString& album);

    /**
     * List the images which have assigned the tags specified by tagIds
     * Updated to support multiple tags
     */
    void listTag(ImageListerReceiver* receiver, QList<int> tagIds);

    /**
     * List the images which have faces. An image with n faces will be listed n times.
     * FIXME: Obviously an ugly way. Should be trashed later in favor of a better method.
     */
    void listFaces(ImageListerReceiver* const receiver, int personId);

    /**
      * List those images whose date lies in the range beginning with startDate (inclusive)
      * and ending before endDate (exclusive).
      */
    void listDateRange(ImageListerReceiver* const receiver, const QDate& startDate, const QDate& endDate);

    /**
     * List the images whose coordinates are between coordinates contained in areaCoordinates(lat1, lat2, lng1, lng2)
     */
    void listAreaRange(ImageListerReceiver* const receiver, double lat1, double lat2, double lon1, double lon2);

    /**
     * Execute the search specified by search XML
     * @param receiver receiver for the searches
     * @param xml SearchXml describing the query
     * @param limit limit the count of the result set. If limit = 0, then no limit is set.
     */
    void listSearch(ImageListerReceiver* const receiver, const QString& xml, int limit = 0);

    /**
     * Execute the search specified by search XML describing a Tag Properties search.
     * Two special add-ons: Non-unique by image id; if enabled, uses the extended ImageRecord protocol
     * to pass the property value in the record's extraValue.
     * @param receiver receiver for the searches
     * @param xml SearchXml describing the query
     */
    void listImageTagPropertySearch(ImageListerReceiver* const receiver, const QString& xml);

    /**
     * Execute the search specified by search XML describing a Haar search
     * @param receiver receiver for the searches
     * @param xml SearchXml describing the query
     */
    void listHaarSearch(ImageListerReceiver* const receiver, const QString& xml);

    QString tagSearchXml(const DatabaseUrl&, const QString& type, bool includeChildTags) const;

private:

    void listFromIdList(ImageListerReceiver* const receiver, const QList<qlonglong>& imageIds);
    QSet<int> albumRootsToList() const;

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // IMAGELISTER_H

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Listing information from database.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#ifndef IMAGELISTER_H
#define IMAGELISTER_H

// Qt includes.

#include <QString>

// KDE includes.

#include <kio/job.h>

// Local includes.

#include "digikam_export.h"
#include "imageinfo.h"
#include "imageinfolist.h"
#include "databaseaccess.h"
#include "databaseurl.h"
#include "imagelisterrecord.h"
#include "imagelisterreceiver.h"

namespace Digikam
{

class DIGIKAM_EXPORT ImageLister
{
public:

    /**
     * Create a TransferJob for the "special" method of one of the database ioslaves,
     * referenced by the URL.
     * @param extraValue If -1, nothing is sent. If it takes another value,
     *                   this value will be sent as a second parameter.
     */
    static KIO::TransferJob *startListJob(const DatabaseUrl &url, int extraValue = -1);

    /**
     * Create a TransferJob for the scanning part of the "special" method of the albums ioslave,
     * referenced by the URL. Filter and extraValue will be added to the job data.
     */
    static KIO::TransferJob *startScanJob(const DatabaseUrl &url, const QString &filter, int extraValue);


    /**
     * Convenience method for Album, Tag and Date URLs, _not_ for Search URLs.
     */
    void list(ImageListerReceiver *receiver,
              const DatabaseUrl &url);

    /**
      * List images in the Album (physical album) specified by albumRoot, album (and albumid).
      * The results will be fed to the specified receiver.
      * @param filter The file format filter to use
      * @param getDimension retrieve dimension - slow!. If false, dimension will be QSize()
      */
    void listAlbum(ImageListerReceiver *receiver,
                   const QString &albumRoot, const QString &album);
    void listAlbum(ImageListerReceiver *receiver,
                   const QString &albumRoot, int albumRootId, const QString &album, int albumid);
    /**
     * List the images which have assigned the tag specified by tagId
     */
    void listTag(ImageListerReceiver *receiver, int tagId);
    /**
      * List those images whose date lies in the month specified by the date
      */
    void listMonth(ImageListerReceiver *receiver, const QDate &date);

    /**
     * Execute the search specified by a SQL expression
     * @param sqlConditionalExpression SQL code for the WHERE part of the query
     * @param boundValues SQL bound values used in the sql conditional expression
     * @param getSize stat the file size. If false, size will be 0
     * @param limit limit the count of the result set. If limit = 0, then no limit is set.
     */
    void listSearch(ImageListerReceiver *receiver,
                    const QString &sqlConditionalExpression,
                    const QList<QVariant> &boundValues,
                    int limit = 0);

    /**
     * Tool method to retrieve the dimension from a file. Not a database query, slow!
     */
    static QSize retrieveDimension(const QString &filePath);
};

}  // namespace Digikam

#endif // IMAGELISTER_H

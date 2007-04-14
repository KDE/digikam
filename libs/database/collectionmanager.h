/* ============================================================
 * Authors: Marcel Wiesweg
 * Date   : 2007-04-09
 * Description : Collection management
 * 
 * Copyright 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef COLLECTIONMANAGER_H
#define COLLECTIONMANAGER_H

// Qt includes

#include <qstring.h>
#include <qstringlist.h>

// KDE includes

#include <kurl.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class CollectionLocation;
class CollectionManagerPrivate;

class DIGIKAM_EXPORT CollectionManager
{
public:

    static CollectionManager *instance();
    static void cleanUp();

    //QList<CollectionLocation *> allLocations();
    //QList<CollectionLocation *> allAvailableLocations();
    QStringList allAvailableAlbumRootPaths();

    CollectionLocation *locationForAlbumRoot(KURL fileUrl);
    CollectionLocation *locationForAlbumRootPath(QString albumRootPath);

    KURL    albumRoot(KURL fileUrl);
    QString albumRootPath(KURL fileUrl);

    QString album(KURL fileUrl);

    /**
     * Returns just one album root, out of the list of available location,
     * the one that is most suitable to serve as a default, e.g.
     * to suggest as default place when the user wants to add files.
     */
    KURL oneAlbumRoot();
    QString oneAlbumRootPath();

private:

    CollectionManager();
    ~CollectionManager();
    static CollectionManager *m_instance;

    CollectionManagerPrivate *d;
};

}

#endif

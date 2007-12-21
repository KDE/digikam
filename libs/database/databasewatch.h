/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-23
 * Description : Keeping image properties in sync.
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

#ifndef DATABASEWATCH_H
#define DATABASEWATCH_H

// Qt includes

#include <QObject>

// KDE includes

// Local includes.

#include "digikam_export.h"
#include "databasechangesets.h"

namespace Digikam
{

class DIGIKAM_EXPORT DatabaseWatch : public QObject
{

    Q_OBJECT

    /**
     * This class notifies of changes in the database.
     * The context when these signals are emitted is important:
     * DatabaseAccess is locked when these signals are emitted,
     * but this allows direct notification immediately after the change.
     */

public:

    DatabaseWatch();

signals:

    /**
     * Notifies of an image-related change
     */
    void imageChange(ImageChangeset changeset);
    void imageTagChange(ImageTagChangeset changeset);
    void collectionImageChange(CollectionImageChangeset changeset);

protected:

    ~DatabaseWatch();

public:

    // --- internal ---

    void sendImageChange(ImageChangeset changeset);
    void sendImageTagChange(ImageTagChangeset changeset);
    void sendCollectionImageChange(CollectionImageChangeset changeset);
};

} // namespace Digikam

#endif // DATABASEATTRIBUTESWATCH_H

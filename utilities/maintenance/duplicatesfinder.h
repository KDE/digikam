/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-20
 * Description : Duplicates items finder.
 *
 * Copyright (C) 2012-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DUPLICATESFINDER_H
#define DUPLICATESFINDER_H

// Qt includes

#include <QString>

// KDE includes

#include <kjob.h>

// Local includes

#include "album.h"
#include "maintenancetool.h"

namespace Digikam
{

class DuplicatesFinder : public MaintenanceTool
{
    Q_OBJECT

public:

    /** Version to find all duplicates over a specific list to PAlbums and TAlbums
     */
    DuplicatesFinder(const AlbumList& albums, const AlbumList& tags, int similarity = 90, ProgressItem* const parent = 0);
    /** Version to find all duplicates over whole collections
     */
    explicit DuplicatesFinder(const int similarity = 90, ProgressItem* const parent = 0);
    ~DuplicatesFinder();

private Q_SLOTS:

    void slotStart();
    void slotDone();
    void slotCancel();
    void slotDuplicatesSearchTotalAmount(KJob*, KJob::Unit, qulonglong);
    void slotDuplicatesSearchProcessedAmount(KJob*, KJob::Unit, qulonglong);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* DUPLICATESFINDER_H */

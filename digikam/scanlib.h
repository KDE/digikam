/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-01
 * Description : scan pictures interface.
 * 
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007      by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#ifndef SCANLIB_H
#define SCANLIB_H

// Qt includes.

#include <QObject>
#include <QString>
#include <QMap>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "collectionscanner.h"
#include "digikam_export.h"

/** @file scanlib.h */

namespace Digikam
{

class DProgressDlg;

/** 
 * Class which is responsible for keeping the database in sync
 * with the disk. Scanlib is a library that takes care of scanning the
 * filesystem for new files and adds them in the database and checking
 * for missing info in the database so that it can be included: if date
 * is empty, it adds the exif or modification date (in that order) and
 * the comment to database. If the file is not present in the database,
 * make sure to add the file to the database and insert the date and
 * comments.
 */
class DIGIKAM_EXPORT ScanLib : public QObject
{
    Q_OBJECT

public:

    /** 
    * Constructor
    */
    ScanLib();

    /**
     * Destructor
     */
    ~ScanLib();

    /**
     * This will execute findFoldersWhichDoNotExist(),
     * findMissingItems() and updateItemsWithoutDate()
     * and deletes all items from the database after confirmation.
     */
    void startScan();

    /**
     * This checks if all albums in the database still existing
     * on the disk
     */
    void findFoldersWhichDoNotExist();

    /**
    * This calls allFiles with the albumPath.
    */
    void findMissingItems();

    /**
     * This queries the db for items that have no date
     * for each item found, storeItemInDatabase is called.
     */
    void updateItemsWithoutDate();

private slots:

    void slotTotalFilesToScan(int count);
    void slotStartScanningAlbum(const QString &albumRoot, const QString &album);
    void slotFinishedScanningAlbum(const QString &, const QString &, int filesScanned);
    void slotStartScanningAlbumRoot(const QString &albumRoot);
    void slotStartScanningForStaleAlbums();
    void slotStartScanningAlbumRoots();

private:

    /**
     * This will delete all items stored in m_filesToBeDeleted
     */
    void deleteStaleEntries();

    /**
     * This is used to print out some timings.
     */
    void timing(const QString& text, int elaspedms);

private:

    /**
     * Member variable so we can update the progress bar everywhere
     */
    DProgressDlg      *m_progressDlg;

    CollectionScanner  m_scanner;
};

}  // namespace Digikam

#endif /* SCANLIB_H */

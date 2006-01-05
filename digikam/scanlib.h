/* ============================================================
 * Author: Tom Albers <tomalbers@kde.nl>
 * Date  : 2005-01-01
 * Description : 
 * 
 * Copyright 2005-2006 by Tom Albers

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

#include <qstring.h>
#include <qmap.h>

// KDE includes.

#include <kurl.h>
#include <kprogress.h>

// Local includes.

#include "digikam_export.h"

/** @file scanlib.h */

namespace Digikam
{

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
class DIGIKAM_EXPORT ScanLib
{
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
     * This calls allFiles with a given path.
     * @param path the path to scan.
     */
    void findMissingItems(const QString &path);
            
    /** 
     * This queries the db for items that have no date
     * for each item found, storeItemInDatabase is called.
     */
    void updateItemsWithoutDate();

private:
    /**
    * This counts all existing files recursively, starting from
    * directory. 
    * @param directory The path to the start searching from
    * @return The amount of items
    */
    int countItemsInFolder(const QString& directory);

    /** 
    * This checks all existing files recursively, starting from
    * directory. Calls storeItemInDatabase to store the found items
    * which are not in the database.
    * @param directory The path to the start searching from
    */
    void allFiles(const QString& directory);

    /**
    * This fetches the exif-date or the modification date when
    * the exif-date is not available and retrieves the JFIF-comment
    * and calls AlbumDB::setItemDateComment to store the info in
    * the db.
    * @param albumURL The album path (relative to the
    *                           album library Path)
    * @param filename The filename of the item to store
    * @param albumID The albumID as used in the database
    */
    void storeItemInDatabase(const QString& albumURL, 
                             const QString& filename, 
                             int albumID);

    /** 
    * This fetches the exif-date or the modification date when
    * the exif-date is not available and calls AlbumDB::setItemDate
    * to store the info in
    * the db.
    * @param albumURL The album path (relative to the album library Path)
    * @param filename The filename of the item to store
    * @param albumID The albumID as used in the database
    */
    void updateItemDate(const QString& albumURL,
                        const QString& filename,
                        int albumID);

    /**
     * This will delete all items stored in m_filesToBeDeleted
     */
    void deleteStaleEntries();

    /**
     * Member variable so we can update the progress bar everywhere
     */
    KProgressDialog     *m_progressBar;

    /**
     * Member to store stale filesystem
     */
    QValueList< QPair<QString,int> >  m_filesToBeDeleted;

    /**
     * This is used to print out some timings.
     */
    void timing(const QString& text, struct timeval tv1, struct timeval tv2);
};

}  // namespace Digikam

#endif /* SCANLIB_H */

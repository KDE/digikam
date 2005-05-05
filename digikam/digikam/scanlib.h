////////////////////////////////////////////////////////////////////////////////
//
//    DIGIKAMAPP.CPP
//
//    Copyright (C) 2005 Tom Albers <tomalbers@kde.nl>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SCANLIB_H
#define SCANLIB_H

#include <kurl.h>
#include <kprogress.h>
#include <qstring.h>

/** @file scanlib.h */

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
class ScanLib
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
    * This calls allFiles with the albumPath.
    */
    void findMissingItems();

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
     * Member variable so we can update the progress bar everywhere
     */
    KProgressDialog   *m_progressBar;
};

#endif /* SCANLIB_H */

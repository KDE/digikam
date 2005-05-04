/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-18
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

/** @file albumdb.h */

#ifndef ALBUMDB_H
#define ALBUMDB_H

#include <qstring.h>
#include <qvaluelist.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include "albuminfo.h"

typedef struct sqlite sqleet; // hehe.
class Album;
class TAlbum;
class PAlbum;

typedef QValueList<int> IntList;
/**
 * This class is responsible for the communication
 * with the sqlite database.
 */
class AlbumDB
{
public:

    /**
     * Constructor
     */
    AlbumDB();
    
    /**
     * Destructor
     */
    ~AlbumDB();

    AlbumInfo::List scanAlbums();
    AlbumInfo::List scanTags();
    
    /**
     * Makes a connection to the database and makes sure all tables
     * are available.
     * @param path The database to open
     */
    void setDBPath(const QString& path);

    void scanTags(TAlbum *album);
    void readAlbum(Album *album);
    void deleteAlbum(Album *album);
    void renameAlbum(Album *album, const QString& newName);
    bool createTAlbum(TAlbum* parent, const QString& name, const QString& icon);
    void moveTAlbum(TAlbum *album, TAlbum *parent);
    bool addPAlbum(const QString& url, const QString& caption,
                   const QDate& date, const QString& collection);
    
    void beginTransaction();
    void commitTransaction();

    void setCaption(PAlbum *album);
    void setCollection(PAlbum *album);
    void setDate(PAlbum *album);
    /**
     * This adds a keyword-value combination to the database Settings table
     * if the keyword already exists, the value will be replaced with the new
     * value.
     * @param keyword The keyword
     * @param value The value
     */
    void setSetting(const QString& keyword, const QString& value);
    void setIcon(TAlbum *album, const QString& icon);
    void setIcon(PAlbum *album, const QString& icon);    

    QString     getItemCaption(PAlbum *album, const QString& name);
    QDateTime   getItemDate(PAlbum *album, const QString& name);
    /**
     * This function returns the value which is stored in the database
     * (table Settings).
     * @param keyword The keyword for which the value has to be returned.
     * @return The values which belongs to the keyword.
     */
    QString     getSetting(const QString& keyword);
    QStringList getItemTagNames(PAlbum *album, const QString& name);
    IntList     getItemTagIDs(PAlbum *album, const QString& name);
    IntList     getItemCommonTagIDs(const IntList& dirIDList, const QStringList& nameList);

    void setItemCaption(PAlbum *album, const QString& name,
                        const QString& caption);
    void setItemTag(PAlbum *album, const QString& name, TAlbum* tag);
    void removeItemTag(PAlbum *album, const QString& name, TAlbum* tag);
    void removeItemAllTags(PAlbum *album, const QString& name);

    /**
     * This can be used to find out the albumID for a given
     * folder. If it does not exist, it will be created and the
     * new albumID will be returned.
     * @param folder The folder for which you want the albumID
     * @return It returns the albumID for that folder.
     */
    int         getOrCreateAlbumId(const QString& folder);
    
    /**
     * Returns all items for a given albumid. This is used to
     * verify if all items on disk are consistent with the database
     * in the scanlib class.
     * @param albumID The albumID for which you want all items.
     * @return It returns a QStringList with the filenames.
     */
    QStringList getItemNamesInAlbum(int albumID);
    
    /**
     * Returns all items in the database without a date. This is used
     * in the scanlib class which tries to find out the date of the 
     * items, so the database holds the date for each item. This was
     * not the case untill the 0.8.0 release.
     * @return The path (starting from albumPath and including the 
     * the filename of all items.
     */
    QStringList getAllItemURLsWithoutDate();

    /**
     * This is simple function to put a new Item in the database,
     * without checking if it already exists, but since albumID+name
     * has to be unique, it will simply replace the datatime and comment
     * for an already existing item. 
     * @param albumID The albumID where the file is located.
     * @param name The filename
     * @param datetime The datetime to be stored. Should try to let that be
     * the exif-datetime, but if not available the modification date.
     * @param comment The user comment as found in the exif-headers of the 
     * file.
     * @return It will always return true. Maybe that will change.
     */
    bool setItemDateComment(int albumID, const QString& name,
                            const QDateTime& datetime,
                            const QString& comment);
    
    /**
     * This is simple function to put a new Item in the database,
     * without checking if it already exists, but since albumID+name
     * has to be unique, it will simply replace the datatime and comment
     * for an already existing item. 
     * @param albumID The albumID where the file is located.
     * @param name The filename
     * @param datetime The datetime to be stored. Should try to let that be
     * the exif-datetime, but if not available the modification date.
     * @return It will always return true. Maybe that will change.
     */
    bool setItemDate(int albumID, const QString& name,
                     const QDateTime& datetime);
        
    QStringList getItemsURL(TAlbum *album);
    void getItemsInTAlbum(TAlbum* album, QStringList& urls,
                          QValueList<int>& dirids);

    void copyItem(PAlbum *srcAlbum,  const QString& srcFile,
                  PAlbum *destAlbum, const QString& destFile); 
    void moveItem(PAlbum *srcAlbum,  const QString& srcFile,
                  PAlbum *destAlbum, const QString& destFile);
    void deleteItem(PAlbum *album,   const QString& file);
    void deleteItem(int albumID,     const QString& file);

private:

    /**
     * Checks the available tables and creates them if they are not 
     * available.
     */
    void initDB();

    bool importXML(PAlbum *album);
    bool exportXML(PAlbum *album);

    void readPAlbum(PAlbum* album);
    void readTAlbum(TAlbum* album);
    void renameTAlbum(TAlbum* album, const QString& name);
    void renamePAlbum(PAlbum* album, const QString& url);
    
    bool readIdentifier(PAlbum *album, int& id);
    void writeIdentifier(PAlbum *album, int id);

    bool checkAlbum(PAlbum *album, int id);

    /**
     * This will execute a given SQL statement to the database.
     * @param sql The SQL statement
     * @param values This will be filled with the result of the SQL statement
     * @param debug If true, it will output the SQL statement 
     * @return It will return if the execution of the statement was succesfull
     */
    bool execSql(const QString& sql, QStringList* const values = 0, 
                 const bool debug = false);

    /**
     * Escapes text fields. This is needed for all queries to the database
     * which happens with an argument which is a text field. It makes sure
     * a ' is replaced with '', as this is needed for sqlite.
     * @param str String to escape
     * @return The escaped string
     */
    QString escapeString(QString str) const;

    void removeInvalidEntries();
    
    sqleet*       m_db;
    bool          m_valid;
};

#endif /* ALBUMDB_H */

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

#ifndef ALBUMDB_H
#define ALBUMDB_H

#include <qstring.h>
#include <qvaluelist.h>
#include <qstringlist.h>
#include <qdatetime.h>

typedef struct sqlite sqleet; // hehe.
class Album;

typedef QValueList<int> IntList;

class AlbumDB
{
public:

    AlbumDB();
    ~AlbumDB();

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
    void setIcon(TAlbum *album, const QString& icon);
    void setIcon(PAlbum *album, const QString& icon);    

    QString     getItemCaption(PAlbum *album, const QString& name);
    QStringList getItemTagNames(PAlbum *album, const QString& name);
    IntList     getItemTagIDs(PAlbum *album, const QString& name);
    IntList     getItemCommonTagIDs(const IntList& dirIDList, const QStringList& nameList);

    void setItemCaption(PAlbum *album, const QString& name,
                        const QString& caption);
    void setItemTag(PAlbum *album, const QString& name, TAlbum* tag);
    void removeItemTag(PAlbum *album, const QString& name, TAlbum* tag);
    void removeItemAllTags(PAlbum *album, const QString& name);

    QStringList getItemsURL(TAlbum *album);
    void getItemsInTAlbum(TAlbum* album, QStringList& urls,
                          QValueList<int>& dirids);

    void copyItem(PAlbum *srcAlbum,  const QString& srcFile,
                  PAlbum *destAlbum, const QString& destFile); 
    void moveItem(PAlbum *srcAlbum,  const QString& srcFile,
                  PAlbum *destAlbum, const QString& destFile);
    void deleteItem(PAlbum *album,   const QString& file);
    
private:

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

    bool execSql(const QString& sql, QStringList* const values = 0, 
                 const bool debug = false);
    QString escapeString(QString str) const;

    void removeInvalidEntries();
    
    sqleet*       m_db;
    bool          m_valid;
};

#endif /* ALBUMDB_H */

/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-15
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

#ifndef ALBUMMANAGER_H
#define ALBUMMANAGER_H

#include <qobject.h>
#include <qstring.h>
#include <qvaluelist.h>

#include <kfileitem.h>
#include <kurl.h>

class Album;
class PAlbum;
class TAlbum;
class AlbumDB;
class AlbumItemHandler;
class AlbumManagerPriv;

class QDate;
class KDirLister;

typedef QValueList<PAlbum*> PAlbumList;
typedef QValueList<TAlbum*> TAlbumList;
    

class AlbumManager : public QObject
{
    Q_OBJECT

    friend class AlbumDB;
    
public:

    AlbumManager();
    ~AlbumManager();

    static AlbumManager* instance();
    
    AlbumDB*   albumDB();

    void       setLibraryPath(const QString& path);
    QString    getLibraryPath() const;

    PAlbumList pAlbums() const;
    TAlbumList tAlbums() const;

    void      setCurrentAlbum(Album *album);
    Album*    currentAlbum() const;

    PAlbum*   findPAlbum(const KURL& url) const;
    PAlbum*   findPAlbum(int id) const;
    TAlbum*   findTAlbum(int id) const;
    
    bool createPAlbum(PAlbum* parent, const QString& name,
                      const QString& caption, const QDate& date,
                      const QString& collection, 
                      QString& errMsg);
    bool deletePAlbum(PAlbum* album, QString& errMsg);
    
    bool renamePAlbum(PAlbum* album, const QString& newName, QString& errMsg);
    bool updatePAlbumIcon(PAlbum *album, const QString& icon, 
                          bool emitSignalChanged, QString& errMsg);
    
    bool createTAlbum(TAlbum* parent, const QString& name, 
                      const QString& icon, QString& errMsg);
    bool deleteTAlbum(TAlbum* album, QString& errMsg);
    bool renameTAlbum(TAlbum* album, const QString& name, QString& errMsg);
    bool updateTAlbumIcon(TAlbum* album, const QString& icon, 
                          bool emitSignalChanged, QString& errMsg);
    bool moveTAlbum(TAlbum* album, TAlbum *parent, QString &errMsg);
    
    void setItemHandler(AlbumItemHandler *handler);
    AlbumItemHandler* getItemHandler();
    void refreshItemHandler(const KURL::List& itemList=KURL::List());
    void emitAlbumItemsSelected(bool val);

private:

    static AlbumManager* m_instance;
    AlbumManagerPriv*    d;

    void addPAlbum(KFileItem* fileItem);
    void createAlbumLister();

    void insertPAlbum(PAlbum *album);
    void removePAlbum(PAlbum *album);
    void insertTAlbum(TAlbum *album);
    void removeTAlbum(TAlbum *album);

private slots:

    void slotNewItems(const KFileItemList& itemList);
    void slotDeleteItem(KFileItem* item);
    void slotClearItems();
    void slotCompleted();
    void slotRedirection(const KURL& oldURL, const KURL& newURL);

signals:

    void signalAlbumAdded(Album* album);
    void signalAlbumDeleted(Album* album);
    void signalAlbumItemsSelected(bool selected);
    void signalAlbumsCleared();
    void signalAlbumCurrentChanged(Album* album);
    void signalAllAlbumsLoaded();
    void signalPAlbumIconChanged(PAlbum* album);    
    void signalTAlbumIconChanged(TAlbum* album);        
};

#endif /* ALBUMMANAGER_H */

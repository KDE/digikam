/* ============================================================
 * File  : albummanager.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-09-22
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
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
#include <qstringlist.h>
#include <kfileitem.h>

class QString;

namespace Digikam
{

class AlbumInfo;
class AlbumItemHandler;
class AlbumManagerPriv;

/*! 
 AlbumManager

 This is the base album manager, which manages the albums in the
 library path. You can traverse through all the albums like this
 \code
 for(AlbumInfo *album=AlbumManager::instance()->firstAlbum();
     album; album = album->nextAlbum()) {
     doSomething(album);
  }
  \endcode
*/

class AlbumManager : public QObject {

    Q_OBJECT

    friend class AlbumItemHandler;
    
public:

    /*! 
      \return the instance of the Album Manager
    */
    static AlbumManager *instance();

    /*! 
      Contructor
      \warning for internal use
    */
    AlbumManager(QObject *parent);

    /*! 
      Destructor   
      \warning for internal use
    */
    ~AlbumManager();

    /*! 
     Set the base library path
     \warning for internal use
     */
    void       setLibraryPath(const QString& libraryPath);

    /*!
      \return the base library path
     */
    QString    getLibraryPath();

    /*!
      \return the first album in this library path
     */
    AlbumInfo *firstAlbum();
    
    /*!
      \return the last album in this library path
     */
    AlbumInfo *lastAlbum();

    /*!
      \return the album with this title or 0 if not found
     */
    AlbumInfo *findAlbum(const QString& title);

    /*! 
      set the current/selected album
      \warning for internal use
     */
    void setCurrentAlbum(AlbumInfo *album);

    /*! 
      \return the current/selection album
     */
    AlbumInfo* currentAlbum();

    /*! 
      rename this album with given title
     */
    void renameAlbum(AlbumInfo *album, const QString& newTitle);

    /*! 
      insert an album
      \warning for internal use
     */
    void insertAlbum(AlbumInfo* album);

    /*! 
      remove an album
      \warning for internal use
     */
    void takeAlbum(AlbumInfo* album);

    /*! 
      Set the item handler
      \warning for internal use
     */
    void setItemHandler(AlbumItemHandler *handler);

    /*! 
      Get the item handler
      \warning for internal use
     */
    AlbumItemHandler* getItemHandler();

    /*! 
     Refresh the item handler, say for eg, when you changed the
     comments of the items
     */
    void refreshItemHandler(const QStringList& itemList=QStringList());
    
private:

    void clearAlbums();
    void addAlbum(const QString& name);
    bool renameDirectory(const QString& oldPath,
                         const QString& newPath,
                         QString& error);
    void emitAlbumItemsSelected(bool val);
    
    AlbumManagerPriv    *d;
    static AlbumManager *m_instance;

signals:

    /*! 
      emitted when a album is added
     */
    void signalAlbumAdded(Digikam::AlbumInfo *album);

    /*! 
      emitted when a album is deleted
     */
    void signalAlbumDeleted(Digikam::AlbumInfo *album);

    /*! 
      emitted when all albums are cleared, for eg, when the album library
      path is changed
     */
    void signalAlbumsCleared();

    /*! 
      emitted when the current/selected album changes
    */
    void signalAlbumCurrentChanged(Digikam::AlbumInfo *album);

    /*! 
      emitted when items are selected (or not) in the current album
    */
    void signalAlbumItemsSelected(bool val);

private slots:

    void slotNewItems(const KFileItemList& itemList);
    void slotDeleteItem(KFileItem* item);
    void slotClearItems();
};

}

#endif /* ALBUMMANAGER_H */

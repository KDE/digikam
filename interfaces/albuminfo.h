/* ============================================================
 * File  : albuminfo.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-08
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

#ifndef ALBUMINFO_H
#define ALBUMINFO_H

#include <qstring.h>
#include <qstringlist.h>
#include <qdatetime.h>

namespace Digikam
{

class AlbumManager;
class AlbumXMLEditor;

/*!
 * AlbumInfo
 *
 * Represents an album and its properties. Also provides an interface
 * to the album database, so that you can retrieve the properties for
 * the photoitems in this album
 */

class AlbumInfo
{
    friend class AlbumManager;
    friend class AlbumXMLEditor;
    friend class AlbumXMLHandler;
    
public:

    /*! 
      Constructor for this album
      \warning for internal use only
     */
    AlbumInfo(AlbumManager *parent,
              const QString& title);

    /*! 
      Destructor for this album
      \warning for internal use only
     */
    ~AlbumInfo();

    /*! 
      Open the database for this album
      \note If you plan to retrieve/set comments from this album or
      change the properties of the album, make sure that you open the
      database first. You don't need to open the database if you are
      only getting the properties of the album like collection, date...
    */
    void openDB();
    
    /*! 
      Close the database for this album
      \note close the database once you have finished with this album
      if you have opened it. 
     */
    void closeDB();

    /*! 
      \return the title of the album
     */
    QString getTitle() const;

    /*! 
      return the path of the album
     */
    QString getPath()  const;
    
    /*! 
      set the comments for this album
     */
    void    setComments(const QString& comments);

    /*! 
      \return the comments for this album
     */
    QString getComments() const;

    /*! 
      set the collection for this album
     */
    void    setCollection(const QString& collection);

    /*! 
      \return the collection for this album
     */
    QString getCollection() const;

    /*! 
     set the date for this album
     */
    void    setDate(const QDate& date);

    /*! 
      \return the date for this album
     */
    QDate   getDate() const;

    /*! 
      \return a pointer to the next album, or 0 if this is the last
      album. You can traverse through all the albums like this
      \code
      for(AlbumInfo *album=AlbumManager::instance()->firstAlbum();
          album; album = album->nextAlbum()) {
          doSomething(album);
      }
      \endcode
     */
    AlbumInfo* nextAlbum();
    
    /*! 
      \return a pointer to the previous album, or 0 if this is the
      first album. 
     */
    AlbumInfo* prevAlbum();

    
    /*! 
     set the comments for an item in this album
     */
    void setItemComments(const QString& name,
                         const QString& comments);
    
    /*! 
     \return the comments for an item in this album or empty string if
     not found in database
     */
    QString getItemComments(const QString& name);

    /*! 
     delete the comments for an item in this album
     */
    void deleteItemComments(const QString& name);

    /*! 
      Get a list of names of all the items in this album if this is the
      current album. Result is unpredictable if this is not the
      current album
     */
    QStringList getAllItems();

    /*! 
      Get a list of names of selected items in this album if this is the
      current album. Result is unpredictable if this is not the
      current album
     */
    QStringList getSelectedItems();
    
    /*! 
      Get a list of paths of all the items in this album if this is the
      current album. Result is unpredictable if this is not the
      current album
     */
    QStringList getAllItemsPath();

    /*! 
      Get a list of paths of selected items in this album if this is the
      current album. Result is unpredictable if this is not the
      current album
     */
    QStringList getSelectedItemsPath();

    /*! 
      set the viewitem for this album.
      \warning only for internal use 
     */
    void    setViewItem(void *viewItem);

    /*! 
      \return the viewitem for this album
      \warning only for internal use 
     */
    void*   getViewItem();

private:

    QString          title_;
    QString          comments_;
    QString          collection_;
    QDate            date_;

    void            *viewItem_;
    AlbumInfo       *next_;
    AlbumInfo       *prev_;
    bool             modified_;
    bool             hidden_;

    AlbumManager    *parent_;
    AlbumXMLEditor  *db_;
};

}

#endif /* ALBUMINFO_H */

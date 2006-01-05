/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-15
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju
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

/** @file album.h */

#ifndef ALBUM_H
#define ALBUM_H

// Qt includes.

#include <qstring.h>
#include <qdatetime.h>
#include <qmap.h>

// KDE includes.

#include <kurl.h>

namespace Digikam
{

/**
 * \class Album
 * \brief Abstract base class for all album types
 *
 * A class which provides an abstraction for a type Album. This class is meant to
 * be derived and everytime a new Album Type is defined add a enum corresponding
 * to that to Album::Type
 *
 * This class provides a means of building a tree representation for
 * Albums @see Album::setParent(). 
 */

class Album
{
public:

    enum Type 
    {
        PHYSICAL=0, /**<  PHYSICAL: A physical album type @see PAlbum */
        TAG,        /**<  TAG:      A tag      album type @see TAlbum */
        DATE,       /**<  DATE:     A date     album type @see DAlbum */
        SEARCH      /**<  SEARCH:   A search   album type @see SAlbum */
    };

    /**
     * Destructor
     *
     * this will also recursively delete all child Albums
     */
    virtual ~Album();

    /**
     * Delete all child albums and also remove any associated extra data
     */
    void    clear();
    
    /**
     * @return the parent album for this album
     */
    Album*  parent() const;
            
    /**
     * @return the first child of this album or 0 if no children
     */
    Album*  firstChild() const;
    
    /**
     * @return the last child of this album or 0 if no children
     */
    Album*  lastChild() const;
    
    /**
     * @return the next sibling of this album of this album or 0
     * if no next sibling
     * @see AlbumIterator
     */
    Album*  next() const;

    /**
     * @return the previous sibling of this album of this album or 0 if no
     * previous sibling
     * @see AlbumIterator
     */
    Album*  prev() const;

    /**
     * @return the type of album
     * @see Type
     */
    Type    type() const;

    /**
     * Each album has a @p ID uniquely identifying it in the set of Albums of
     * a Type
     *
     * \note The @p ID for a root Album is always 0
     *
     * @return the @p ID of the album
     * @see globalID()
     */
    int     id() const;

    /**
     * An album ID is only unique among the set of all Albums of its Type.
     * This is a global Identifier which will uniquely identifying the Album
     * among all Albums
     *
     * \note If you are adding a new Album Type make sure to update
     * this implementation.
     *
     * You can always get the @p ID of the album using something like
     *
     * \code
     * int albumID = rootAlbum->globalID() - album->globalID();
     * \endcode
     * 
     * @return the @p globalID of the album
     * @see id()
     */
    int     globalID() const;

    /**
     * @return the @p title aka name of the album
     */
    QString title() const;

    /**
     * @return the kde url of the album
     */
    virtual KURL kurl() const = 0;

    /**
     * @return true is the album is a Root Album
     */
    bool    isRoot() const;
    
    /**
     * @return true if the @p album is in the parent hierarchy
     *
     * @param album Album to check whether it belongs in the child
     * hierarchy
     */
    bool    isAncestorOf(Album* album) const;
    
    /**
     * This allows to associate some "extra" data to a Album. As one
     * Album can be used by several objects (often views) which all need
     * to add some data, you have to use a key to reference your extra data
     * within the Album.
     *
     * That way a Album can hold and provide access to all those views
     * separately.
     *
     * for eg,
     *
     * \code
     * album->setExtraData( this, searchFolderItem );
     * \endcode
     *
     * and can later access the searchFolderItem by doing
     *
     * \code
     * SearchFolderItem *item = static_cast<SearchFolderItem*>(album->extraData(this));
     * \endcode
     *
     * Note: you have to remove and destroy the data you associated yourself
     * when you don't need it anymore!
     *
     * @param key the key of the extra data
     * @param value the value of the extra data
     * @see extraData
     * @see removeExtraData                                   
     */
    void    setExtraData(const void* key, void *value);

    /**
     * Remove the associated extra data associated with @p key
     *
     * @param key the key of the extra data
     * @see setExtraData
     * @see extraData
     */
    void    removeExtraData(const void* key);
    
    /**
     * Retrieve the associated extra data associated with @p key
     *
     * @param key the key of the extra data
     * @see setExtraData
     * @see extraData
     */
    void*   extraData(const void* key) const;

protected:
    
    /**
     * Constructor
     */
    Album(Album::Type type, int id, bool root);

    /**
     * @internal use only
     *
     * Set a new title for the album
     *
     * @param title new title for the album
     */
    void setTitle(const QString& title);

    /**
     * @internal use only
     *
     * Set the parent of the album
     *
     * @param parent set the parent album of album to @p parent
     */
    void setParent(Album* parent);

    /**
     * @internal use only
     *
     * Insert an Album as a child for this album
     *
     * @param child the Album to add as child
     */
    void insertChild(Album* child);

    /**
     * @internal use only
     *
     * Remove a Album from the children list for this album
     *
     * @param child the Album to remove
     */
    void removeChild(Album* child);

private:
    
    /**
     * Disable copy and default constructor
     */
    Album();
    Album(const Album&);
    Album& operator==(const Album&);
    
private:
    
    Type                       m_type;
    int                        m_id;
    bool                       m_root;
    QString                    m_title;
                               
    Album*                     m_parent;
    Album*                     m_firstChild;
    Album*                     m_lastChild;
    Album*                     m_next;
    Album*                     m_prev;
    bool                       m_clearing;

    QMap<const void*, void*>   m_extraMap;

    friend class AlbumManager;
};
    
/**
 * \class PAlbum
 *
 * A Physical Album representation 
 */

class PAlbum : public Album
{
public:

    PAlbum(const QString& title, int id, bool root=false);
    ~PAlbum();

    void setCaption(const QString& caption);
    void setCollection(const QString& collection);
    void setDate(const QDate& date);

    QString    caption() const;
    QString    collection() const;
    QDate      date() const;
    QString    url() const;
    QString    prettyURL() const;
    QString    folderPath() const;
    KURL       kurl() const;
    QString    icon() const;
    KURL       iconKURL() const;
    
private:

    QString    m_collection;
    QString    m_caption;
    QDate      m_date;
    QString    m_icon;

    friend class AlbumManager;
};

/**
 * \class TAlbum
 *
 * A Tag Album representation 
 */

class TAlbum : public Album
{
public:

    TAlbum(const QString& title, int id, bool root=false);
    ~TAlbum();

    QString url() const;
    KURL    kurl() const;
    QString prettyURL() const;
    QString icon() const;

private:

    QString m_icon;
    int     m_pid;

    friend class AlbumManager;
};

/**
 * \class DAlbum
 *
 * A Date Album representation 
 */

class DAlbum : public Album
{
public:

    DAlbum(const QDate& date, bool root=false);
    ~DAlbum();

    QDate date() const;
    KURL  kurl() const;

private:

    QDate       m_date;
    static int  m_uniqueID;

    friend class AlbumManager;
};

/**
 * \class SAlbum
 *
 * A Search Album representation 
 */

class SAlbum : public Album
{
public:

    SAlbum(int id, const KURL& url, bool simple, bool root=false);
    ~SAlbum();

    KURL    kurl() const;
    bool    isSimple() const;

private:

    KURL m_kurl;
    bool m_simple;

    friend class AlbumManager;
};

/**
 *  \class AlbumIterator
 *  
 *  Iterate over all children of this Album.
 *  \note It will not include the specified album
 *
 *  Example usage:
 *  \code
 *  AlbumIterator it(album);
 *  while ( it.current() )
 *  {
 *     kdDebug() << "Album: " << it.current() ->getTitle() << endl;
 *     ++it;
 *  }
 * \endcode
 *
 *  \warning Do not delete albums using this iterator.
 */

class AlbumIterator
{
public:

    AlbumIterator(Album *album);
    ~AlbumIterator();

    AlbumIterator& operator++();
    Album*         operator*();
    Album*         current() const;
    
private:

    AlbumIterator() {}
    AlbumIterator(const AlbumIterator&) {}
    AlbumIterator& operator=(const AlbumIterator&){ return *this; }
    
    Album* m_current;
    Album* m_root;
};
    
}  // namespace Digikam

#endif /* ALBUM_H */

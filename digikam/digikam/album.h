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

#ifndef ALBUM_H
#define ALBUM_H

#include <kurl.h>

#include <qstring.h>
#include <qdatetime.h>
#include <qpixmap.h>

/* Abstract base class for all albums */

class Album
{
    friend class AlbumManager;
    
public:

    enum Type 
    {
        PHYSICAL=0,
        TAG,
        DATE,
        SEARCH
    };

    Album(Album::Type type, int id, const QString& title, bool root);
    virtual ~Album();
    
    void    setViewItem(void *viewItem);
    void*   getViewItem() const;
            
    void    setParent(Album* parent);
    Album*  getParent() const;
            
    void    insertChild(Album* child);
    void    removeChild(Album* child);
    Album*  firstChild() const;
    Album*  lastChild() const;
    Album*  next() const;
    Album*  prev() const;

    void    clear();

    void    setID(int id);
    int     getID() const;

    void    setTitle(const QString& title);
    QString getTitle() const;
    virtual QString getURL() const;
    virtual KURL    getKURL() const = 0;
    Type    type() const;

    bool    isRoot() const;
    bool    isAncestorOf(Album* album) const;
    
    void    setIcon(const QString& icon);
    void    deleteIcon();
    virtual QString getIcon() const;
    
protected:
    
    QString m_url;
    int     m_id;
    bool    m_root;
    QString m_title;
    QString m_icon;

private:
    
    Type    m_type;

    Album*  m_parent;
    Album*  m_firstChild;
    Album*  m_lastChild;
    Album*  m_next;
    Album*  m_prev;
    bool    m_clearing;

    void*   m_viewItem;
};
    
/* A Physical Album representation */

class PAlbum : public Album
{
public:

    PAlbum(const QString& title, int id, bool root=false);
    ~PAlbum();

    void setCaption(const QString& caption, bool addToDB=true);
    void setCollection(const QString& collection, bool addToDB=true);
    void setDate(const QDate& date, bool addToDB=true);

    QString    getCaption() const;
    QString    getCollection() const;
    QDate      getDate() const;
    QString    getURL() const;
    QString    getPrettyURL() const;
    KURL       getKURL() const;
    KURL       getIconKURL() const;
    QString    getFolderPath() const;
    
private:

    QString    m_collection;
    QString    m_caption;
    QDate      m_date;
};

/* A Tag Album representation */

class TAlbum : public Album
{
public:

    TAlbum(const QString& title, int id, bool root=false);
    ~TAlbum();

    void    setPID(int id);
    int     getPID() const;
    QString getURL() const;
    KURL    getKURL() const;
    QString getPrettyURL() const;
    QPixmap getPixmap() const;
    
private:

    int     m_pid;
};

class DAlbum : public Album
{
public:

    DAlbum(const QDate& date, bool root=false);
    ~DAlbum();

    QDate getDate() const;
    KURL  getKURL() const;

private:

    QDate m_date;
};

class SAlbum : public Album
{
public:

    SAlbum(const KURL& url, bool simple, bool root=false);
    ~SAlbum();

    KURL    getKURL() const;
    QString getName() const;
    bool    isSimple() const;
    

private:

    KURL m_kurl;
    bool m_simple;

    friend class AlbumManager;
};

/* Iterate over all children of this Album.
   Note: It doesn't include the specified album

   Example usage:
   
   AlbumIterator it(album);
   while ( it.current() )
   {
      kdDebug() << "Album: " << it.current() ->getTitle() << endl;
      ++it;
   }

   Do not delete albums using this iterator.
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
    
#endif /* ALBUM_H */

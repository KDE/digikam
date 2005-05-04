/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-21
 * Copyright 2005 by Renchi Raju
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
 * ============================================================ */

/** @file imageinfo.h */

#ifndef IMAGEINFO_H
#define IMAGEINFO_H

#include <qstring.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <kurl.h>

class PAlbum;
class AlbumManager;

/**
 * Class ImageInfo
 */
class ImageInfo
{
public:

    /**
     * Constructor
     * @param     albumID  id of the PAlbum to which this item belongs
     * @param     name     name of the image
     * @param     datetime datetime for the image
     * @param     size     filesize for the image
     */
    ImageInfo(int albumID, const QString& name,
              const QDateTime& datetime, size_t size);

    /** 
     * Destructor
     */
    ~ImageInfo();

    /**
     * @return the name of the image
     */
    QString   name() const;

    /**
     * Set a new name for the image. This will rename the file on the
     * disk to the new name. Only use if you are sure of what you are
     * doing
     * @param  newName new name for the image
     * @return true if successful, false otherwise
     */
    bool      setName(const QString& newName);
    
    /**
     * @return the datetime of the image
     */
    QDateTime dateTime() const;

    /**
     * @return the filesize of the image
     */
    size_t    fileSize() const;

    /**
     * @return the standard KDE url with file protocol. The path for
     * the url is the absolute path of the image
     */
    KURL      kurl() const;

    /**
     * @return the id of the PAlbum to which this item belongs
     */
    int       albumID() const;

    PAlbum*   album() const;

    QString     caption() const;
    QStringList tagNames() const;
    QStringList tagPaths() const;
    void        setTag(int tagID);
    void        removeTag(int tagID);
    
    void        setViewItem(void *d);
    void*       getViewItem() const;

    /**
     * refresh the properties of the imageinfo. it reads the database
     * again for getting the updated date and stats the file to get
     * the updated size 
     */
    void        refresh();
    
private:

    ImageInfo() {}
    ImageInfo(const ImageInfo& ) {}
    ImageInfo& operator=(const ImageInfo& ) {return *this;}
    
    int                   m_albumID;
    QString               m_name;
    QDateTime             m_datetime;
    size_t                m_size;
    void*                 m_viewitem;
    static  AlbumManager* m_man;
};

typedef QPtrList<ImageInfo>         ImageInfoList;
typedef QPtrListIterator<ImageInfo> ImageInfoListIterator;

#endif /* IMAGEINFO_H */

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
 *
 * ============================================================ */

/** @file imageinfo.h */

#ifndef IMAGEINFO_H
#define IMAGEINFO_H

// Qt includes.

#include <qstring.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qsize.h>

// KDE includes.

#include <kurl.h>

namespace Digikam
{

class PAlbum;
class AlbumManager;

/**
 * This is an abstraction of a file entity on the disk.
 *
 * This is an abstraction of a file entity on the disk and is uniquely 
 * identified by an Album identifier (PAlbum in which its located) and its
 * name. Additional methods are provided for accessing/modifying:
 * - datetime
 * - caption
 * - filesize
 * - tags
 * - url (ImageInfo::kurl()) -> Note: accessing this will give you the 
 *   standard KDE file representation. 
 * (for eg: file:///home/johndoe/Pictures/ItalyPictures/Rome-2004/file001.jpeg)
 */
class ImageInfo
{
public:

    /**
     * Constructor
     * Creates a null image info
     */
    ImageInfo();

    /**
     * Constructor
     * @param     ID       unique ID for this image
     * @param     albumID  id of the PAlbum to which this item belongs
     * @param     name     name of the image
     * @param     datetime datetime of the image
     * @param     size     filesize of the image
     * @param     dims     dimensions of the image
     */
    ImageInfo(Q_LLONG ID, int albumID, const QString& name,
              const QDateTime& datetime, size_t size,
              const QSize& dims=QSize());

    /**
     * Constructor
     * @param    ID       unique ID for this image
     */
    ImageInfo(Q_LLONG ID);

    /** 
     * Destructor
     */
    ~ImageInfo();

    /**
     * Returns if this objects contains valid data
     */
    bool      isNull() const;

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
     * @return the modification datetime of the image
     */
    QDateTime modDateTime() const;

    /**
     * @return the filesize of the image
     */
    size_t    fileSize() const;

    /**
     * @return the dimensions of the image (valid only if dimensions
     * have been requested)
     */
    QSize     dimensions() const;
    
    /**
     * @return the standard KDE url with file protocol. The path for
     * the url is the absolute path of the image.
     * DEPRECATED: don't use this. if you need only the file path, then
     * use filePath(). And if you need the digikam "proper" kurl, use
     * kurlForKIO(). At some point kurl and kurlForKIO() will be merged
     * together
     */
    KURL      kurl() const;

    /**
     * @return the absolute file path of the image
     */
    QString   filePath() const;

    /**
     * @return the kurl for KIO or rather DIO. Use this instead of kurl()
     * for metadata preserving file IO operations. Also, this method needs
     * to be merged with kurl()
     */
    KURL      kurlForKIO() const;
    
    /**
     * @return the unique image id for this item
     */
    Q_LLONG   id() const;

    /**
     * @return the id of the PAlbum to which this item belongs
     */
    int       albumID() const;

    /** 
     * @return the PAlbum to which this item belongs
     */
    PAlbum*   album() const;

    /** 
     * @return the caption for this item
     */
    QString  caption() const;

    /**
     * Set the caption (writes it to database)
     * @param caption the new caption for this item
     */
    void  setCaption(const QString& caption);

    /**
     * Set the date and time (write it to database)
     * @param dateTime the new date and time.
     */
    void setDateTime(const QDateTime& dateTime);

    /**
     * @return a list of names of all tags assigned to this item
     * @see tagPaths
     */
    QStringList tagNames() const;

    /**
     * @return a list of complete path of all tags assigned to this item. The
     * complete path for a tag is a '/' separated string of its hierarchy
     * @see tagPaths
     */
    QStringList tagPaths(bool leadingSlash = true) const;

    
    /**
     * @return a list of IDs of tags assigned to this item
     * @see tagNames
     * @see tagPaths
     * @see Album::id()
     */
    QValueList<int> tagIDs() const;

    /**
     * Adds a tag to the item (writes it to database)
     * @param tagID the ID of the tag to add
     */
    void        setTag(int tagID);

    /**
     * Adds tags in the list to the item.
     * Tags are created if they do not yet exist
     */
    void        addTagPaths(const QStringList &tagPaths);

    /**
     * Remove a tag from the item (removes it from database)
     * @param tagID the ID of the tag to remove
     */
    void        removeTag(int tagID);

    /**
     * Remove all tags from the item (removes it from database)
     */
    void        removeAllTags();

    int         rating() const;

    void        setRating(int value);

    /**
     * Copy database information of this item to a newly created item
     * @param  dstAlbum    destination album
     * @param  dstFileName new filename
     * @return an ImageInfo object of the new item
     */
    ImageInfo   copyItem(PAlbum *dstAlbum, const QString &dstFileName);

    /**
     * Assign a viewitem for this item. This is useful when a view has a
     * corresponding viewitem for this item and wants to access the viewitem, given
     * this item
     * @see getViewItem()
     */
    void        setViewItem(void *d);
    
    /**
     * Returns the viewitem associated with this item a viewitem for this item.
     * @see setViewItem()
     */
    void*       getViewItem() const;

    /**
     * refresh the properties of the imageinfo. it reads the database
     * again for getting the updated date and stats the file to get
     * the updated size 
     */
    void        refresh();

private:

    Q_LLONG               m_ID;
    int                   m_albumID;
    QString               m_name;
    mutable QDateTime     m_datetime;
    mutable QDateTime     m_modDatetime;
    mutable size_t        m_size;
    QSize                 m_dims;
    void*                 m_viewitem;
    static  AlbumManager* m_man;
};

typedef QPtrList<ImageInfo>         ImageInfoList;
typedef QPtrListIterator<ImageInfo> ImageInfoListIterator;

}  // namespace Digikam

#endif /* IMAGEINFO_H */

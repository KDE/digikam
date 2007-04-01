/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-21
 * Copyright 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

// Local includes

#include "digikam_export.h"
#include "databaseurl.h"
#include "imagelister.h"
#include "imagelisterrecord.h"

namespace Digikam
{

class PAlbum;

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
class DIGIKAM_EXPORT ImageInfo
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
    ImageInfo(Q_LLONG ID, int albumID,
              const QString &album, const QString& albumName,
              const KURL& albumRoot,
              const QDateTime& datetime, size_t size,
              const QSize& dims=QSize());

    /**
     * Essentially the same as above
     */
    ImageInfo(const ImageListerRecord &record);

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
     * Returns the digikamalbums:// URL.
     * The returned object can be used as a KURL.
     * Always use this for KIO operations
     */
    DatabaseUrl databaseUrl() const;

    /**
     * Returns the file:// url.
     * This is equivalent to databaseUrl().fileUrl()
     */
    KURL       fileUrl() const;

    /**
     * Equivalent to fileUrl()
     */
    KDE_DEPRECATED KURL      kurl() const;

    /**
     * Equivalent to fileUrl().path()
     */
    // Deprecate?
    QString   filePath() const;

    /**
     * Equivalent to databaseUrl()
     */
    KDE_DEPRECATED KURL      kurlForKIO() const;

    /**
     * @return the unique image id for this item
     */
    Q_LLONG   id() const;

    /**
     * @return the id of the PAlbum to which this item belongs
     */
    int       albumID() const;

    /**
     * @return the caption for this item
     */
    QString   comment() const;

    int       rating() const;

    /**
     * @return a list of names of all tags assigned to this item
     * @see tagPaths
     */
    QStringList tagNames() const;

    /**
     * @return a list of IDs of tags assigned to this item
     * @see tagNames
     * @see tagPaths
     * @see Album::id()
     */
    QValueList<int> tagIDs() const;



    /**
     * Set the caption (writes it to database)
     * @param caption the new caption for this item
     */
    void  setComment(const QString& caption);

    /**
     * Set the date and time (write it to database)
     * @param dateTime the new date and time.
     */
    void setDateTime(const QDateTime& dateTime);

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

    /**
     * Set the rating for the item
     */
    void        setRating(int value);


    /**
     * Copy database information of this item to a newly created item
     * @param  dstAlbum    destination album
     * @param  dstFileName new filename
     * @return an ImageInfo object of the new item
     */
    //TODO: Move to album?
    KDE_DEPRECATED ImageInfo   copyItem(int dstAlbumID, const QString &dstFileName);

    /**
     * refresh the properties of the imageinfo. it reads the database
     * again for getting the updated date and stats the file to get
     * the updated size 
     */
    void        refresh();

private:

    Q_LLONG               m_ID;
    int                   m_albumID;
    DatabaseUrl           m_url;

    mutable QDateTime     m_datetime;
    mutable QDateTime     m_modDatetime;
    mutable size_t        m_size;
    mutable QSize         m_dims;
};

}  // namespace Digikam

#endif /* IMAGEINFO_H */

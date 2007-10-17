/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : Handling accesss to one image and associated data
 *
 * Copyright 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <QString>
#include <QDateTime>
#include <QList>
#include <QSize>

// KDE includes.

#include <kurl.h>

// Local includes

#include "digikam_export.h"
#include "dshareddata.h"
#include "databaseurl.h"
#include "imagelister.h"
#include "imagelisterrecord.h"

namespace Digikam
{

class ImageInfoData;

/**
 * The ImageInfo class contains provides access to the database for a single image.
 * The properties can be read and written. Information will be cached.
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
     * Constructor. Creates an ImageInfo object without any cached data initially.
     * @param    ID       unique ID for this image
     */
    explicit ImageInfo(qlonglong ID);

    /**
     * Constructor. Creates an ImageInfo object from a file url.
     */
    ImageInfo(const KUrl &url);

    /**
     * Constructor. Creates an ImageInfo object where the provided information
     * will initially be available cached, without database access.
     * @param     ID       unique ID for this image
     * @param     albumID  id of the PAlbum to which this item belongs
     * @param     name     name of the image
     * @param     datetime datetime of the image
     * @param     size     filesize of the image
     * @param     dims     dimensions of the image
     */
    ImageInfo(qlonglong ID, int albumID,
              const QString &album, const QString& albumName,
              const KUrl& albumRoot,
              const QDateTime& datetime,
              uint size,
              const QSize& dims=QSize());

    /**
     * Essentially the same as above
     */
    ImageInfo(const ImageListerRecord &record);

    ImageInfo(const ImageInfo &info);

    /**
     * Destructor
     */
    ~ImageInfo();

    ImageInfo &operator=(const ImageInfo &info);

    bool operator==(const ImageInfo &info) const;
    bool operator<(const ImageInfo &info) const;
    uint hash() const;

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
    uint      fileSize() const;

    /**
     * @return the dimensions of the image (valid only if dimensions
     * have been requested)
     */
    QSize     dimensions() const;

    /**
     * Returns the digikamalbums:// URL.
     * The returned object can be used as a KUrl.
     * Always use this for KIO operations
     */
    DatabaseUrl databaseUrl() const;

    /**
     * Returns the file:// url.
     * This is equivalent to databaseUrl().fileUrl()
     */
    KUrl       fileUrl() const;

    /**
     * Equivalent to fileUrl()
     */
    KDE_DEPRECATED KUrl      kurl() const;

    /**
     * Equivalent to fileUrl().path()
     */
    // Deprecate?
    QString   filePath() const;

    /**
     * Equivalent to databaseUrl()
     */
    KDE_DEPRECATED KUrl      kurlForKIO() const;

    /**
     * @return the unique image id for this item
     */
    qlonglong   id() const;

    /**
     * @return the id of the PAlbum to which this item belongs
     */
    int       albumId() const;

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
    QList<int> tagIds() const;



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

    DSharedDataPointer<ImageInfoData> m_data;
};

inline uint qHash(const ImageInfo &info) { return info.hash(); }

}  // namespace Digikam

#endif /* IMAGEINFO_H */

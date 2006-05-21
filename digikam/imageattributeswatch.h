/* ============================================================
 * File  : imageattributeswatch.h
 * Author: Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2006-05-04
 * Description : Watch image attributes
 * 
 * Copyright 2006 by Marcel Wiesweg
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

#ifndef IMAGEATTRIBUTESWATCH_H
#define IMAGEATTRIBUTESWATCH_H

// Qt includes

#include <qobject.h>

// KDE includes

#include <kurl.h>

namespace Digikam
{

class ImageAttributesWatch : public QObject
{

    Q_OBJECT

public:

    static ImageAttributesWatch *instance();
    static void cleanUp();

    void imageTagsChanged(Q_LLONG imageId);
    void imagesChanged(int albumId);

    void imageRatingChanged(Q_LLONG imageId);
    void imageDateChanged(Q_LLONG imageId);
    void imageCaptionChanged(Q_LLONG imageId);

    void fileMetadataChanged(const KURL &url);

signals:

    /** Indicates that tags have been assigned or removed
        for image with given imageId.
        There is no guarantee that the tags were actually changed.
        This signal, the signal below, or both may be sent.
    */
    void signalImageTagsChanged(Q_LLONG imageId);

    /**
        Indicates that images in the given album id may have changed their tags.
        This signal, the signal above, or both may be sent.
     */
    void signalImagesChanged(int albumId);

    /** These signals indicated that the rating, data or caption
        of the image with given imageId was set.
        There is no guarantee that it actually changed.
    */
    void signalImageRatingChanged(Q_LLONG imageId);
    void signalImageDateChanged(Q_LLONG imageId);
    void signalImageCaptionChanged(Q_LLONG imageId);

    /**
        Indicates that the metadata if the given file
        has been changed (a write operation on the file on disk).
        Usually, the database is updated accordingly, so then this
        signal is sent in combination with one or more of the above signals.
    */
    void signalFileMetadataChanged(const KURL &url);

protected:

    ~ImageAttributesWatch();

    static ImageAttributesWatch *m_instance;
};


}

#endif

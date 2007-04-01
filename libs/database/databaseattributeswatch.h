/* ============================================================
 * File  : imageattributeswatch.h
 * Author: Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2006-03-23
 * Description : Watch database image attributes
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

#ifndef DATABASEATTRIBUTESWATCH_H
#define DATABASEATTRIBUTESWATCH_H

// Qt includes

#include <qobject.h>

// KDE includes

#include <kurl.h>

#include "digikam_export.h"

namespace Digikam
{


/*
    TODO:
    This class is a copy of ImagesAttributesWatch,
    and currently not too cleanly integrated.
    Rethink.
*/
class DIGIKAM_EXPORT DatabaseAttributesWatch : public QObject
{

    Q_OBJECT

public:

    static DatabaseAttributesWatch *instance();
    static void cleanUp();
    static void shutDown();

    void imageTagsChanged(Q_LLONG imageId);
    void imagesChanged(int albumId);

    void imageRatingChanged(Q_LLONG imageId);
    void imageDateChanged(Q_LLONG imageId);
    void imageCaptionChanged(Q_LLONG imageId);

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

protected:

    ~DatabaseAttributesWatch();

    static DatabaseAttributesWatch *m_instance;
};


}

#endif

/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-06-13
 * Description : JPEG files metadata parser.
 * 
 * Copyright 2005 by Renchi Raju
 * Copyright 2006 by Caulier Gilles
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

// KDE includes.

#include <kdebug.h>
#include <kfilemetainfo.h>

// Local includes.

#include "dmetadata.h"
#include "jpegmetadata.h"

namespace Digikam
{

void readJPEGMetaData(const QString& filePath, QString& comments,
                      QDateTime& datetime, int& rating)
{
    comments = QString();
    datetime = QDateTime();
    rating   = 0;
    
    KFileMetaInfo metaInfo(filePath, "image/jpeg", KFileMetaInfo::Fastest);

    if (metaInfo.isValid())
    {
        if (metaInfo.mimeType() == "image/jpeg" &&
            metaInfo.containsGroup("Jpeg EXIF Data"))
        {
            DMetadata metadata(filePath);
        
            // Trying to get comments from image :
            // In first, from standard JPEG comments, or
            // In second, from EXIF comments tag, or
            // In third, from IPTC comments tag.
                
            comments = metadata.getImageComment();
            
            // Trying to get date and time from image :
            // In first, from EXIF date & time tags, or
            // In second, from IPTC date & time tags.
            
            datetime = metadata.getImageDateTime();

            // Trying to get image rating from IPTC Urgency tag.
            
            rating = metadata.getImageRating();
        }
    }
}

} // namespace Digikam

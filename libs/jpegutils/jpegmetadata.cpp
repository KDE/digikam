/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-06-13
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju

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

void readJPEGMetaData(const QString& filePath,
                      QString& comments,
                      QDateTime& datetime)
{
    comments = QString();
    datetime = QDateTime();
    
    KFileMetaInfo metaInfo(filePath, "image/jpeg", KFileMetaInfo::Fastest);

    if (metaInfo.mimeType() == "image/jpeg" && metaInfo.containsGroup("Jpeg EXIF Data"))
    {
        DMetadata metadata(filePath);
    
        // Trying to get comments from image :
        // In first, from standard JPEG comments, or
        // In second, from Exif comments tag, or
        // In third, from IPTC comments tag.
            
        comments = metadata.getImageComment();
        
        // Trying to get date and time from image :
        // In first, from Exif date & time tags, or
        // In second, from IPTC date & time tags.
        
        datetime = metadata.getDateTime();
    }
}

} // namespace Digikam

/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-04-24
 * Description : DMetadata private data class
 *
 * Copyright 2006 by Gilles Caulier
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

#ifndef DMETADATAPRIVATE_H
#define DMETADATAPRIVATE_H

// C++ includes.

#include <string>

// QT includes.

#include <qstring.h>

// Exiv2 includes.

#include <exiv2/iptc.hpp>
#include <exiv2/exif.hpp>

// Local includes.

#include "dimg.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DMetadataPriv
{
public:

    DMetadataPriv()
    {
        fileFormat = DImg::NONE;
    }

    QString         filePath;

    DImg::FORMAT    fileFormat;

    std::string     imageComments;  

    Exiv2::ExifData exifMetadata;
    Exiv2::IptcData iptcMetadata;

};

}  // NameSpace Digikam

#endif /* DMETADATAPRIVATE_H */

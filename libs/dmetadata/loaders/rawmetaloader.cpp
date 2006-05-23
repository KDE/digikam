/* ============================================================
 * Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-23
 * Description : RAW file metadata loader
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

// Qt includes.

#include <qfile.h>

// Local includes.

#include "rawmetaloader.h"

namespace Digikam
{

RAWMetaLoader::RAWMetaLoader(DMetadata* metadata)
             : DMetaLoader(metadata)
{
}

RAWMetaLoader::~RAWMetaLoader()
{
}

bool RAWMetaLoader::load(const QString& filePath)
{
    // In first we trying to use Exiv2 library
    // Exiv2 0.10 support : DNG, CRW, CR2, NEF, PEF, MRW, SR2
    if (loadWithExiv2(filePath))
        return true;

    // TODO new experimental RAW parser can be added here.
    return false;
}

bool RAWMetaLoader::save(const QString& filePath)
{
    if (saveWithExiv2(filePath))
        return true;
        
    return false;
}

}  // NameSpace Digikam

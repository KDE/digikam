/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-23
 * Description : image metadata loader interface
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

// C++ includes.

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>

// Qt includes.

#include <qfile.h>

// LibExiv2 includes.

#include <exiv2/image.hpp>
#include <exiv2/exif.hpp>
#include <exiv2/iptc.hpp>

// Local includes.

#include "ddebug.h"
#include "dmetadataprivate.h"
#include "dmetadata.h"
#include "dmetaloader.h"

namespace Digikam
{

DMetaLoader::DMetaLoader(DMetadata* metadata)
{
    m_metadata    = metadata;
    m_hasExif     = false;
    m_hasIptc     = false;
    m_hasComments = false;
}

Exiv2::ExifData& DMetaLoader::exifMetadata()
{
    return m_metadata->d->exifMetadata;
}

Exiv2::IptcData& DMetaLoader::iptcMetadata()
{
    return m_metadata->d->iptcMetadata;
}

std::string& DMetaLoader::imageComments()
{
    return m_metadata->d->imageComments;
}

bool DMetaLoader::loadWithExiv2(const QString& filePath)
{
    try
    {    
        if (filePath.isEmpty())
            return false;

        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const char*)
                                      (QFile::encodeName(filePath)));
        image->readMetadata();

        // Image comments ---------------------------------

        imageComments() = image->comment();

        if (!imageComments().empty())
            m_hasComments = true;

        // Exif metadata ----------------------------------
        
        exifMetadata() = image->exifData();

        if (!exifMetadata().empty())
            m_hasExif = true;

        // Iptc metadata ----------------------------------
        
        iptcMetadata() = image->iptcData();
        
        if (!iptcMetadata().empty())
            m_hasIptc = true;
    
        return true;
    }
    catch( Exiv2::Error &e )
    {
        DDebug() << "Cannot load metadata using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
        return false;
    }
}

bool DMetaLoader::saveWithExiv2(const QString& filePath)
{
    try
    {    
        if (filePath.isEmpty())
            return false;

        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const char*)
                                      (QFile::encodeName(filePath)));
        
        // Image Comments ---------------------------------
        
        if (!imageComments().empty())
        {
            image->setComment(imageComments());
        }

        // Exif metadata ----------------------------------
        
        if (!exifMetadata().empty())
        {
            image->setExifData(exifMetadata());
        }

        // Iptc metadata ----------------------------------
        
        if (!iptcMetadata().empty())
        {
            image->setIptcData(iptcMetadata());
        }
    
        image->writeMetadata();

        return true;
    }
    catch( Exiv2::Error &e )
    {
        DDebug() << "Cannot save metadata using Exiv2 ("
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
        return false;
    }
}

}  // NameSpace Digikam

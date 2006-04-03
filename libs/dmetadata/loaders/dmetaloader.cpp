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

// KDE includes.

#include <kdebug.h>

// LibExiv2 includes.

#include <exiv2/image.hpp>
#include <exiv2/exif.hpp>
#include <exiv2/iptc.hpp>

// Local includes.

#include "dmetadata.h"
#include "dmetaloader.h"

namespace Digikam
{

DMetaLoader::DMetaLoader(DMetadata* metadata)
{
    m_metadata = metadata;
    m_hasExif  = false;
    m_hasIptc  = false;
}

QByteArray& DMetaLoader::exifMetadata()
{
    return m_metadata->m_exifMetadata;
}

QByteArray& DMetaLoader::iptcMetadata()
{
    return m_metadata->m_iptcMetadata;
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

        // Exif metadata ----------------------------------
        
        Exiv2::ExifData &exifData = image->exifData();
        Exiv2::DataBuf const c1(exifData.copy());
        exifMetadata() = QByteArray(c1.size_);
        memcpy(exifMetadata().data(), c1.pData_, c1.size_);

        if (!exifMetadata().isEmpty())
            m_hasExif = true;

        // Iptc metadata ----------------------------------
        
        Exiv2::IptcData &iptcData = image->iptcData();
        Exiv2::DataBuf const c2(iptcData.copy());
        iptcMetadata() = QByteArray(c2.size_);
        memcpy(iptcMetadata().data(), c2.pData_, c2.size_);
        
        if (!iptcMetadata().isEmpty())
            m_hasIptc = true;
    
        return true;
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot load metadata using Exiv2 (" 
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
        
        // To prevent lost other metadata like image comments for ex.
        image->readMetadata(); 

        // Exif metadata ----------------------------------
        
        if (!exifMetadata().isEmpty())
        {
            Exiv2::ExifData exifData;

            if (exifData.load((Exiv2::byte*)exifMetadata().data(), exifMetadata().size()))
                kdDebug() << "Cannot parse EXIF metadata to save using Exiv2" << endl;
            else
                image->setExifData(exifData);
        }

        // Iptc metadata ----------------------------------
        
        if (!iptcMetadata().isEmpty())
        {
            Exiv2::IptcData iptcData;

            if (iptcData.load((Exiv2::byte*)iptcMetadata().data(), iptcMetadata().size()))
                kdDebug() << "Cannot parse IPTC metadata to save using Exiv2" << endl;
            else
                image->setIptcData(iptcData);
        }
    
        image->writeMetadata();

        return true;
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot save metadata using Exiv2 ("
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
        return false;
    }
}

}  // NameSpace Digikam

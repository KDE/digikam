/* ============================================================
 * Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-23
 * Description : TIFF file metadata loader
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

// C ansi includes.

extern "C" 
{
#include <tiffio.h>
#include <tiff.h>
}

// C++ includes.

#include <cstdio>

// Qt includes.

#include <qfile.h>

// KDE includes.

#include <kdebug.h>

// LibExiv2 includes.

#include <exiv2/exif.hpp>
#include <exiv2/tags.hpp>
#include <exiv2/iptc.hpp>
#include <exiv2/datasets.hpp>

// Local includes.

#include "tiffmetaloader.h"

namespace Digikam
{

TIFFMetaLoader::TIFFMetaLoader(DMetadata* metadata)
              : DMetaLoader(metadata)
{
}

bool TIFFMetaLoader::load(const QString& filePath)
{
    // -------------------------------------------------------------------
    // Open the file
    
    TIFF* tif = TIFFOpen(QFile::encodeName(filePath), "r");
    if (!tif)
    {
        kdDebug() << k_funcinfo << "Cannot open image file." << endl;
        return false;
    }

    // -------------------------------------------------------------------
    // Get text meta-data markers contents.

    try
    {    
        uchar *textData = 0;
        Exiv2::IptcData iptcData;
                
        if(TIFFGetField(tif, TIFFTAG_SOFTWARE, &textData))
        {
            QCString txtValue((const char*)textData, 32);
            iptcData["Iptc.Application2.Program"] = std::string(txtValue);
        }

        if(TIFFGetField(tif, TIFFTAG_ARTIST, &textData))
        {
            QCString txtValue((const char*)textData, 32);
            iptcData["Iptc.Application2.Byline"] = std::string(txtValue);
        }
        
        if(TIFFGetField(tif, TIFFTAG_COPYRIGHT, &textData))
        {
            QCString txtValue((const char*)textData, 128);
            iptcData["Iptc.Application2.Copyright"] = std::string(txtValue);
        }

        if(TIFFGetField(tif, TIFFTAG_DATETIME, &textData))
        {
            QCString txtValue((const char*)textData, 128);
            iptcData["Iptc.Application2.TimeCreated"] = std::string(txtValue);
        }

        if(TIFFGetField(tif, TIFFTAG_DATETIME, &textData))
        {
            QCString txtValue((const char*)textData, 11);
            iptcData["Iptc.Application2.TimeCreated"] = std::string(txtValue);
        }

        if(TIFFGetField(tif, TIFFTAG_DOCUMENTNAME, &textData))
        {
            QCString txtValue((const char*)textData, 64);
            iptcData["Iptc.Application2.ObjectName"] = std::string(txtValue);
        }

        if(TIFFGetField(tif, TIFFTAG_IMAGEDESCRIPTION, &textData))
        {
            QCString txtValue((const char*)textData, 2000);
            iptcData["Iptc.Application2.Caption"] = std::string(txtValue);
        }

        if(TIFFGetField(tif, TIFFTAG_MAKE, &textData))
        {
            QCString txtValue((const char*)textData, 32);
            iptcData["Iptc.Application2.Source"] = std::string(txtValue);
        }

        if(TIFFGetField(tif, TIFFTAG_MODEL, &textData))
        {
            QCString txtValue((const char*)textData, 32);
            iptcData["Iptc.Application2.Writer"] = std::string(txtValue);
        }

        if(TIFFGetField(tif, TIFFTAG_PAGENAME))
        {
            QCString txtValue((const char*)textData, 236);
            iptcData["Iptc.Application2.Subject"] = std::string(txtValue);
        }
        
        Exiv2::DataBuf const c2(iptcData.copy());
        iptcMetadata() = QByteArray(c2.size_);
        memcpy(iptcMetadata().data(), c2.pData_, c2.size_);
    
        if (!iptcMetadata().isEmpty())
            m_hasIptc = true;
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Exiv2 Exception (" << e.code() << ")" << endl;
    }

    TIFFClose(tif);

    return true;
}

bool TIFFMetaLoader::save(const QString& /*filePath*/)
{
    // TODO
    return false;
}

}  // NameSpace Digikam

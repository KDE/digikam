/* ============================================================
 * Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-23
 * Description : PNG file metadata loader
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

// C Ansi includes.

extern "C"
{
#include <unistd.h>
}

// C++ includes.

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>

// Qt includes.

#include <qfile.h>

// KDE includes.

#include <kfilemetainfo.h>
#include <kdebug.h>

// LibExiv2 includes.

#include <exiv2/iptc.hpp>
#include <exiv2/datasets.hpp>

// Local includes.

#include "pngmetaloader.h"

namespace Digikam
{

PNGMetaLoader::PNGMetaLoader(DMetadata* metadata)
             : DMetaLoader(metadata)
{
}

bool PNGMetaLoader::load(const QString& filePath)
{
    KFileMetaInfo metainfo(filePath);

    if (metainfo.isValid() && !metainfo.isEmpty() )
    {
        QStringList keys = metainfo.preferredKeys();

        // -------------------------------------------------------------------
        // Get embbeded text data and metadata.

        // Extra Raw profiles tag are used by ImageMagick and defines at this URL :
        // http://search.cpan.org/src/EXIFTOOL/Image-ExifTool-5.87/html/TagNames/PNG.html#TextualData

        // In FIRST we check if we have a Raw profile embedded using ImageMagick technic.

        for (QStringList::iterator it = keys.begin() ; it!=keys.end() ; ++it)
        {
            KFileMetaInfoItem item = metainfo.item( *it );
            if ( item.isValid() )
            {
                QString s = item.string();
                QString k = item.key();

                if ( k == "Raw profile type exif" || k == "Raw profile type APP1" )
                {
                    uint length = s.length();
                    uchar *data = readRawProfile(s.ascii(), &length);
                    if (!data)
                        continue;
                    // We removing standard Exif header
                    exifMetadata().load((const Exiv2::byte*)data+6, length-6);
                    delete [] data;
                    
                    if (!exifMetadata().empty())
                        m_hasExif = true;
                }
                
                if ( k == "Raw profile type iptc" )
                {
                    uint length = s.length();
                    uchar *data = readRawProfile(s.ascii(), &length);
                    if (!data)
                        continue;
                    iptcMetadata().load((const Exiv2::byte*)data, length);
                    delete [] data;

                    if (!iptcMetadata().empty())
                        m_hasIptc = true;
                }
            }
        }
    
        // In SECOND, we check all others embedded text.
    
        // Standard Embedded text includes in PNG :
    
        // Title            Short (one line) title or caption for image
        // Author           Name of image's creator
        // Description      Description of image (possibly long)
        // Copyright        Copyright notice
        // Creation Time    Time of original image creation
        // Software         Software used to create the image
        // Disclaimer       Legal disclaimer
        // Warning          Warning of nature of content
        // Source           Device used to create the image
        // Comment          Miscellaneous comment; conversion from GIF comment
    
        // Non-standard embedded text includes in PNG provide by any software :
    
        // Artist           string
        // Document         string
        // Label            string
        // Make             string
        // Model            string
        // TimeStamp        string
        // URL              string
    
        try
        {
            if (!m_hasIptc)
            {
                QString pngKey("Iptc.");
                Exiv2::IptcData iptcData;
    
                for (QStringList::iterator it = keys.begin() ; it!=keys.end() ; ++it)
                {
                    KFileMetaInfoItem item = metainfo.item( *it );
                    if ( item.isValid() )
                    {
                        QString value = item.key();
                        int maxCharSize;
        
                        // Standard embbeded text layout:
                        if      (value == "Title")         { pngKey.append("Application2.Subject");             maxCharSize = 236;  }
                        else if (value == "Author")        { pngKey.append("Application2.Headline");            maxCharSize = 256;  }
                        else if (value == "Description")   { pngKey.append("Application2.Caption");             maxCharSize = 2000; }
                        else if (value == "Copyright")     { pngKey.append("Application2.Copyright");           maxCharSize = 128;  }
                        else if (value == "Creation Time") { pngKey.append("Application2.TimeCreated");         maxCharSize = 11;   }
                        else if (value == "Software")      { pngKey.append("Application2.Program");             maxCharSize = 32;   }
                        else if (value == "Disclaimer")    { pngKey.append("Application2.Credit");              maxCharSize = 32;   }
                        else if (value == "Warning")       { pngKey.append("Application2.SpecialInstructions"); maxCharSize = 256;  }
                        else if (value == "Source")        { pngKey.append("Application2.Byline");              maxCharSize = 32;   }
                        else if (value == "Comment")       { pngKey.append("Application2.Keywords");            maxCharSize = 64;   }
        
                        // Non-standard embbeded text layout:
                        else if (value == "Artist")        { pngKey.append("Application2.BylineTitle");         maxCharSize = 32;   }
                        else if (value == "Document")      { pngKey.append("Application2.ObjectName");          maxCharSize = 64;   }
                        else if (value == "Label")         { pngKey.append("Application2.FixtureId");           maxCharSize = 32;   }
                        else if (value == "Make")          { pngKey.append("Application2.Source");              maxCharSize = 32;   }
                        else if (value == "Model")         { pngKey.append("Application2.Writer");              maxCharSize = 32;   }
                        else if (value == "TimeStamp")     { pngKey.append("Application2.DateCreated");         maxCharSize = 8;    }
                        else if (value == "URL")           { pngKey.append("Application2.EditStatus");          maxCharSize = 64;   }
                        else continue;
        
                        QString txtValue = item.string();
                        txtValue.truncate(maxCharSize);
                        kdDebug() << pngKey << "=" << txtValue << endl;
                        iptcData[pngKey.ascii()] = std::string(txtValue.ascii());
                    }
    
                iptcMetadata() = iptcData;
        
                if (!iptcMetadata().empty())
                    m_hasIptc = true;
                }
            }
        }
        catch( Exiv2::Error &e )
        {
            kdDebug() << "Exiv2 Exception (" << e.code() << ")" << endl;
            return false;
        }

        return true;
    }
    
    return false;
}

bool PNGMetaLoader::save(const QString& /*filePath*/)
{
    return false;
}

uchar* PNGMetaLoader::readRawProfile(const char* text, uint *length)
{
    uchar          *info = 0;

    register long   i;

    register uchar *dp;

    const char     *sp;

    uint            nibbles;

    unsigned char   unhex[103]={0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,1, 2,3,4,5,6,7,8,9,0,0,
                                0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,10,11,12,
                                13,14,15};

    sp = text+1;

    // Look for newline

    while (*sp != '\n')
        sp++;

    // Look for length

    while (*sp == '\0' || *sp == ' ' || *sp == '\n')
        sp++;

    *length = (uint) atol(sp);

    while (*sp != ' ' && *sp != '\n')
        sp++;

    // Allocate space

    if (*length == 0)
    {
        kdDebug() << "Unable To Copy Raw Profile: invalid profile length"  << endl;
        return 0;
    }

    info = new uchar[*length];

    if (!info)
    {
        kdDebug() << "Unable To Copy Raw Profile: cannot allocate memory"  << endl;
        return 0;
    }

    // Copy profile, skipping white space and column 1 "=" signs

    dp      = info;
    nibbles = *length * 2;

    for (i = 0; i < (long) nibbles; i++)
    {
        while (*sp < '0' || (*sp > '9' && *sp < 'a') || *sp > 'f')
        {
            if (*sp == '\0')
            {
                kdDebug() << "Unable To Copy Raw Profile: ran out of data" << endl;
                return 0;
            }

            sp++;
        }

        if (i%2 == 0)
            *dp = (uchar) (16*unhex[(int) *sp++]);
        else
            (*dp++) += unhex[(int) *sp++];
    }

    return info;
}

}  // NameSpace Digikam

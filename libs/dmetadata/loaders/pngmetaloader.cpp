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
    // In first we trying to use Exiv2 library
    // Exiv2 0.11 will support PNG
    if (loadWithExiv2(filePath))
        return true;

    KFileMetaInfo metainfo(filePath);

    if (metainfo.isValid() && !metainfo.isEmpty() )
    {
        QStringList keys = metainfo.preferredKeys();

        // ***********************************************************
        // TODO : removing this part when Exiv2 0.11 will be released.

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
                    if (data)
                    {
                        char exifHeader[] = { 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };
                        QByteArray exifData(length);
                        memcpy(exifData.data(), data, length);
                        
                        if (!exifData.isEmpty())
                        {                    
                            int i = exifData.find(*exifHeader);
                            if (i != -1)
                            {
                                kdDebug() << filePath << " : Exif header found at position " << i << endl;
                                i = i + sizeof(exifHeader);
                                exifMetadata().load((const Exiv2::byte*)data+i, exifData.size()-i);
                            }
                        }
                        
                        delete [] data;
                        
                        if (!exifMetadata().empty())
                            m_hasExif = true;
                    }
                }
                
                if ( k == "Raw profile type iptc" )
                {
                    uint length = s.length();
                    uchar *data = readRawProfile(s.ascii(), &length);
                    if (data)
                    {
                        iptcMetadata().load((const Exiv2::byte*)data, length);
                        delete [] data;
    
                        if (!iptcMetadata().empty())
                            m_hasIptc = true;
                    }
                }
            }
        }
        // ***********************************************************

        // In SECOND, we check all others embedded text.
        //
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
            if (!m_hasExif)
            {
                Exiv2::ExifData exifData;
    
                // Embedded Text tags.
                
                for (QStringList::iterator it = keys.begin() ; it!=keys.end() ; ++it)
                {
                    QString exifKey("Exif.");
                    KFileMetaInfoItem item = metainfo.item( *it );
                    
                    if ( item.isValid() )
                    {
                        QString value = item.key();
        
                        // Standard Exif tags:
                        if      (value == "Author")        { exifKey.append("Image.Artist");           }
                        else if (value == "Description")   { exifKey.append("Image.ImageDescription"); }
                        else if (value == "Copyright")     { exifKey.append("Image.Copyright");        }
                        else if (value == "Creation Time") { exifKey.append("Image.DateTime");         }
                        else if (value == "Software")      { exifKey.append("Image.Software");         }
                        else if (value == "Source")        { exifKey.append("Image.Model");            }
                        else if (value == "Make")          { exifKey.append("Image.Make");             }
                        else if (value == "Title")         { exifKey.append("Image.DocumentName");     }
                        else if (value == "Comment")       { exifKey.append("Photo.UserComment");      }

                        // Advanced Exif tags rules:
                        
                        else if (value == "Disclaimer" &&
                            (exifData.findKey(Exiv2::ExifKey("Exif.Image.Copyright")) == exifData.end()))
                                { exifKey.append("Image.Copyright"); }

                        else if (value == "Warning" &&
                            (exifData.findKey(Exiv2::ExifKey("Exif.Image.DocumentName")) == exifData.end()))
                                { exifKey.append("Image.DocumentName"); }

                        else if (value == "Artist" &&
                            (exifData.findKey(Exiv2::ExifKey("Exif.Image.Artist")) == exifData.end()))
                                { exifKey.append("Image.Artist"); }

                        else if (value == "Document" &&
                            (exifData.findKey(Exiv2::ExifKey("Exif.Image.DocumentName")) == exifData.end()))
                                { exifKey.append("Image.DocumentName"); }

                        else if (value == "Label" &&
                            (exifData.findKey(Exiv2::ExifKey("Exif.Image.ImageDescription")) == exifData.end()))
                                { exifKey.append("Image.ImageDescription"); }
                        
                        else if (value == "Model" &&
                            (exifData.findKey(Exiv2::ExifKey("Exif.Image.Model")) == exifData.end()))
                                { exifKey.append("Image.Model"); }

                        else if (value == "URL" &&
                            (exifData.findKey(Exiv2::ExifKey("Exif.Image.DocumentName")) == exifData.end()))
                                { exifKey.append("Image.DocumentName"); }

                        else if (value == "TimeStamp" &&
                            (exifData.findKey(Exiv2::ExifKey("Exif.Image.DateTime")) == exifData.end()))
                                { exifKey.append("Image.DateTime"); }
        
                        if (exifKey != "Exif.")
                        {
                            QString txtValue = item.string();
                            kdDebug() << exifKey << "=" << txtValue << endl;
                            exifData[exifKey.ascii()] = std::string(txtValue.ascii());
                        }
                    }
                }
                 
                // Image technical informations.
                   
                for (QStringList::iterator it = keys.begin() ; it!=keys.end() ; ++it)
                {
                    KFileMetaInfoItem item = metainfo.item( *it );
                    
                    if ( item.isValid() )
                    {
                        if (item.key() == "Dimensions")
                        {
                            exifData["Exif.Image.ImageWidth"]      = item.value().toSize().width();
                            exifData["Exif.Image.ImageLength"]     = item.value().toSize().height();
                            exifData["Exif.Photo.PixelXDimension"] = item.value().toSize().width();
                            exifData["Exif.Photo.PixelYDimension"] = item.value().toSize().height();
                        }
                        
                        if (item.key() == "BitDepth")
                            exifData["Exif.Image.BitsPerSample"] = item.value().toInt();
                            
                            
                        // KfileMetaInfo return a string about ColorMode and Compression values. 
                        // We cannot translate these tags to Exif.
                    }
                }
                                
                exifMetadata() = exifData;
                        
                if (!exifMetadata().empty())
                    m_hasExif = true;
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

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

#define PNG_BYTES_TO_CHECK 4

/* Needed to access various internal PNG chunk routines */
#define PNG_INTERNAL

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
    FILE        *f;
    png_structp  png_ptr  = NULL;
    png_infop    info_ptr = NULL;

    // -------------------------------------------------------------------
    // Open the file

    f = fopen(QFile::encodeName(filePath), "rb");
    if ( !f )
        return false;

    unsigned char buf[PNG_BYTES_TO_CHECK];

    fread(buf, 1, PNG_BYTES_TO_CHECK, f);
    if (!png_check_sig(buf, PNG_BYTES_TO_CHECK))
    {
        fclose(f);
        return false;
    }
    rewind(f);

    // -------------------------------------------------------------------
    // Initialize the internal structures

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        fclose(f);
        return false;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(f);
        return false;
    }

    if (setjmp(png_ptr->jmpbuf))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(f);
        return false;
    }

    png_init_io(png_ptr, f);

    // -------------------------------------------------------------------
    // Read all PNG info up to image data

    png_read_info(png_ptr, info_ptr);

    png_skip_till_end(png_ptr, info_ptr);

    // -------------------------------------------------------------------
    // Get embbeded text data and metadata.

    /*
    Extra Raw profiles tag are used by ImageMagick and defines at this URL :
    http://search.cpan.org/src/EXIFTOOL/Image-ExifTool-5.87/html/TagNames/PNG.html#TextualData
    */    

    png_text* text_ptr;
    int num_comments = png_get_text(png_ptr, info_ptr, &text_ptr, NULL);

    // In first we check if we have a Raw profile embedded using ImageMagick technic.

    for (int i = 0; i < num_comments; i++)
    {
        if ( (QString(text_ptr[i].key) == "Raw profile type exif") ||
             (QString(text_ptr[i].key) == "Raw profile type APP1") )
        {
            png_uint_32 length;
            uchar *data = readRawProfile(text_ptr, &length, i);
            // We removing standard Exif header
            exifMetadata() = QByteArray(length-6);
            memcpy(exifMetadata().data(), data+6, length-6);
            delete [] data;
            
            if (!exifMetadata().isEmpty())
                m_hasExif = true;
        }

        if (QString(text_ptr[i].key) == "Raw profile type iptc")
        {
            png_uint_32 length;
            uchar *data = readRawProfile(text_ptr, &length, i);
            iptcMetadata() = QByteArray(length);
            memcpy(iptcMetadata().data(), data, length);
            delete [] data;
            
            if (!iptcMetadata().isEmpty())
                m_hasIptc = true;
        }
    }
    
    // In second, we check all others embedded text.

    /*
    Standard Embedded text includes in PNG :
    
    Title            Short (one line) title or caption for image
    Author           Name of image's creator
    Description      Description of image (possibly long)
    Copyright        Copyright notice
    Creation Time    Time of original image creation
    Software         Software used to create the image
    Disclaimer       Legal disclaimer
    Warning          Warning of nature of content
    Source           Device used to create the image
    Comment          Miscellaneous comment; conversion from GIF comment

    Non-standard embedded text includes in PNG provide by any software :

    Artist           string 
    Document         string 
    Label            string 
    Make             string 
    Model            string 
    TimeStamp        string 
    URL              string 
    
    */
    
    try
    {    
        if (!m_hasIptc)
        {    
            QString pngKey("Iptc.");
            Exiv2::IptcData iptcData;
        
            for (int i = 0; i < num_comments; i++)
            {
                QString value(text_ptr[i].key);
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
                    
                QString txtValue = QString(text_ptr[i].text);
                txtValue.truncate(maxCharSize);
                kdDebug() << pngKey << "=" << txtValue << endl;
                iptcData[pngKey.ascii()] = std::string(txtValue.ascii());
            }
        
        Exiv2::DataBuf const c2(iptcData.copy());
        iptcMetadata() = QByteArray(c2.size_);
        memcpy(iptcMetadata().data(), c2.pData_, c2.size_);

        if (!iptcMetadata().isEmpty())
            m_hasIptc = true;
        }
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Exiv2 Exception (" << e.code() << ")" << endl;
    }
    
    // -------------------------------------------------------------------

    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
    png_destroy_info_struct(png_ptr, (png_infopp) & info_ptr);

    fclose(f);
    return true;
}

bool PNGMetaLoader::save(const QString& filePath)
{
    return false;
}

uchar* PNGMetaLoader::readRawProfile(png_textp text, png_uint_32 *length, int ii)
{
    uchar          *info = 0;

    register long   i;

    register uchar *dp;

    register        png_charp sp;

    png_uint_32     nibbles;

    unsigned char unhex[103]={0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,0,1, 2,3,4,5,6,7,8,9,0,0,
                              0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,10,11,12,
                              13,14,15};

    sp = text[ii].text+1;

    // Look for newline

    while (*sp != '\n')
        sp++;

    // Look for length

    while (*sp == '\0' || *sp == ' ' || *sp == '\n')
        sp++;

    *length = (png_uint_32) atol(sp);

    while (*sp != ' ' && *sp != '\n')
        sp++;

    // Allocate space

    if (*length == 0)
    {
        kdDebug() << "Unable To Copy Raw Profile: invalid profile length"  << endl;
        return (false);
    }

    info = new uchar[*length];

    if (!info)
    {
        kdDebug() << "Unable To Copy Raw Profile: cannot allocate memory"  << endl;
        return (false);
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
                return (false);
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

/* read data, ignoring IDATs, till the end of the png file.
   
   Will not read past the end of the file, will verify the end is
   accurate, and will read any comments or time information at the
   end of the file, if info is not NULL. */
   
void PNGMetaLoader::png_skip_till_end(png_structp png_ptr, png_infop info_ptr)
{
    png_byte chunk_length[4];
    png_uint_32 length;
    
    length=png_ptr->idat_size;
    
    /* Skip IDAT chunks */
    do
    {
        png_crc_finish(png_ptr, length);
    
        png_read_data(png_ptr, chunk_length, 4);
        length = png_get_uint_32(chunk_length);
    
        png_reset_crc(png_ptr);
        png_crc_read(png_ptr, png_ptr->chunk_name, 4);
    }
    while (!png_memcmp(png_ptr->chunk_name, png_IDAT, 4));
   
    png_ptr->mode |= PNG_AFTER_IDAT;
    
    do
    {
        if (!png_memcmp(png_ptr->chunk_name, png_IHDR, 4))
            png_handle_IHDR(png_ptr, info_ptr, length);
        else if (!png_memcmp(png_ptr->chunk_name, png_IDAT, 4))
        {
        /* Zero length IDATs are legal after the last IDAT has been
         * read, but not after other chunks have been read.
         */
        if (length > 0 || png_ptr->mode & PNG_AFTER_IDAT)
            png_error(png_ptr, "Too many IDAT's found");
        else
            png_crc_finish(png_ptr, 0);
        }
#if defined(PNG_READ_tIME_SUPPORTED)
        else if (!png_memcmp(png_ptr->chunk_name, png_tIME, 4))
            png_handle_tIME(png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_tEXt_SUPPORTED)
        else if (!png_memcmp(png_ptr->chunk_name, png_tEXt, 4))
            png_handle_tEXt(png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_zTXt_SUPPORTED)
        else if (!png_memcmp(png_ptr->chunk_name, png_zTXt, 4))
            png_handle_zTXt(png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_iTXt_SUPPORTED)
        else if (!png_memcmp(png_ptr->chunk_name, png_iTXt, 4))
            png_handle_iTXt(png_ptr, info_ptr, length);
#endif
        else if (!png_memcmp(png_ptr->chunk_name, png_IEND, 4))
            png_handle_IEND(png_ptr, info_ptr, length);
        else
            png_handle_unknown(png_ptr, info_ptr, length);
    
        if (!(png_ptr->mode & PNG_HAVE_IEND))
        {
            png_read_data(png_ptr, chunk_length, 4);
            length = png_get_uint_32(chunk_length);
            
            png_reset_crc(png_ptr);
            png_crc_read(png_ptr, png_ptr->chunk_name, 4);
        }
    }
    while (!(png_ptr->mode & PNG_HAVE_IEND));
}


}  // NameSpace Digikam

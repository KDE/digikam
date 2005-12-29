/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2005-11-01
 * Description : A digital camera RAW files loader for DImg 
 *               framework using dcraw program.
 *
 * Copyright 2005 by Gilles Caulier
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

// This line must be commented to prevent any latency time
// when we use threaded image loader interface for each image
// files io. Uncomment this line only for debugging.
//#define ENABLE_DEBUG_MESSAGES 

extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "iccjpeg.h"
}

// QT includes.

#include <qfile.h>
#include <qcstring.h>
#include <qimage.h>

// KDE includes.

#include <kdebug.h>
#include <kprocess.h>

// Local includes.

#include "dimg.h"
#include "dcraw_parse.h"
#include "rawloader.h"

namespace Digikam
{

RAWLoader::RAWLoader(DImg* image)
         : DImgLoader(image)
{
    m_hasAlpha   = false;
}

bool RAWLoader::load(const QString& filePath)
{
    return ( load16bits(filePath) );
}

bool RAWLoader::load8bits(const QString& filePath)
{
    int  width, height, rgbmax;
    char nl;

    QCString command;

    // run dcraw with options:
    // -c : write to stdout
    // -q : Use simple bilinear interpolation for quick results
    // -2 : 8bit ppm output
    // -w : Use camera white balance, if possible  
    // -a : Use automatic white balance
    command  = "dcraw -c -q -2 -w -a ";
    command += QFile::encodeName( KProcess::quote( filePath ) );

#ifdef ENABLE_DEBUG_MESSAGES
    kdDebug() << "Running dcraw command : " << command << endl;
#endif

    FILE* f = popen( command.data(), "r" );

    if ( !f )
    {
        kdDebug() << "dcraw program unavailable." << endl;
        return false;
    }

    if (fscanf (f, "P6 %d %d %d%c", &width, &height, &rgbmax, &nl) != 4) 
    {
        kdDebug() << "Not a raw digital camera image." << endl;
        pclose (f);
        return false;
    }

    uchar *data = new uchar[width*height*4];
    uchar *dst = data;
    uchar src[3];

    for (int i = 0; i < width*height; i++)
    {
        fread (src, 3 *sizeof(uchar), 1, f);

        // Swap byte order to preserve compatibility with PPC.

        if (QImage::systemByteOrder() == QImage::BigEndian)
            swab((const char *) src, (char *) src, 3 *sizeof(uchar));

        // No need to adapt RGB components accordinly with rgbmax value because dcraw
        // always return rgbmax to 255 in 8 bits/color/pixels.

        dst[0] = src[2];    // Blue
        dst[1] = src[1];    // Green
        dst[2] = src[0];    // Red
        dst[3] = 0xFF;      // Alpha

        dst += 4;
    }

    pclose( f );

    //----------------------------------------------------------

    char       model[256], constructor[256];
    DcrawParse rawFileParser;
    
    if ( rawFileParser.getCameraModel(QFile::encodeName(filePath), &constructor[0], &model[0]) == 0 )
    {
        imageSetCameraModel(QString::QString(model));
        imageSetCameraConstructor(QString::QString(constructor));
    }

    //----------------------------------------------------------

    m_sixteenBit  = false;
    imageWidth()  = width;
    imageHeight() = height;
    imageData()   = data;
    imageSetAttribute("format", "RAW");
    
    return true;
}

bool RAWLoader::load16bits(const QString& filePath)
{
    int	 width, height, rgbmax;
    char nl;

    QCString command;

    // run dcraw with options:
    // -c : write to stdout
    // -q : Use simple bilinear interpolation for quick results
    // -4 : 16bit ppm output
    // -a : Use automatic white balance
    // -w : Use camera white balance, if possible
    command  = "dcraw -c -q -4 -w -a ";
    command += "'";
    command += QFile::encodeName( filePath );
    command += "'";

#ifdef ENABLE_DEBUG_MESSAGES
    kdDebug() << "Running dcraw command " << command << endl;
#endif

    FILE* f = popen( command.data(), "r" );

    if ( !f )
    {
        kdDebug() << "dcraw program unavailable." << endl;
        return false;
    }

    if (fscanf (f, "P6 %d %d %d%c", &width, &height, &rgbmax, &nl) != 4) 
    {
        kdDebug() << "Not a raw digital camera image." << endl;
        pclose (f);
        return false;
    }

    unsigned short *data = new unsigned short[width*height*4];
    unsigned short *dst  = data;
    uchar src[6];
    float fac = 65535.0 / rgbmax;

    for (int i = 0; i < width*height; i++)
    { 
        fread (src, 6 *sizeof(unsigned char), 1, f);

        // Swap byte order to preserve compatibility with PPC.

        if (QImage::systemByteOrder() == QImage::BigEndian)
            swab((const uchar *) src, (uchar *) src, 6*sizeof(uchar));

        dst[0] = (unsigned short)((src[4]*256 + src[5]) * fac);      // Blue
        dst[1] = (unsigned short)((src[2]*256 + src[3]) * fac);      // Green
        dst[2] = (unsigned short)((src[0]*256 + src[1]) * fac);      // Red
        dst[3] = 0xFFFF;

        dst += 4;
    }

    pclose( f );

    //----------------------------------------------------------

    char model[256], constructor[256];
    DcrawParse rawFileParser;
    
    if ( rawFileParser.getCameraModel(QFile::encodeName(filePath), constructor, model) == 0 )
    {
        imageSetCameraModel(QString::QString(model));
        imageSetCameraConstructor(QString::QString(constructor));
    }

    //----------------------------------------------------------

    m_sixteenBit  = true;
    imageWidth()  = width;
    imageHeight() = height;
    imageData()   = (uchar *)data;
    imageSetAttribute("format", "RAW");

/*
    if (!getICCProfileFromTIFF(filePath))
       getICCProfileFromJPEG(filePath);
*/

    return true;
}

bool RAWLoader::getICCProfileFromJPEG(const QString& filePath)
{
    FILE *file = fopen(QFile::encodeName(filePath), "rb");
    if (!file)
        return false;

    fseek(file, 0L, SEEK_SET);
    struct jpeg_decompress_struct cinfo;
    struct dimg_jpeg_error_mgr    jerr;

    // -------------------------------------------------------------------
    // JPEG error handling.

    cinfo.err = jpeg_std_error(&jerr);

    // If an error occurs during reading, libjpeg will jump here

    if (setjmp(jerr.setjmp_buffer)) 
    {
        jpeg_destroy_decompress(&cinfo);
        fclose(file);
        return false;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, file);
    jpeg_read_header(&cinfo, true);
    jpeg_start_decompress(&cinfo);

    JOCTET *profile_data=NULL;
    uint    profile_size;

    read_icc_profile (&cinfo, &profile_data, &profile_size);

    if (profile_data != NULL) 
    {
        QByteArray profile_rawdata = imageICCProfil();
        profile_rawdata.resize(profile_size);
        memcpy(profile_rawdata.data(), profile_data, profile_size);
        free (profile_data);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    fclose(file);    
 
    return true;
}

bool RAWLoader::getICCProfileFromTIFF(const QString& filePath)
{
    return false;
}

bool RAWLoader::save(const QString& /*filePath*/)
{
    return false;    
}

bool RAWLoader::hasAlpha() const
{
    return m_hasAlpha;
}

bool RAWLoader::sixteenBit() const
{
    return m_sixteenBit;
}

}  // NameSpace Digikam

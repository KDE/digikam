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

extern "C"
{
#include <stdio.h>
#include <math.h>
#include <unistd.h>
}

// QT includes.

#include <qfile.h>
#include <qcstring.h>
#include <qimage.h>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "dimg.h"
#include "rawloader.h"

// From dcraw program (parse.c) to identify camera model and constructor

extern "C"
{
    int dcraw_getCameraModel(const char* infile, char* cameraConstructor, char* cameraModel);
}

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
    // -h : Half-size color image (3x faster than -q)
    // -2 : 8bit ppm output
    // -w : Use camera white balance, if possible  
    // -a : Use automatic white balance
    command  = "dcraw -c -h -2 -w -a ";
    command += "'";
    command += QFile::encodeName( filePath );
    command += "'";
    kdWarning() << "Running dcraw command : " << command << endl;

    FILE* f = popen( command.data(), "r" );

    if ( !f )
    {
        kdWarning() << "dcraw program unvailable." << endl;
        return false;
    }

    if (fscanf (f, "P6 %d %d %d%c", &width, &height, &rgbmax, &nl) != 4) 
    {
        kdWarning() << "Not a raw digital camera image." << endl;
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

    char model[256], constructor[256];

    if ( dcraw_getCameraModel(QFile::encodeName(filePath), &constructor[0], &model[0]) == 0 )
    {
        imageSetCameraModel(QString::QString(model));
        imageSetCameraConstructor(QString::QString(constructor));
    }

    //----------------------------------------------------------

    m_sixteenBit  = false;
    imageWidth()  = width;
    imageHeight() = height;
    imageData()   = data;

    return true;
}

bool RAWLoader::load16bits(const QString& filePath)
{
    int	 width, height, rgbmax;
    char nl;

    QCString command;

    // run dcraw with options:
    // -c : write to stdout
    // -h : Half-size color image (3x faster than -q)
    // -4 : 16bit ppm output
    // -a : Use automatic white balance
    // -w : Use camera white balance, if possible
    command  = "dcraw -c -h -4 -w -a ";
    command += "'";
    command += QFile::encodeName( filePath );
    command += "'";
    kdWarning() << "Running dcraw command " << command << endl;

    FILE* f = popen( command.data(), "r" );

    if ( !f )
    {
        kdWarning() << "dcraw program unvailable." << endl;
        return false;
    }

    if (fscanf (f, "P6 %d %d %d%c", &width, &height, &rgbmax, &nl) != 4) 
    {
        kdWarning() << "Not a raw digital camera image." << endl;
        pclose (f);
        return false;
    }

    unsigned short *data = new unsigned short[width*height*4];
    unsigned short *dst  = data;
    uchar src[6];
    float fac = 65535.0 / rgbmax;

    kdWarning() << "rgbmax=" << rgbmax << "  fac=" << fac << endl;

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

    if ( dcraw_getCameraModel(QFile::encodeName(filePath), constructor, model) == 0 )
    {
        imageSetCameraModel(QString::QString(model));
        imageSetCameraConstructor(QString::QString(constructor));
    }

    //----------------------------------------------------------

    m_sixteenBit  = true;
    imageWidth()  = width;
    imageHeight() = height;
    imageData()   = (uchar *)data;

    return true;
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

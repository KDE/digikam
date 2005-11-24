/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2005-21-11
 * Description : A 16 bits/color/pixel PPM IO file for 
 *               DImg framework
 * 
 * Copyright 2005 by Renchi Raju, Gilles Caulier
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
#include <stdio.h>
#include <math.h>
#include <unistd.h>
}

// QT includes.

#include <qfile.h>
#include <qimage.h>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "ppmloader.h"

namespace Digikam
{

PPMLoader::PPMLoader(DImg* image)
         : DImgLoader(image)
{
}

bool PPMLoader::load(const QString& filePath)
{
    int	 width, height, rgbmax;
    char nl;
    
    FILE *file = fopen(QFile::encodeName(filePath), "rb");
    if (!file)
    {
        kdWarning() << k_funcinfo << "Cannot open image file." << endl;
        return false;
    }

    ushort header;

    if (fread(&header, 2, 1, file) != 1)
    {
        kdWarning() << k_funcinfo << "Cannot read header of file." << endl;
        fclose(file);
        return false;
    }

    uchar* c = (uchar*) &header;
    if (*c != 'P')
    {
        kdWarning() << k_funcinfo << "Not a PPM file." << endl;
        fclose(file);
        return false;
    }

    c++;
    if (*c != '6')
    {
        kdWarning() << k_funcinfo << "Not a PPM file." << endl;
        fclose(file);
        return false;
    }

    rewind(file);
    
    if (fscanf (file, "P6 %d %d %d%c", &width, &height, &rgbmax, &nl) != 4) 
    {
        kdWarning() << "Corrupted PPM file." << endl;
        pclose (file);
        return false;
    }
    
    if (rgbmax <= 255)
    {
        kdWarning() << k_funcinfo << "Not a 16 bits per color per pixel PPM file." << endl;
        pclose (file);
        return false;
    }
    
    unsigned short *data = new unsigned short[width*height*4];
    unsigned short *dst  = data;
    uchar src[6];
    float fac = 65535.0 / rgbmax;
    
    kdWarning() << "rgbmax=" << rgbmax << "  fac=" << fac << endl;

    for (int i = 0; i < width*height; i++)
    { 
        fread (src, 6 *sizeof(unsigned char), 1, file);

        // Swap byte order to preserve compatibility with PPC.
    
        if (QImage::systemByteOrder() == QImage::BigEndian)
            swab((const uchar *) src, (uchar *) src, 6*sizeof(uchar));
 
        dst[0] = (unsigned short)((src[4]*256 + src[5]) * fac);      // Blue
        dst[1] = (unsigned short)((src[2]*256 + src[3]) * fac);      // Green
        dst[2] = (unsigned short)((src[0]*256 + src[1]) * fac);      // Red
        dst[3] = 0xFFFF;

        dst += 4;
    }

    pclose( file );

    //----------------------------------------------------------

    imageWidth()  = width;
    imageHeight() = height;
    imageData()   = (uchar *)data;

    return true;
}

bool PPMLoader::save(const QString& /*filePath*/)
{
    return false;
}

}  // NameSpace Digikam

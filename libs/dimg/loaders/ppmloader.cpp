/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at gmail dot com> 
 * Date  : 2005-21-11
 * Description : A 16 bits/color/pixel PPM IO file for 
 *               DImg framework
 * 
 * Copyright 2005 by Renchi Raju, Gilles Caulier
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

// This line must be commented to prevent any latency time
// when we use threaded image loader interface for each image
// files io. Uncomment this line only for debugging.
//#define ENABLE_DEBUG_MESSAGES 

// C ansi includes.

extern "C" 
{
#include <unistd.h>
}

// C++ includes.

#include <cstdio>
#include <cmath>

// QT includes.

#include <qfile.h>
#include <qimage.h>

// Local includes.

#include "ddebug.h"
#include "dimg.h"
#include "dimgloaderobserver.h"
#include "ppmloader.h"

namespace Digikam
{

PPMLoader::PPMLoader(DImg* image)
         : DImgLoader(image)
{
}

bool PPMLoader::load(const QString& filePath, DImgLoaderObserver *observer)
{
    //TODO: progress information
    int	 width, height, rgbmax;
    char nl;
    
    FILE *file = fopen(QFile::encodeName(filePath), "rb");
    if (!file)
    {
        DDebug() << k_funcinfo << "Cannot open image file." << endl;
        return false;
    }

    ushort header;

    if (fread(&header, 2, 1, file) != 1)
    {
        DDebug() << k_funcinfo << "Cannot read header of file." << endl;
        fclose(file);
        return false;
    }

    uchar* c = (uchar*) &header;
    if (*c != 'P')
    {
        DDebug() << k_funcinfo << "Not a PPM file." << endl;
        fclose(file);
        return false;
    }

    c++;
    if (*c != '6')
    {
        DDebug() << k_funcinfo << "Not a PPM file." << endl;
        fclose(file);
        return false;
    }

    rewind(file);
    
    if (fscanf (file, "P6 %d %d %d%c", &width, &height, &rgbmax, &nl) != 4) 
    {
        DDebug() << "Corrupted PPM file." << endl;
        pclose (file);
        return false;
    }
    
    if (rgbmax <= 255)
    {
        DDebug() << k_funcinfo << "Not a 16 bits per color per pixel PPM file." << endl;
        pclose (file);
        return false;
    }
    
    if (observer)
        observer->progressInfo(m_image, 0.1);

    unsigned short *data;
    
    data = new unsigned short[width*height*4];
    unsigned short *dst  = data;
    uchar src[6];
    float fac = 65535.0 / rgbmax;
    int checkpoint = 0;

#ifdef ENABLE_DEBUG_MESSAGES
    DDebug() << "rgbmax=" << rgbmax << "  fac=" << fac << endl;
#endif

    for (int h = 0; h < height; h++)
    {

        if (observer && h == checkpoint)
        {
            checkpoint += granularity(observer, height, 0.9);
            if (!observer->continueQuery(m_image))
            {
                delete [] data;
                pclose( file );
                return false;
            }
            observer->progressInfo(m_image, 0.1 + (0.9 * ( ((float)h)/((float)height) )));
        }

        for (int w = 0; w < width; w++)
        {

            fread (src, 6 *sizeof(unsigned char), 1, file);

            dst[0] = (unsigned short)((src[4]*256 + src[5]) * fac);      // Blue
            dst[1] = (unsigned short)((src[2]*256 + src[3]) * fac);      // Green
            dst[2] = (unsigned short)((src[0]*256 + src[1]) * fac);      // Red
            dst[3] = 0xFFFF;

            dst += 4;
        }
    }

    fclose( file );

    //----------------------------------------------------------

    imageWidth()  = width;
    imageHeight() = height;
    imageData()   = (uchar*)data;
    imageSetAttribute("format", "PPM");
    
    return true;
}

bool PPMLoader::save(const QString& /*filePath*/, DImgLoaderObserver */*observer*/)
{
    return false;
}

}  // NameSpace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-21-11
 * Description : A 16 bits/color/pixel PPM IO file for
 *               DImg framework
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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


#include "ppmloader.h"

// C ANSI includes

extern "C"
{
#include <unistd.h>
}

// C++ includes

#include <cstdio>
#include <cmath>

// Qt includes

#include <QFile>
#include <QImage>

// Local includes

#include "dimg.h"
#include "dimgloaderobserver.h"
#include "debug.h"

namespace Digikam
{

PPMLoader::PPMLoader(DImg* image)
         : DImgLoader(image)
{
}

bool PPMLoader::load(const QString& filePath, DImgLoaderObserver *observer)
{
    //TODO: progress information
    int  width, height, rgbmax;
    char nl;

    FILE *file = fopen(QFile::encodeName(filePath), "rb");
    if (!file)
    {
        kDebug(digiKamAreaCode) << "Cannot open image file.";
        return false;
    }

    ushort header;

    if (fread(&header, 2, 1, file) != 1)
    {
        kDebug(digiKamAreaCode) << "Cannot read header of file.";
        fclose(file);
        return false;
    }

    uchar* c = (uchar*) &header;
    if (*c != 'P')
    {
        kDebug(digiKamAreaCode) << "Not a PPM file.";
        fclose(file);
        return false;
    }

    ++c;
    if (*c != '6')
    {
        kDebug(digiKamAreaCode) << "Not a PPM file.";
        fclose(file);
        return false;
    }

    rewind(file);

    if (fscanf (file, "P6 %d %d %d%c", &width, &height, &rgbmax, &nl) != 4)
    {
        kDebug(digiKamAreaCode) << "Corrupted PPM file.";
        pclose (file);
        return false;
    }

    if (rgbmax <= 255)
    {
        kDebug(digiKamAreaCode) << "Not a 16 bits per color per pixel PPM file.";
        pclose (file);
        return false;
    }

    if (observer)
        observer->progressInfo(m_image, 0.1F);

    unsigned short *data = 0;

    if (m_loadFlags & LoadImageData)
    {
        data = new_short_failureTolerant(width*height*4);
        if (!data)
        {
            kDebug(digiKamAreaCode) << "Failed to allocate memory for loading" << filePath;
            fclose(file);
            return false;
        }

        unsigned short *dst  = data;
        uchar src[6];
        float fac = 65535.0 / rgbmax;
        int checkpoint = 0;

    #ifdef ENABLE_DEBUG_MESSAGES
        kDebug(digiKamAreaCode) << "rgbmax=" << rgbmax << "  fac=" << fac;
    #endif

        for (int h = 0; h < height; ++h)
        {

            if (observer && h == checkpoint)
            {
                checkpoint += granularity(observer, height, 0.9F);
                if (!observer->continueQuery(m_image))
                {
                    delete [] data;
                    pclose( file );
                    return false;
                }
                observer->progressInfo(m_image, 0.1 + (0.9 * ( ((float)h)/((float)height) )));
            }

            for (int w = 0; w < width; ++w)
            {

                fread (src, 6 *sizeof(unsigned char), 1, file);

                dst[0] = (unsigned short)((src[4]*256 + src[5]) * fac);      // Blue
                dst[1] = (unsigned short)((src[2]*256 + src[3]) * fac);      // Green
                dst[2] = (unsigned short)((src[0]*256 + src[1]) * fac);      // Red
                dst[3] = 0xFFFF;

                dst += 4;
            }
        }
    }

    fclose( file );

    //----------------------------------------------------------

    imageWidth()  = width;
    imageHeight() = height;
    imageData()   = (uchar*)data;
    imageSetAttribute("format", "PPM");
    imageSetAttribute("originalColorFormat", DImg::RGB);
    imageSetAttribute("originalBitDepth", 8);

    return true;
}

bool PPMLoader::save(const QString& /*filePath*/, DImgLoaderObserver* /*observer*/)
{
    return false;
}

}  // namespace Digikam

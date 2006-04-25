/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date  : 2005-11-01
 * Description : A digital camera RAW files loader for DImg 
 *               framework using dcraw program.
 *
 * Copyright 2005-2006 by Gilles Caulier
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
#define ENABLE_DEBUG_MESSAGES

extern "C"
{
#include <unistd.h>
}

// C++ includes.

#include <cstdio>
#include <cstdlib>
#include <cmath>

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
#include "dcrawbinary.h"

namespace Digikam
{

RAWLoader::RAWLoader(DImg* image, RawDecodingSettings rawDecodingSettings)
         : DImgLoader(image)
{
    m_hasAlpha            = false;
    m_rawDecodingSettings = rawDecodingSettings;
}

bool RAWLoader::load(const QString& filePath, DImgLoaderObserver *observer)
{
    readMetadata(filePath, DImg::RAW);
    return ( load16bits(filePath, observer) );
}

bool RAWLoader::load8bits(const QString& filePath, DImgLoaderObserver *observer)
{
    int  width, height, rgbmax;
    char nl;

    QCString command;

    // run dcraw with options:
    // -c : write to stdout
    // -2 : 8bit ppm output
    // -f : Interpolate RGB as four colors. This blurs the image a little, but it eliminates false 2x2 mesh patterns.
    // -w : Use camera white balance, if possible  
    // -a : Use automatic white balance
    // -n : Don't clip colors
    // -s : Use secondary pixels (Fuji Super CCD SR only)
    // -q : Use simple bilinear interpolation for quick results
    // -B : Use bilateral filter to smooth noise while preserving edges.
    // -p : Use the input ICC profiles to define the camera's raw colorspace.
    // -o : Use ICC profiles to define the output colorspace.

    command = DcrawBinary::instance()->path();
    command += " -c -2 ";
    
    if (m_rawDecodingSettings.cameraColorBalance)
        command += "-w ";
    
    if (m_rawDecodingSettings.automaticColorBalance)
        command += "-a ";

    if (m_rawDecodingSettings.RGBInterpolate4Colors)
        command += "-f ";

    if (m_rawDecodingSettings.SuperCCDsecondarySensor)
        command += "-s ";

    if (m_rawDecodingSettings.unclipColors)
        command += "-n -b 0.25 ";    

    command += "-q ";

    if (m_rawDecodingSettings.enableRAWQuality)
    {
        QCString rawQuality;
        command += rawQuality.setNum(m_rawDecodingSettings.RAWQuality);
        command += " ";
    }

    if (m_rawDecodingSettings.enableNoiseReduction)
    {
        QCString NRSigmaDomain, NRSigmaRange;
        command += "-B ";
        command += NRSigmaDomain.setNum(m_rawDecodingSettings.NRSigmaDomain);
        command += " ";
        command += NRSigmaRange.setNum(m_rawDecodingSettings.NRSigmaRange);
        command += " ";
    }

    switch (m_rawDecodingSettings.ICCColorCorrectionMode)
    {
        case RawDecodingSettings::EMBED:
            command += "-p embed ";
            break;
            
        case RawDecodingSettings::USERPROFILE:
            command += "-p ";
            command += QFile::encodeName( KProcess::quote( m_rawDecodingSettings.cameraICCProfilePath ) );
            command += " ";
            break;
            
        default:
            break;
    }
    
    if (m_rawDecodingSettings.ICCColorCorrectionMode != RawDecodingSettings::NOICC &&
        !m_rawDecodingSettings.outputICCProfilePath.isEmpty())
    {
        command += "-o ";
        command += QFile::encodeName( KProcess::quote( m_rawDecodingSettings.outputICCProfilePath ) );
        command += " ";
    }
    
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

    // -------------------------------------------------------------------
    // Get image data
    
    uchar *data = 0;

    data = new uchar[width*height*4];
    uchar *dst = data;
    uchar src[3];
    int   checkpoint = 0;

    for (int h = 0; h < height; h++)
    {

        if (observer && h == checkpoint)
        {
            checkpoint += granularity(observer, height);
            if (!observer->continueQuery(m_image))
            {
                delete [] data;
                //TODO: real shutdown! pclose() waits!
                pclose( f );
                return false;
            }
            observer->progressInfo(m_image, ((float)h)/((float)height) );
        }

        for (int w = 0; w < width; w++)
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
    }
    
    pclose( f );

    //----------------------------------------------------------
    // Get Camera and Constructor model
    
    char       model[256], constructor[256];
    DcrawParse rawFileParser;
    
    if ( rawFileParser.getCameraModel(QFile::encodeName(filePath), &constructor[0], &model[0]) == 0 )
    {
        imageSetCameraModel(QString(model));
        imageSetCameraConstructor(QString(constructor));
    }

    //----------------------------------------------------------

    m_sixteenBit  = false;
    imageWidth()  = width;
    imageHeight() = height;
    imageSetAttribute("format", "RAW");
    imageData()   = data;
    
    readMetadata(filePath, DImg::RAW);
    
    return true;
}

bool RAWLoader::load16bits(const QString& filePath, DImgLoaderObserver *observer)
{
    int	 width, height, rgbmax;
    char nl;

    QCString command;

    // run dcraw with options:
    // -c : write to stdout
    // -4 : 16bit ppm output
    // -f : Interpolate RGB as four colors. This blurs the image a little, but it eliminates false 2x2 mesh patterns.
    // -a : Use automatic white balance
    // -w : Use camera white balance, if possible
    // -n : Don't clip colors
    // -s : Use secondary pixels (Fuji Super CCD SR only)
    // -q : Use simple bilinear interpolation for quick results
    // -B : Use bilateral filter to smooth noise while preserving edges.
    // -p : Use the input ICC profiles to define the camera's raw colorspace.
    // -o : Use ICC profiles to define the output colorspace.

    command = DcrawBinary::instance()->path();
    command += " -c -4 ";
    
    if (m_rawDecodingSettings.cameraColorBalance)
        command += "-w ";
    
    if (m_rawDecodingSettings.automaticColorBalance)
        command += "-a ";

    if (m_rawDecodingSettings.RGBInterpolate4Colors)
        command += "-f ";

    if (m_rawDecodingSettings.SuperCCDsecondarySensor)
        command += "-s ";
    
    if (m_rawDecodingSettings.unclipColors)
        command += "-n -b 0.25 ";    

    command += "-q ";

    if (m_rawDecodingSettings.enableRAWQuality)
    {
        QCString rawQuality;
        command += rawQuality.setNum(m_rawDecodingSettings.RAWQuality);
        command += " ";
    }
    
    if (m_rawDecodingSettings.enableNoiseReduction)
    {
        QCString NRSigmaDomain, NRSigmaRange;
        command += "-B ";
        command += NRSigmaDomain.setNum(m_rawDecodingSettings.NRSigmaDomain);
        command += " ";
        command += NRSigmaRange.setNum(m_rawDecodingSettings.NRSigmaRange);
        command += " ";
    }

    switch (m_rawDecodingSettings.ICCColorCorrectionMode)
    {
        case RawDecodingSettings::EMBED:
            command += "-p embed ";
            break;
            
        case RawDecodingSettings::USERPROFILE:
            command += "-p ";
            command += QFile::encodeName( KProcess::quote( m_rawDecodingSettings.cameraICCProfilePath ) );
            command += " ";
            break;
            
        default:
            break;
    }

    if (m_rawDecodingSettings.ICCColorCorrectionMode != RawDecodingSettings::NOICC &&
        !m_rawDecodingSettings.outputICCProfilePath.isEmpty())
    {
        command += "-o ";
        command += QFile::encodeName( KProcess::quote( m_rawDecodingSettings.outputICCProfilePath ) );
        command += " ";
    }
        
    command += QFile::encodeName( KProcess::quote( filePath ) );

#ifdef ENABLE_DEBUG_MESSAGES
    kdDebug() << "Running dcraw command " << command << endl;
#endif

    // post one progress info before starting the program.
    // There may be a delay before the first data is read.
    if (observer)
        observer->progressInfo(m_image, 0.1);

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

    // -------------------------------------------------------------------
    // Get image data
    
    unsigned short  *data = 0;

    data = new unsigned short[width*height*4];
    unsigned short *dst  = data;
    uchar src[6];
    float fac = 65535.0 / rgbmax;
    int   checkpoint = 0;

    for (int h = 0; h < height; h++)
    {

        if (observer && h == checkpoint)
        {
            checkpoint += granularity(observer, height, 0.9);
            if (!observer->continueQuery(m_image))
            {
                delete [] data;
                //TODO: real shutdown! pclose() waits!
                pclose( f );
                return false;
            }
            observer->progressInfo(m_image, 0.1 + 0.9*(((float)h)/((float)height)) );
        }

        for (int w = 0; w < width; w++)
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
    }

    pclose( f );

    //----------------------------------------------------------
    // Get Camera and Constructor model

    char model[256], constructor[256];
    DcrawParse rawFileParser;
    
    if ( rawFileParser.getCameraModel(QFile::encodeName(filePath), constructor, model) == 0 )
    {
        imageSetCameraModel(QString(model));
        imageSetCameraConstructor(QString(constructor));
    }

    //----------------------------------------------------------

    m_sixteenBit  = true;
    imageWidth()  = width;
    imageHeight() = height;
    imageSetAttribute("format", "RAW");
    imageData() = (uchar *)data;

    readMetadata(filePath, DImg::RAW);
                
    return true;
}

bool RAWLoader::save(const QString&, DImgLoaderObserver *)
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

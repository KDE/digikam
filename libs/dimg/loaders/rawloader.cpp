/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com> 
 *          Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date   : 2005-11-01
 * Description : A digital camera RAW files loader for DImg 
 *               framework using an external dcraw instance.
 *
 * Copyright 2005-2007 by Gilles Caulier and Marcel Wiesweg
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

// QT includes.

#include <qcstring.h>

// Local includes.

#include "ddebug.h"
#include "dimg.h"
#include "dimgloaderobserver.h"
#include "rawloader.h"
#include "rawloader.moc"

namespace Digikam
{

RAWLoader::RAWLoader(DImg* image, KDcrawIface::RawDecodingSettings rawDecodingSettings)
         : DImgLoader(image)
{
    m_rawDecodingSettings = rawDecodingSettings;
    m_observer            = 0;
}

bool RAWLoader::load(const QString& filePath, DImgLoaderObserver *observer)
{
    m_observer = observer;

    // We are using KProcess here, and make two assumptions:
    // - there is an event loop (not for ioslaves)
    // - we are not called from the event loop thread
    // These assumptions are currently true for all use cases in digikam,
    // except the thumbnails iosalve, which will set this attribute.
    // I hope when porting to Qt4, all the event loop stuff (and this problem) can be removed.
    if (imageGetAttribute("noeventloop").isValid())
        return false;
    
    readMetadata(filePath, DImg::RAW);
  
    // NOTE: Here, we don't check a possible embeded work-space color profile using 
    // the method checkExifWorkingColorSpace() like with JPEG, PNG, and TIFF loaders, 
    // because RAW file are always in linear mode.
    
    int width, height, rgbmax;
    QByteArray data;
    if (!KDcrawIface::KDcraw::decodeRAWImage(filePath, m_rawDecodingSettings, 
                                             data, width, height, rgbmax))
        return false;

    return (loadedFromDcraw(data, width, height, rgbmax, observer));
}

bool RAWLoader::checkToCancelWaitingData()
{
    return (m_observer ? !m_observer->continueQuery(m_image) : false);
}

bool RAWLoader::checkToCancelRecievingData()
{
    return (m_observer ? m_observer->isShuttingDown() : false);
}

void RAWLoader::setWaitingDataProgress(double value)
{
    if (m_observer)
        m_observer->progressInfo(m_image, value);
}

void RAWLoader::setRecievingDataProgress(double value)
{
    if (m_observer)
        m_observer->progressInfo(m_image, value);
}

bool RAWLoader::loadedFromDcraw(QByteArray data, int width, int height, int rgbmax,
                                DImgLoaderObserver *observer)
{
    int checkpoint = 0;

    if (m_rawDecodingSettings.sixteenBitsImage)       // 16 bits image
    {
        uchar *image = new uchar[width*height*8];

        unsigned short *dst = (unsigned short *)image;
        uchar          *src = (uchar*)data.data();
        float fac           = 65535.0 / rgbmax;
        checkpoint          = 0;

        for (int h = 0; h < height; h++)
        {
            if (observer && h == checkpoint)
            {
                checkpoint += granularity(observer, height, 1.0);
                if (!observer->continueQuery(m_image))
                {
                    return false;
                }
                observer->progressInfo(m_image, 0.7 + 0.3*(((float)h)/((float)height)) );
            }

            for (int w = 0; w < width; w++)
            {
                dst[0] = (unsigned short)((src[4]*256 + src[5]) * fac);      // Blue
                dst[1] = (unsigned short)((src[2]*256 + src[3]) * fac);      // Green
                dst[2] = (unsigned short)((src[0]*256 + src[1]) * fac);      // Red
                dst[3] = 0xFFFF;

                dst += 4;
                src += 6;
            }
        }

        imageData() = (uchar *)image;
    }
    else        // 8 bits image
    {
        uchar *image = new uchar[width*height*4];
        uchar *dst   = image;
        uchar *src   = (uchar*)data.data();
        checkpoint   = 0;

        for (int h = 0; h < height; h++)
        {

            if (observer && h == checkpoint)
            {
                checkpoint += granularity(observer, height, 1.0);
                if (!observer->continueQuery(m_image))
                {
                    return false;
                }
                observer->progressInfo(m_image, 0.7 + 0.3*(((float)h)/((float)height)) );
            }

            for (int w = 0; w < width; w++)
            {
                // No need to adapt RGB components accordinly with rgbmax value because dcraw
                // always return rgbmax to 255 in 8 bits/color/pixels.

                dst[0] = src[2];    // Blue
                dst[1] = src[1];    // Green
                dst[2] = src[0];    // Red
                dst[3] = 0xFF;      // Alpha

                dst += 4;
                src += 3;
            }
        }

        imageData() = image;
    }

    //----------------------------------------------------------

    imageWidth()  = width;
    imageHeight() = height;
    imageSetAttribute("format", "RAW");

    return true;
}

}  // NameSpace Digikam



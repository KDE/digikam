/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-11-01
 * Description : A digital camera RAW files loader for DImg
 *               framework using an external dcraw instance.
 *
 * Copyright (C) 2005-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "rawloader.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QByteArray>

// KDE includes

#include <kstandarddirs.h>
#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "dimgloaderobserver.h"
#include "icctransform.h"
#include "imagehistogram.h"
#include "imagelevels.h"
#include "curvesfilter.h"
#include "bcgfilter.h"
#include "wbfilter.h"
#include "globals.h"
#include "dimagehistory.h"

namespace Digikam
{

RAWLoader::RAWLoader(DImg* const image, const DRawDecoding& rawDecodingSettings)
    : DImgLoader(image),
      m_observer(0),
      m_filter(0)
{
    m_rawDecodingSettings = rawDecodingSettings.rawPrm;
    m_filter              = new RawProcessingFilter(this);
    m_filter->setSettings(rawDecodingSettings);
}

bool RAWLoader::load(const QString& filePath, DImgLoaderObserver* const observer)
{
    m_observer = observer;

    readMetadata(filePath, DImg::RAW);

    DcrawInfoContainer dcrawIdentify;

    if (!KDcraw::rawFileIdentify(dcrawIdentify, filePath))
    {
        return false;
    }

    if (m_loadFlags & LoadImageData)
    {
        int        width, height, rgbmax;
        QByteArray data;

        // NOTE: Here, we don't check a possible embedded work-space color profile using
        // the method checkExifWorkingColorSpace() like with JPEG, PNG, and TIFF loaders,
        // because RAW file are always in linear mode.

        if (m_rawDecodingSettings.outputColorSpace == RawDecodingSettings::CUSTOMOUTPUTCS)
        {
            if (m_rawDecodingSettings.outputProfile == IccProfile::sRGB().filePath())
            {
                m_rawDecodingSettings.outputColorSpace = RawDecodingSettings::SRGB;
            }
            else if (m_rawDecodingSettings.outputProfile == IccProfile::adobeRGB().filePath())
            {
                m_rawDecodingSettings.outputColorSpace = RawDecodingSettings::ADOBERGB;
            }
            else if (m_rawDecodingSettings.outputProfile == IccProfile::wideGamutRGB().filePath())
            {
                m_rawDecodingSettings.outputColorSpace = RawDecodingSettings::WIDEGAMMUT;
            }
            else if (m_rawDecodingSettings.outputProfile == IccProfile::proPhotoRGB().filePath())
            {
                m_rawDecodingSettings.outputColorSpace = RawDecodingSettings::PROPHOTO;
            }
            else
            {
                // Specifying a custom output is broken somewhere. We use the extremely
                // wide gamut pro photo profile for 16bit (sRGB for 8bit) and convert afterwards.
                m_filter->setOutputProfile(m_rawDecodingSettings.outputProfile);

                if (m_rawDecodingSettings.sixteenBitsImage)
                {
                    m_rawDecodingSettings.outputColorSpace = RawDecodingSettings::PROPHOTO;
                }
                else
                {
                    m_rawDecodingSettings.outputColorSpace = RawDecodingSettings::SRGB;
                }
            }
        }

        if (!KDcraw::decodeRAWImage(filePath, m_rawDecodingSettings,
                                                 data, width, height, rgbmax))
        {
            loadingFailed();
            return false;
        }

        if (!loadedFromRawData(data, width, height, rgbmax, observer))
        {
            loadingFailed();
            return false;
        }
    }
    else
    {
        imageWidth()  = dcrawIdentify.imageSize.width();
        imageHeight() = dcrawIdentify.imageSize.height();
    }

    imageSetAttribute("format",             "RAW");
    imageSetAttribute("originalColorModel", DImg::COLORMODELRAW);
    imageSetAttribute("originalBitDepth",   16);
    imageSetAttribute("originalSize",       dcrawIdentify.imageSize);

    return true;
}

bool RAWLoader::checkToCancelWaitingData()
{
    return (m_observer ? !m_observer->continueQuery(m_image) : false);
}

void RAWLoader::setWaitingDataProgress(double value)
{
    if (m_observer)
    {
        m_observer->progressInfo(m_image, value);
    }
}

bool RAWLoader::loadedFromRawData(const QByteArray& data, int width, int height, int rgbmax,
                                  DImgLoaderObserver* const observer)
{
    int checkpoint = 0;

    if (m_rawDecodingSettings.sixteenBitsImage)       // 16 bits image
    {
        uchar* image = new_failureTolerant(width, height, 8);

        if (!image)
        {
            kDebug() << "Failed to allocate memory for loading raw file";
            return false;
        }

        unsigned short* dst = reinterpret_cast<unsigned short*>(image);
        uchar*          src = (uchar*)data.data();
        float fac           = 65535.0 / rgbmax;
        checkpoint          = 0;

        for (int h = 0; h < height; ++h)
        {
            if (observer && h == checkpoint)
            {
                checkpoint += granularity(observer, height, 1.0);

                if (!observer->continueQuery(m_image))
                {
                    delete [] image;
                    return false;
                }

                observer->progressInfo(m_image, 0.7 + 0.2 * (((float)h) / ((float)height)));
            }

            for (int w = 0; w < width; ++w)
            {
                if (QSysInfo::ByteOrder == QSysInfo::LittleEndian)     // Intel
                {
                    dst[0] = (unsigned short)((src[5] * 256 + src[4]) * fac);    // Blue
                    dst[1] = (unsigned short)((src[3] * 256 + src[2]) * fac);    // Green
                    dst[2] = (unsigned short)((src[1] * 256 + src[0]) * fac);    // Red
                }
                else
                {
                    dst[0] = (unsigned short)((src[4] * 256 + src[5]) * fac);    // Blue
                    dst[1] = (unsigned short)((src[2] * 256 + src[3]) * fac);    // Green
                    dst[2] = (unsigned short)((src[0] * 256 + src[1]) * fac);    // Red
                }

                dst[3]  = 0xFFFF;

                dst    += 4;
                src    += 6;
            }
        }

        // ----------------------------------------------------------

        imageData() = (uchar*)image;
    }
    else        // 8 bits image
    {
        uchar* image = new_failureTolerant(width, height, 4);

        if (!image)
        {
            kDebug() << "Failed to allocate memory for loading raw file";
            return false;
        }

        uchar* dst = image;
        uchar* src = (uchar*)data.data();
        checkpoint = 0;

        for (int h = 0; h < height; ++h)
        {

            if (observer && h == checkpoint)
            {
                checkpoint += granularity(observer, height, 1.0);

                if (!observer->continueQuery(m_image))
                {
                    delete [] image;
                    return false;
                }

                observer->progressInfo(m_image, 0.7 + 0.2 * (((float)h) / ((float)height)));
            }

            for (int w = 0; w < width; ++w)
            {
                // No need to adapt RGB components accordingly with rgbmax value because dcraw
                // always return rgbmax to 255 in 8 bits/color/pixels.

                dst[0]  = src[2];    // Blue
                dst[1]  = src[1];    // Green
                dst[2]  = src[0];    // Red
                dst[3]  = 0xFF;      // Alpha

                dst    += 4;
                src    += 3;
            }
        }

        // NOTE: if Color Management is not used here, output color space is in sRGB* color space.
        // Gamma and White balance are previously adjusted by dcraw in 8 bits color depth.

        imageData() = image;
    }

    //----------------------------------------------------------
    // Assign the right color-space profile.

    switch (m_rawDecodingSettings.outputColorSpace)
    {
        case RawDecodingSettings::SRGB:
        {
            imageSetIccProfile(IccProfile::sRGB());
            break;
        }

        case RawDecodingSettings::ADOBERGB:
        {
            imageSetIccProfile(IccProfile::adobeRGB());
            break;
        }

        case RawDecodingSettings::WIDEGAMMUT:
        {
            imageSetIccProfile(IccProfile::wideGamutRGB());
            break;
        }

        case RawDecodingSettings::PROPHOTO:
        {
            imageSetIccProfile(IccProfile::proPhotoRGB());
            break;
        }

        case RawDecodingSettings::CUSTOMOUTPUTCS:
        {
            imageSetIccProfile(m_rawDecodingSettings.outputProfile);
            break;
        }

        case RawDecodingSettings::RAWCOLOR:
        {
            // No icc color-space profile to assign in RAW color mode.
            imageSetAttribute("uncalibratedColor", true);
            break;
        }
    }

    //----------------------------------------------------------

    FilterAction action = m_filter->filterAction();
    m_image->addFilterAction(action);

    imageWidth()        = width;
    imageHeight()       = height;
    imageSetAttribute("rawDecodingSettings",     QVariant::fromValue(m_filter->settings()));
    imageSetAttribute("rawDecodingFilterAction", QVariant::fromValue(action));
    // other attributes are set above

    return true;
}

void RAWLoader::postProcess(DImgLoaderObserver* const observer)
{
    if (m_filter->settings().postProcessingSettingsIsDirty())
    {
        m_filter->setObserver(observer, 90, 100);
        m_filter->setupFilter(*m_image);
        m_filter->startFilterDirectly();
    }
}

FilterAction RAWLoader::filterAction() const
{
    return m_filter->filterAction();
}

bool RAWLoader::save(const QString& /*filePath*/, DImgLoaderObserver* const /*observer=0*/)
{
    // NOTE: RAW files are always Read only.
    return false;
}

bool RAWLoader::hasAlpha() const
{
    return false;
}

bool RAWLoader::isReadOnly() const
{
    return true;
}

bool RAWLoader::sixteenBit() const
{
    return m_rawDecodingSettings.sixteenBitsImage;
}

}  // namespace Digikam

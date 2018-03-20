/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-21-11
 * Description : A 16 bits/color/pixel PPM IO file for
 *               DImg framework
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "digikam_config.h"
#include "dimg.h"
#include "digikam_debug.h"
#include "dimgloaderobserver.h"

namespace Digikam
{

PPMLoader::PPMLoader(DImg* const image)
    : DImgLoader(image)
{
}

bool PPMLoader::load(const QString& filePath, DImgLoaderObserver* const observer)
{
    //TODO: progress information
    int  width, height, rgbmax;
    char nl;

    qCDebug(DIGIKAM_DIMG_LOG_PPM) << "Opening file" << filePath;

    FILE* const file = fopen(QFile::encodeName(filePath).constData(), "rb");

    if (!file)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PPM) << "Cannot open image file.";
        loadingFailed();
        return false;
    }

    ushort header;

    if (fread(&header, 2, 1, file) != 1)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PPM) << "Cannot read header of file.";
        fclose(file);
        loadingFailed();
        return false;
    }

    uchar* c = (uchar*) &header;

    if (*c != 'P')
    {
        qCWarning(DIGIKAM_DIMG_LOG_PPM) << "Not a PPM file.";
        fclose(file);
        loadingFailed();
        return false;
    }

    ++c;

    if (*c != '6')
    {
        qCWarning(DIGIKAM_DIMG_LOG_PPM) << "Not a PPM file.";
        fclose(file);
        loadingFailed();
        return false;
    }

    rewind(file);

    // FIXME: scanf without field width limits can crash with huge input data
    if (fscanf(file, "P6 %d %d %d%c", &width, &height, &rgbmax, &nl) != 4)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PPM) << "Corrupted PPM file.";
        fclose(file);
        loadingFailed();
        return false;
    }

    if (rgbmax <= 255)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PPM) << "Not a 16 bits per color per pixel PPM file.";
        fclose(file);
        loadingFailed();
        return false;
    }

    if (observer)
    {
        observer->progressInfo(m_image, 0.1F);
    }

    QScopedArrayPointer<unsigned short> data;

    if (m_loadFlags & LoadImageData)
    {
        data.reset(new_short_failureTolerant((size_t)(width * height * 4)));

        if (data.isNull())
        {
            qCWarning(DIGIKAM_DIMG_LOG_PPM) << "Failed to allocate memory for loading" << filePath;
            fclose(file);
            loadingFailed();
            return false;
        }

        unsigned short* dst  = data.data();
        uchar src[6];
        float fac = 65535.0 / rgbmax;
        int checkpoint = 0;

        qCDebug(DIGIKAM_DIMG_LOG_PPM) << "rgbmax=" << rgbmax << "  fac=" << fac;

        for (int h = 0; h < height; ++h)
        {

            if (observer && h == checkpoint)
            {
                checkpoint += granularity(observer, height, 0.9F);

                if (!observer->continueQuery(m_image))
                {
                    fclose(file);
                    loadingFailed();
                    return false;
                }

                observer->progressInfo(m_image, 0.1 + (0.9 * (((float)h) / ((float)height))));
            }

            for (int w = 0; w < width; ++w)
            {

                if (fread(src, 6 * sizeof(unsigned char), 1, file) != 1)
                {
                    qCWarning(DIGIKAM_DIMG_LOG_PPM) << "Premature end of PPM file";
                    fclose(file);
                    loadingFailed();
                    return false;
                }

                dst[0] = (unsigned short)((src[4] * 256 + src[5]) * fac);    // Blue
                dst[1] = (unsigned short)((src[2] * 256 + src[3]) * fac);    // Green
                dst[2] = (unsigned short)((src[0] * 256 + src[1]) * fac);    // Red
                dst[3] = 0xFFFF;

                dst += 4;
            }
        }
    }

    fclose(file);

    //----------------------------------------------------------

    imageWidth()  = width;
    imageHeight() = height;
    imageData()   = (uchar*)data.take();
    imageSetAttribute(QLatin1String("format"),              QLatin1String("PPM"));
    imageSetAttribute(QLatin1String("originalColorFormat"), DImg::RGB);
    imageSetAttribute(QLatin1String("originalBitDepth"),    8);
    imageSetAttribute(QLatin1String("originalSize"),        QSize(width, height));

    return true;
}

bool PPMLoader::save(const QString& /*filePath*/, DImgLoaderObserver* const /*observer*/)
{
    return false;
}

bool PPMLoader::hasAlpha()   const
{
    return false;
}

bool PPMLoader::sixteenBit() const
{
    return true;
}

bool PPMLoader::isReadOnly() const
{
    return true;
}

}  // namespace Digikam

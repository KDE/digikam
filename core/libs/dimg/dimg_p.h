/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-15
 * Description : DImg private data members
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_DIMG_PRIVATE_H
#define DIGIKAM_DIMG_PRIVATE_H

#include "digikam_config.h"

// C ANSI includes

#ifndef Q_OS_WIN
extern "C"
{
#endif

#include <stdint.h>

#ifndef Q_OS_WIN
#include <math.h>
}
#endif

// C++ includes

#include <cstdio>

// Qt includes

#include <QString>
#include <QByteArray>
#include <QVariant>
#include <QMap>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QPaintEngine>
#include <QPainter>
#include <QPixmap>
#include <QSysInfo>
#include <QUuid>

// Local includes

#include "dimg.h"
#include "digikam_export.h"
#include "digikam_debug.h"
#include "dmetadata.h"
#include "dshareddata.h"
#include "dimagehistory.h"
#include "iccprofile.h"
#include "metaengine_rotation.h"
#include "drawdecoder.h"
#include "dimagehistory.h"
#include "pngloader.h"
#include "tiffloader.h"
#include "rawloader.h"
#include "pgfloader.h"
#include "qimageloader.h"
#include "jpegloader.h"
#include "filereadwritelock.h"
#include "iccmanager.h"
#include "icctransform.h"
#include "exposurecontainer.h"
#include "dmetadata.h"
#include "dimgloaderobserver.h"
#include "randomnumbergenerator.h"

#ifdef HAVE_JASPER
#   include "jp2kloader.h"
#endif // HAVE_JASPER

#ifdef HAVE_IMAGE_MAGICK
#   include "magickloader.h"
#endif // HAVE_IMAGE_MAGICK

/** Lanczos kernel is precomputed in a table with this resolution
    The value below seems to be enough for HQ upscaling up to eight times
 */
#define LANCZOS_TABLE_RES  256

/** A support of 3 gives an overall sharper looking image, but
    it is a) slower b) gives more sharpening artifacts
 */
#define LANCZOS_SUPPORT    2

/** Define this to use a floating-point implementation of Lanczos interpolation.
    The integer implementation is a little bit less accurate, but MUCH faster
    (even on machines with FPU - ~2.5 times faster on Core2); besides, it will
    run a hell lot faster on computers without a FPU (e.g. PDAs).
 */
//#define LANCZOS_DATA_FLOAT
#ifdef LANCZOS_DATA_FLOAT
#   define LANCZOS_DATA_TYPE float
#   define LANCZOS_DATA_ONE 1.0
#else
#   define LANCZOS_DATA_TYPE int
#   define LANCZOS_DATA_ONE 4096
#endif

typedef uint64_t ullong;    // krazy:exclude=typedefs
typedef int64_t  llong;     // krazy:exclude=typedefs

namespace Digikam
{

class DIGIKAM_EXPORT DImg::Private : public DSharedData
{
public:

    explicit Private()
    {
        null         = true;
        width        = 0;
        height       = 0;
        data         = 0;
        lanczos_func = 0;
        alpha        = false;
        sixteenBit   = false;
    }

    ~Private()
    {
        delete [] data;
        delete [] lanczos_func;
    }

    static QStringList fileOriginAttributes()
    {
        QStringList list;
        list << QLatin1String("format")
             << QLatin1String("isReadOnly")
             << QLatin1String("originalFilePath")
             << QLatin1String("originalSize")
             << QLatin1String("originalImageHistory")
             << QLatin1String("rawDecodingSettings")
             << QLatin1String("rawDecodingFilterAction")
             << QLatin1String("uniqueHash")
             << QLatin1String("uniqueHashV2");

        return list;
    }

    /**
     * x,y, w x h is a section of the image. The image size is width x height.
     * Clips the section to the bounds of the image.
     * Returns if the (clipped) section is a valid rectangle.
     */
    static bool clipped(int& x, int& y, int& w, int& h, uint width, uint height)
    {
        QRect inner(x, y, w, h);
        QRect outer(0, 0, width, height);

        if (!outer.contains(inner))
        {
            QRect clipped = inner.intersected(outer);
            x             = clipped.x();
            y             = clipped.y();
            w             = clipped.width();
            h             = clipped.height();
            return clipped.isValid();
        }

        return inner.isValid();
    }

public:

    bool                    null;
    bool                    alpha;
    bool                    sixteenBit;

    unsigned int            width;
    unsigned int            height;

    unsigned char*          data;
    LANCZOS_DATA_TYPE*      lanczos_func;

    MetaEngineData          metaData;
    QMap<QString, QVariant> attributes;
    QMap<QString, QString>  embeddedText;
    IccProfile              iccProfile;
    DImageHistory           imageHistory;
};

} // namespace Digikam

#endif // DIGIKAM_DIMG_PRIVATE_H

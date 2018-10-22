/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-15
 * Description : DImg private data members
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

#ifndef DIGIKAM_DIMG_PRIVATE_H
#define DIGIKAM_DIMG_PRIVATE_H

// Qt includes

#include <QMutexLocker>
#include <QByteArray>
#include <QVariant>
#include <QString>
#include <QMutex>
#include <QMap>

// Local includes

#include "digikam_export.h"
#include "dmetadata.h"
#include "dshareddata.h"
#include "dimagehistory.h"
#include "iccprofile.h"

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
#define LANCZOS_DATA_TYPE float
#define LANCZOS_DATA_ONE 1.0
#else
#define LANCZOS_DATA_TYPE int
#define LANCZOS_DATA_ONE 4096
#endif

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
             << QLatin1String("isreadonly")
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

    void attributesInsert(const QString& key, const QVariant& value)
    {
        QMutexLocker lock(&mutex);
        attributes.insert(key, value);
    }

    QVariant attributesValue(const QString& key) const
    {
        QMutexLocker lock(const_cast<QMutex*>(&mutex));
        return attributes.value(key);
    }

    bool attributesContains(const QString& key) const
    {
        QMutexLocker lock(const_cast<QMutex*>(&mutex));
        return attributes.contains(key);
    }

    void attributesRemove(const QString& key)
    {
        QMutexLocker lock(&mutex);
        attributes.remove(key);
    }

    void embeddedTextInsert(const QString& key, const QString& value)
    {
        QMutexLocker lock(&mutex);
        embeddedText.insert(key, value);
    }

    QString embeddedTextValue(const QString& key) const
    {
        QMutexLocker lock(const_cast<QMutex*>(&mutex));
        return embeddedText.value(key);
    }

    bool embeddedTextContains(const QString& key) const
    {
        QMutexLocker lock(const_cast<QMutex*>(&mutex));
        return embeddedText.contains(key);
    }

    void clearMaps()
    {
        QMutexLocker lock(&mutex);
        attributes.clear();
        embeddedText.clear();
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

    QMutex                  mutex;
};

} // namespace Digikam

#endif // DIGIKAM_DIMG_PRIVATE_H

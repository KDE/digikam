/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2009-10-11
 * Description : save image thread
 *
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "saveimgthread.h"

// Qt includes

#include <QImage>
#include <QDateTime>

// LibKSane includes

#include <ksane.h>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "dmetadata.h"

using namespace KSaneIface;

namespace Digikam
{

class SaveImgThread::Private
{
public:

    Private()
    {
        width        = 0;
        height       = 0;
        bytesPerLine = 0;
        frmt         = 0;
    }

    int        width;
    int        height;
    int        bytesPerLine;
    int        frmt;

    QByteArray ksaneData;

    QString    make;
    QString    model;
    QString    format;

    QUrl       newUrl;
};

SaveImgThread::SaveImgThread(QObject* const parent)
    : QThread(parent),
      d(new Private)
{
}

SaveImgThread::~SaveImgThread()
{
    // wait for the thread to finish
    wait();

    delete d;
}

void SaveImgThread::setImageData(const QByteArray& ksaneData, int width, int height,
                                 int bytesPerLine, int ksaneFormat)
{
    d->width        = width;
    d->height       = height;
    d->bytesPerLine = bytesPerLine;
    d->frmt         = ksaneFormat;
    d->ksaneData    = ksaneData;
}

void SaveImgThread::setTargetFile(const QUrl& url, const QString& format)
{
    d->newUrl = url;
    d->format = format;
}

void SaveImgThread::setScannerModel(const QString& make, const QString& model)
{
    d->make  = make;
    d->model = model;
}

void SaveImgThread::run()
{
    bool sixteenBit = (d->frmt == KSaneWidget::FormatRGB_16_C);
    DImg img((uint)d->width, (uint)d->height, sixteenBit, false);

    if (!sixteenBit)
    {
        uchar* src = (uchar*)d->ksaneData.data();
        uchar* dst = img.bits();

        for (int w = 0; w < d->width; ++w)
        {
            dst[0]  = src[2];    // Blue
            dst[1]  = src[1];    // Green
            dst[2]  = src[0];    // Red
            dst[3]  = 0x00;      // Alpha

            dst    += 4;
            src    += 3;
        }
    }
    else
    {
        unsigned short* src = (unsigned short*)d->ksaneData.data();
        unsigned short* dst = (unsigned short*)img.bits();

        for (int w = 0; w < d->width; ++w)
        {
            dst[0]  = src[2];    // Blue
            dst[1]  = src[1];    // Green
            dst[2]  = src[0];    // Red
            dst[3]  = 0x0000;    // Alpha

            dst    += 4;
            src    += 3;
        }
    }

    bool success = img.save(d->newUrl.toLocalFile(), d->format);

    if (!success)
    {
        emit signalComplete(d->newUrl, success);
        return;
    }
 
    DMetadata meta(d->newUrl.toLocalFile());
    meta.setExifTagString("Exif.Image.DocumentName", QLatin1String("Scanned Image")); // not i18n
    meta.setExifTagString("Exif.Image.Make",  d->make);
    meta.setXmpTagString("Xmp.tiff.Make",     d->make);
    meta.setExifTagString("Exif.Image.Model", d->model);
    meta.setXmpTagString("Xmp.tiff.Model",    d->model);
    meta.setImageOrientation(DMetadata::ORIENTATION_NORMAL);
    meta.setImageColorWorkSpace(DMetadata::WORKSPACE_SRGB);
    meta.applyChanges();

    emit signalComplete(d->newUrl, success);
}

}  // namespace Digikam

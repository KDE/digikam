/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-24
 * Description : utils to generate video frames.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "frameutils.h"

// Qt includes

#include <QColor>
#include <QPainter>

// Local includes

#include "dimg.h"
#include "drawdecoding.h"
#include "digikam_config.h"
#include "digikam_debug.h"

namespace Digikam
{

QImage FrameUtils::makeFramedImage(const QString& file, const QSize& outSize)
{
    QImage timg;

    if (!file.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Load frame from" << file;

        // The Raw decoding settings for DImg loader.
        DRawDecoderSettings settings;
        settings.halfSizeColorImage    = false;
        settings.sixteenBitsImage      = false;
        settings.RGBInterpolate4Colors = false;
        settings.RAWQuality            = DRawDecoderSettings::BILINEAR;

        DImg dimg(file, 0, DRawDecoding(settings));
        timg = dimg.copyQImage();
    }

    return makeScaledImage(timg, outSize);
}

QImage FrameUtils::makeScaledImage(QImage& timg, const QSize& outSize)
{
    QImage qimg(outSize, QImage::Format_ARGB32);
    qimg.fill(QColor(0, 0, 0, 255));

    if (!timg.isNull())
    {
        timg = timg.scaled(outSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QPainter p(&qimg);
        p.drawImage((qimg.width()  / 2) - (timg.width()  / 2),
                    (qimg.height() / 2) - (timg.height() / 2),
                    timg);
    }

    return qimg;
}

} // namespace Digikam

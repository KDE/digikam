/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-03-12
 * Description : a command line tool to test autocrop of DImg
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012-2013 by Sayantan Datta <sayantan dot knz at gmail dot com>
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

#include <QFileInfo>
#include <QString>
#include <QRect>
#include <QDebug>

// Local includes

#include "dimg.h"
#include "drawdecoding.h"
#include "autocrop.h"
#include "dimgthreadedfilter.h"
#include "metaengine.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        qDebug() << "testautocrop - test auto-crop transform";
        qDebug() << "Usage: <image>";
        return -1;
    }

    MetaEngine::initializeExiv2();

    QFileInfo input(QString::fromUtf8(argv[1]));
    QString   outFilePath(input.baseName() + QLatin1String(".cropped.png"));

    DRawDecoderSettings settings;
    settings.halfSizeColorImage    = false;
    settings.sixteenBitsImage      = true;
    settings.RGBInterpolate4Colors = false;
    settings.RAWQuality            = DRawDecoderSettings::BILINEAR;

    DImg img(input.filePath(), 0, DRawDecoding(settings));

    AutoCrop ac(&img);
    ac.startFilterDirectly();
    QRect rect = ac.autoInnerCrop();

    qDebug() << "Cropped image area: " << rect;

    img.crop(rect);
    img.save(outFilePath, QLatin1String("PNG"));

    MetaEngine::cleanupExiv2();

    return 0;
}

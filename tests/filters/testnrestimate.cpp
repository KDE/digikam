/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-10-23
 * Description : a command line tool to test DImg image loader
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
#include <QDebug>

// Local includes

#include "dimg.h"
#include "drawdecoding.h"
#include "nrestimate.h"
#include "nrfilter.h"
#include "dimgthreadedfilter.h"
#include "metaengine.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        qDebug() << "testnrestimate - test NR parameters";
        qDebug() << "Usage: <image>";
        return -1;
    }

    MetaEngine::initializeExiv2();

    QFileInfo input(QString::fromUtf8(argv[1]));
    QString   outFilePath(input.baseName() + QLatin1String(".denoise.png"));

    DRawDecoderSettings settings;
    settings.halfSizeColorImage    = false;
    settings.sixteenBitsImage      = true;
    settings.RGBInterpolate4Colors = false;
    settings.RAWQuality            = DRawDecoderSettings::BILINEAR;

    DImg img(input.filePath(), 0, DRawDecoding(settings));
    NREstimate nre(&img);
    nre.setLogFilesPath(input.filePath());
    nre.startFilterDirectly();
    NRContainer prm = nre.settings();

    qDebug() << prm;

    NRFilter nrf(&img, 0, prm);
    nrf.startFilterDirectly();
    img.putImageData(nrf.getTargetImage().bits());
    img.save(outFilePath, QLatin1String("PNG"));

    MetaEngine::cleanupExiv2();

    return 0;
}

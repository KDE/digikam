/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2012-10-23
 * @brief  a command line tool to test DImg image loader
 *
 * @author Copyright (C) 2012 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Qt includes

#include <QString>
#include <QFileInfo>

// KDE includes

#include "kdebug.h"

// LibKExiv2 includes

#include <libkexiv2/kexiv2.h>

// Local includes

#include "dimg.h"
#include "drawdecoding.h"

using namespace Digikam;
using namespace KExiv2Iface;
using namespace KDcrawIface;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        kDebug() << "testdimgloader - test test DImg image loader";
        kDebug() << "Usage: <image>";
        return -1;
    }

    KExiv2::initializeExiv2();

    QString filePath(argv[1]);
    bool valRet = false;

    QFileInfo           input(filePath);
    QString             outFilePath(input.baseName() + QString(".out.png"));
    DImg                img;
    RawDecodingSettings settings;
    settings.halfSizeColorImage    = false;
    settings.sixteenBitsImage      = false;
    settings.RGBInterpolate4Colors = false;
    settings.RAWQuality            = RawDecodingSettings::AHD;

    img.load(filePath, 0, DRawDecoding(settings));
    img.save(outFilePath, "PNG");

    KExiv2::cleanupExiv2();

    return valRet;
}

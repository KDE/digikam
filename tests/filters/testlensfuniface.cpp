/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-15
 * Description : a command line tool to parse metadata dedicated to LensFun
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QString>
#include <QFile>
#include <QDebug>

// Local includes

#include "lensfuniface.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        qDebug() << "testlensfuniface - test to parse metadata dedicated to LensFun";
        qDebug() << "Usage: <image>";
        return -1;
    }

    MetaEngine::initializeExiv2();

    QString filePath(QString::fromUtf8(argv[1]));
    bool valRet = false;

    DImg         img(filePath);
    DMetadata    meta(img.getMetadata());
    LensFunIface iface;
    LensFunIface::MetadataMatch ret = iface.findFromMetadata(meta);

    if (ret == LensFunIface::MetadataExactMatch)
    {
        LensFunFilter filter(&img, 0L, iface.settings());
        filter.startFilterDirectly();
        img.putImageData(filter.getTargetImage().bits());

        Digikam::MetaEngineData data = img.getMetadata();
        filter.registerSettingsToXmp(data);
        img.setMetadata(data);
        img.save(QLatin1String("lensfuniface-output.png"), QLatin1String("PNG"));

        valRet = true;
    }

    MetaEngine::cleanupExiv2();

    return valRet;
}

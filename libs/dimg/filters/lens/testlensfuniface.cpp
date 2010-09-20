/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-09-15
 * @brief  a command line tool to parse metadata dedicated to LensFun
 *
 * @author Copyright (C) 2010 by Gilles Caulier
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
#include <QFile>

// KDE includes.

#include "kdebug.h"

// Local includes.

#include "dmetadata.h"
#include "lensfuniface.h"

using namespace Digikam;

int main (int argc, char** argv)
{
    if(argc != 2)
    {
        kDebug() << "testlensfuniface - test to parse metadata dedicated to LensFun";
        kDebug() << "Usage: <image>";
        return -1;
    }

    QString filePath(argv[1]);

    DImg             img(filePath);
    DMetadata        meta(img.getMetadata());
    LensFunIface     iface;
    LensFunContainer settings;
    bool ret = iface.findFromMetadata(meta, settings);
    if (ret)
    {
        iface.setSettings(settings);
        LensFunFilter filter(&img, 0L, &iface);
        filter.startFilterDirectly();
        img.putImageData(filter.getTargetImage().bits());
        return img.save("lensfuniface-output.png", "PNG");
    }

    return false;
}

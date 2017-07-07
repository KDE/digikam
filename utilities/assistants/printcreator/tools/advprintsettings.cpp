/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-07
 * Description : a tool to print images
 *
 * Copyright (C) 2007-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "advprintsettings.h"

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

namespace Digikam
{

AdvPrintSettings::AdvPrintSettings()
{
    selMode   = IMAGES;
    // select a different page to force a refresh in initPhotoSizes.
    pageSize  = QSizeF(-1, -1);
}

AdvPrintSettings::~AdvPrintSettings()
{
    for (int i = 0 ; i < photos.count() ; ++i)
        delete photos.at(i);

    photos.clear();
}

void AdvPrintSettings::readSettings(KConfigGroup& group)
{
    selMode  = (Selection)group.readEntry("SelMode",
               (int)IMAGES);

}

void AdvPrintSettings::writeSettings(KConfigGroup& group)
{
    group.writeEntry("SelMode",  (int)selMode);
}

/*
QMap<AdvPrintSettings::ImageFormat, QString> AdvPrintSettings::imageFormatNames()
{
    QMap<ImageFormat, QString> frms;

    frms[JPEG] = i18nc("Image format: JPEG", "Jpeg");
    frms[PNG]  = i18nc("Image format: PNG",  "Png");

    return frms;
}
*/
} // namespace Digikam

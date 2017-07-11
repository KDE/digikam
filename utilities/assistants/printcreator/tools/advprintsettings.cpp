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
    selMode            = IMAGES;
    // select a different page to force a refresh in initPhotoSizes.
    pageSize           = QSizeF(-1, -1);
    currentPreviewPage = 0;
    currentCropPhoto   = 0;
    disableCrop        = false;
    imageFormat        = JPEG;
}

AdvPrintSettings::~AdvPrintSettings()
{
    for (int i = 0 ; i < photos.count() ; ++i)
        delete photos.at(i);

    photos.clear();
}

QString AdvPrintSettings::format() const
{
    if (imageFormat == JPEG)
        return QLatin1String("JPEG");
    else if (imageFormat == TIFF)
        return QLatin1String("TIF");

    return QLatin1String("PNG");
}

QMap<AdvPrintSettings::ImageFormat, QString> AdvPrintSettings::imageFormatNames()
{
    QMap<ImageFormat, QString> frms;

    frms[JPEG] = i18nc("Image format: JPEG", "JPEG");
    frms[PNG]  = i18nc("Image format: PNG",  "PNG");
    frms[TIFF] = i18nc("Image format: TIFF", "TIFF");

    return frms;
}


} // namespace Digikam

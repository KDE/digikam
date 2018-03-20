/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-01-24
 * Description : Web Service settings container.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "wssettings.h"

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

namespace Digikam
{

WSSettings::WSSettings()
{
    selMode           = IMAGES;
    addFileProperties = false;
    imagesChangeProp  = false;
    removeMetadata    = false;
    imageCompression  = 75;
    webService        = FLICKR;
    imageSize         = 1024;
    imageFormat       = JPEG;
}

WSSettings::~WSSettings()
{
}

void WSSettings::readSettings(KConfigGroup& group)
{
    selMode           = (Selection)group.readEntry("SelMode",
                        (int)IMAGES);
    addFileProperties = group.readEntry("AddCommentsAndTags",
                        false);
    imagesChangeProp  = group.readEntry("ImagesChangeProp",
                        false);
    removeMetadata    = group.readEntry("RemoveMetadata",
                        false);
    imageCompression  = group.readEntry("ImageCompression",
                        75);
    webService        = (WebService)group.readEntry("WebService",
                        (int)FLICKR);
    imageSize         = group.readEntry("ImageSize",
                        1024);
    imageFormat       = (ImageFormat)group.readEntry("ImageFormat",
                        (int)JPEG);
}

void WSSettings::writeSettings(KConfigGroup& group)
{
    group.writeEntry("SelMode",            (int)selMode);
    group.writeEntry("AddCommentsAndTags", addFileProperties);
    group.writeEntry("ImagesChangeProp",   imagesChangeProp);
    group.writeEntry("RemoveMetadata",     removeMetadata);
    group.writeEntry("ImageCompression",   imageCompression);
    group.writeEntry("WebService",         (int)webService);
    group.writeEntry("ImageSize",          imageSize);
    group.writeEntry("ImageFormat",        (int)imageFormat);
}

QString WSSettings::format() const
{
    if (imageFormat == JPEG)
        return QLatin1String("JPEG");

    return QLatin1String("PNG");
}

void WSSettings::setMailUrl(const QUrl& orgUrl, const QUrl& emailUrl)
{
    itemsList.insert(orgUrl, emailUrl);
}

QUrl WSSettings::mailUrl(const QUrl& orgUrl) const
{
    if (itemsList.contains(orgUrl))
    {
        return itemsList.find(orgUrl).value();
    }

    return QUrl();
}

QMap<WSSettings::WebService, QString> WSSettings::webServiceNames()
{
    QMap<WebService, QString> services;

    services[FLICKR]    = i18nc("Web Service: FLICKR",    "Flickr");
    services[DROPBOX]   = i18nc("Web Service: DROPBOX",   "Dropbox");
    services[IMGUR]     = i18nc("Web Service: IMGUR",     "Imgur");

    return services;
}

QMap<WSSettings::ImageFormat, QString> WSSettings::imageFormatNames()
{
    QMap<ImageFormat, QString> frms;

    frms[JPEG] = i18nc("Image format: JPEG", "Jpeg");
    frms[PNG]  = i18nc("Image format: PNG",  "Png");

    return frms;
}

} // namespace Digikam

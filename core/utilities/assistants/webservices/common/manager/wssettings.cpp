/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-01-24
 * Description : Web Service settings container.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

// Qt includes

#include <QSettings>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "wstoolutils.h"
#include "o0globals.h"

namespace Digikam
{

WSSettings::WSSettings(QObject* const parent)
  : QObject(parent)
{
    selMode             = EXPORT;
    addFileProperties   = false;
    imagesChangeProp    = false;
    removeMetadata      = false;
    imageCompression    = 75;
    webService          = FLICKR;
    currentAlbumId      = QLatin1String("");
    userName            = QLatin1String("");
    oauthSettings       = WSToolUtils::getOauthSettings(parent);
    oauthSettingsStore  = new O0SettingsStore(oauthSettings, QLatin1String(O2_ENCRYPTION_KEY), this);
    imageSize           = 1024;
    imageFormat         = JPEG;
}

WSSettings::~WSSettings()
{
    delete oauthSettings;
    delete oauthSettingsStore;
}

void WSSettings::readSettings(KConfigGroup& group)
{
    selMode           = (Selection)group.readEntry("SelMode",
                        (int)EXPORT);
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
    userName          = group.readEntry("UserName",
                        QString(""));
    currentAlbumId    = group.readEntry("Album",
                        QString(""));
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
    group.writeEntry("UserName",           userName);
    group.writeEntry("Album",              currentAlbumId);
    group.writeEntry("ImageSize",          imageSize);
    group.writeEntry("ImageFormat",        (int)imageFormat);
}

QString WSSettings::format() const
{
    if (imageFormat == JPEG)
        return QLatin1String("JPEG");

    return QLatin1String("PNG");
}

QMap<WSSettings::WebService, QString> WSSettings::webServiceNames()
{
    QMap<WebService, QString> services;

    services[FLICKR]    = i18nc("Web Service: FLICKR",    "Flickr");
    services[DROPBOX]   = i18nc("Web Service: DROPBOX",   "Dropbox");
    services[IMGUR]     = i18nc("Web Service: IMGUR",     "Imgur");
    services[FACEBOOK]  = i18nc("Web Service: FACEBOOK",  "Facebook");
    services[SMUGMUG]   = i18nc("Web Service: SMUGMUG",   "Smugmug");
    services[GDRIVE]    = i18nc("Web Service: GDRIVE",    "Google Drive");
    services[GPHOTO]    = i18nc("Web Service: GPHOTO",    "Google Photo");

    return services;
}

QMap<WSSettings::ImageFormat, QString> WSSettings::imageFormatNames()
{
    QMap<ImageFormat, QString> frms;

    frms[JPEG] = i18nc("Image format: JPEG", "Jpeg");
    frms[PNG]  = i18nc("Image format: PNG",  "Png");

    return frms;
}

QStringList WSSettings::allUserNames(const QString& serviceName)
{
    QStringList userNames;

    oauthSettings->beginGroup(serviceName);
    oauthSettings->beginGroup("users");
    userNames = oauthSettings->allKeys();
    oauthSettings->endGroup();
    oauthSettings->endGroup();

    return userNames;
}

} // namespace Digikam

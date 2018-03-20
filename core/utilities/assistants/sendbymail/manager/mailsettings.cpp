/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-07
 * Description : mail settings container.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Andi Clemens <andi dot clemens at googlemail dot com>
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

#include "mailsettings.h"

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

namespace Digikam
{

MailSettings::MailSettings()
{
    selMode           = IMAGES;
    addFileProperties = false;
    imagesChangeProp  = false;
    removeMetadata    = false;
    attLimitInMbytes  = 17;
    imageCompression  = 75;
    mailProgram       = THUNDERBIRD;
    imageSize         = 1024;
    imageFormat       = JPEG;
}

MailSettings::~MailSettings()
{
}

void MailSettings::readSettings(KConfigGroup& group)
{
    selMode           = (Selection)group.readEntry("SelMode",
                        (int)IMAGES);
    addFileProperties = group.readEntry("AddCommentsAndTags",
                        false);
    imagesChangeProp  = group.readEntry("ImagesChangeProp",
                        false);
    removeMetadata    = group.readEntry("RemoveMetadata",
                        false);
    attLimitInMbytes  = group.readEntry("AttLimitInMbytes",
                        17);
    imageCompression  = group.readEntry("ImageCompression",
                        75);
    mailProgram       = (MailClient)group.readEntry("MailProgram",
                        (int)THUNDERBIRD);
    imageSize         = group.readEntry("ImageSize",
                        1024);
    imageFormat       = (ImageFormat)group.readEntry("ImageFormat",
                        (int)JPEG);
}

void MailSettings::writeSettings(KConfigGroup& group)
{
    group.writeEntry("SelMode",            (int)selMode);
    group.writeEntry("AddCommentsAndTags", addFileProperties);
    group.writeEntry("ImagesChangeProp",   imagesChangeProp);
    group.writeEntry("RemoveMetadata",     removeMetadata);
    group.writeEntry("AttLimitInMbytes",   attLimitInMbytes);
    group.writeEntry("ImageCompression",   imageCompression);
    group.writeEntry("MailProgram",        (int)mailProgram);
    group.writeEntry("ImageSize",          imageSize);
    group.writeEntry("ImageFormat",        (int)imageFormat);
}

QString MailSettings::format() const
{
    if (imageFormat == JPEG)
        return QLatin1String("JPEG");

    return QLatin1String("PNG");
}

void MailSettings::setMailUrl(const QUrl& orgUrl, const QUrl& emailUrl)
{
    itemsList.insert(orgUrl, emailUrl);
}

QUrl MailSettings::mailUrl(const QUrl& orgUrl) const
{
    if (itemsList.contains(orgUrl))
    {
        return itemsList.find(orgUrl).value();
    }

    return QUrl();
}

qint64 MailSettings::attachementLimit() const
{
    qint64 val = attLimitInMbytes * 1024 * 1024;
    return val;
}

QMap<MailSettings::MailClient, QString> MailSettings::mailClientNames()
{
    QMap<MailClient, QString> clients;

    clients[BALSA]         = i18nc("Mail client: BALSA",         "Balsa");
    clients[CLAWSMAIL]     = i18nc("Mail client: CLAWSMAIL",     "Clawsmail");
    clients[EVOLUTION]     = i18nc("Mail client: EVOLUTION",     "Evolution");
    clients[KMAIL]         = i18nc("Mail client: KMAIL",         "Kmail");
    clients[NETSCAPE]      = i18nc("Mail client: NETSCAPE",      "Netscape Messenger");
    clients[SYLPHEED]      = i18nc("Mail client: SYLPHEED",      "Sylpheed");
    clients[THUNDERBIRD]   = i18nc("Mail client: THUNDERBIRD",   "Thunderbird");

    return clients;
}

QMap<MailSettings::ImageFormat, QString> MailSettings::imageFormatNames()
{
    QMap<ImageFormat, QString> frms;

    frms[JPEG] = i18nc("Image format: JPEG", "Jpeg");
    frms[PNG]  = i18nc("Image format: PNG",  "Png");

    return frms;
}

} // namespace Digikam

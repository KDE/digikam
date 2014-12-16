/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to provide camera information to the parser
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "cameranameoption.moc"

// KDE includes

#include <klocale.h>

// Local includes

#include "parser.h"
#include "dmetadata.h"

namespace Digikam
{

CameraNameOption::CameraNameOption()
    : Option(i18n("Camera"), i18n("Add the camera name"), "camera-photo")
{
    QString token("[cam]");
    addToken(token, i18n("Camera name"));

    QRegExp reg(escapeToken(token));
    reg.setMinimal(true);
    setRegExp(reg);
}

QString CameraNameOption::parseOperation(ParseSettings& settings)
{
    QString result;

    ImageInfo info = ImageInfo::fromUrl(settings.fileUrl);

    if (!info.isNull())
    {
        result = info.photoInfoContainer().make + ' ' + info.photoInfoContainer().model;
    }
    else
    {
        // If ImageInfo is not available, read the information from the EXIF data
        QString make;
        QString model;

        DMetadata meta(settings.fileUrl.toLocalFile());

        if (!meta.isEmpty())
        {
            KExiv2::MetaDataMap dataMap;
            dataMap = meta.getExifTagsDataList(QStringList(), true);

            foreach(const QString& key, dataMap.keys())
            {
                if (key.toLower().contains("exif.image.model"))
                {
                    model = dataMap[key];
                }
                else if (key.toLower().contains("exif.image.make"))
                {
                    make = dataMap[key];
                }
            }
        }

        result = make + ' ' + model;
    }

    return result.simplified();
}

} // namespace Digikam

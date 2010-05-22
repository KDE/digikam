/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-22
 * Description : image category key
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "mediatypekey.h"

// KDE includes

#include <klocale.h>

// local includes

#include "imageinfo.h"
#include "albuminfo.h"

namespace Digikam
{

MediaTypeKey::MediaTypeKey(bool localized)
            : DbOptionKey()
{
    name = QString("MediaType");
    QString desc("Media type");
    QString desc2("e.g. Image, Audio, Movie");
    description = i18n("%1 (%2)", desc, desc2);

    isLocalized = localized;
    if (isLocalized)
    {
        name.append("Loc");
        description = i18n("%1 (localized)", desc);
    }
}

QString MediaTypeKey::getDbValue(ParseSettings& settings)
{
    ImageInfo info(settings.fileUrl);
    QString result;
    switch (info.category())
    {
        case DatabaseItem::UndefinedCategory:
            result = isLocalized ? i18n("Undefined") : QString("Undefined");
            break;
        case DatabaseItem::Image:
            result = isLocalized ? i18n("Image") : QString("Image");
            break;
        case DatabaseItem::Video:
            result = isLocalized ? i18n("Video") : QString("Video");
            break;
        case DatabaseItem::Audio:
            result = isLocalized ? i18n("Audio") : QString("Audio");
            break;
        case DatabaseItem::Other:
        default:
            result = isLocalized ? i18n("Other") : QString("Other");
            break;
    }
    return result;
}

} // namespace Digikam

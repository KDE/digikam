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

#include "categorykey.h"

// KDE includes

#include <klocale.h>

// local includes

#include "imageinfo.h"
#include "albuminfo.h"

namespace Digikam
{

CategoryKey::CategoryKey()
           : DbOptionKey(QString("Category"), i18n("File category (e.g. Image, Audio, Movie)"))
{
}

QString CategoryKey::getDbValue(ParseSettings& settings)
{
    ImageInfo info(settings.fileUrl);
    QString result;
    switch (info.category())
    {
        case DatabaseItem::UndefinedCategory: result = i18n("Undefined"); break;
        case DatabaseItem::Image:             result = i18n("Image");     break;
        case DatabaseItem::Video:             result = i18n("Video");     break;
        case DatabaseItem::Audio:             result = i18n("Audio");     break;
        case DatabaseItem::Other:
        default:                              result = i18n("Other");     break;
    }
    return result;
}

} // namespace Digikam

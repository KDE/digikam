/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-22
 * Description : rating key
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

#include "ratingkey.h"

// KDE includes

#include <klocale.h>

// local includes

#include "imageinfo.h"

namespace Digikam
{

RatingKey::RatingKey()
         : DbOptionKey(QString("Rating"), i18n("Rating of the image"))
{
}

QString RatingKey::getDbValue(ParseSettings& settings)
{
    ImageInfo info(settings.fileUrl);
    return QString::number(info.rating());
}

} // namespace Digikam

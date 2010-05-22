/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-22
 * Description : image dimension key
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

#include "dimensionkey.h"

// KDE includes

#include <klocale.h>

// local includes

#include "imageinfo.h"

namespace Digikam
{

DimensionKey::DimensionKey()
            : DbOptionKey(QString("Dimension"), i18n("Image dimension"))
{
}

QString DimensionKey::getDbValue(ParseSettings& settings)
{
    ImageInfo info(settings.fileUrl);
    QSize dimension = info.dimensions();
    if (dimension.isEmpty() || dimension.isNull() || !dimension.isValid())
    {
        dimension.setWidth(0);
        dimension.setHeight(0);
    }
    return QString("%1x%2").arg(dimension.width()).arg(dimension.height());
}

} // namespace Digikam

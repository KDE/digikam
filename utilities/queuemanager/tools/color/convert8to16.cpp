/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-16
 * Description : 8 to 16 bits color depth converter batch tool.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "convert8to16.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "dimgbuiltinfilter.h"

namespace Digikam
{

Convert8to16::Convert8to16(QObject* const parent)
    : BatchTool(QLatin1String("Convert8to16"), ColorTool, parent)
{
    setToolTitle(i18n("Convert to 16 bits"));
    setToolDescription(i18n("Convert color depth from 8 to 16 bits."));
    setToolIconName(QLatin1String("depth8to16"));
}

Convert8to16::~Convert8to16()
{
}

bool Convert8to16::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    DImgBuiltinFilter filter(DImgBuiltinFilter::ConvertTo16Bit);
    applyFilter(&filter);

    return (savefromDImg());
}

}  // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-16
 * Description : 16 to 8 bits color depth converter batch tool.
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

#include "convert16to8.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "dimgbuiltinfilter.h"

namespace Digikam
{

Convert16to8::Convert16to8(QObject* const parent)
    : BatchTool(QLatin1String("Convert16to8"), ColorTool, parent)
{
    setToolTitle(i18n("Convert to 8 bits"));
    setToolDescription(i18n("Convert color depth from 16 to 8 bits."));
    setToolIconName(QLatin1String("depth16to8"));
}

Convert16to8::~Convert16to8()
{
}

bool Convert16to8::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    DImgBuiltinFilter filter(DImgBuiltinFilter::ConvertTo8Bit);
    applyFilter(&filter);

    return (savefromDImg());
}

}  // namespace Digikam

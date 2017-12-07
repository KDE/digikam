/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-16
 * Description : Invert colors batch tool.
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

#include "invert.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "invertfilter.h"

namespace Digikam
{

Invert::Invert(QObject* const parent)
    : BatchTool(QLatin1String("Invert"), ColorTool, parent)
{
    setToolTitle(i18n("Invert Colors"));
    setToolDescription(i18n("Invert image colors."));
    setToolIconName(QLatin1String("edit-select-invert"));
}

Invert::~Invert()
{
}

bool Invert::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    InvertFilter inv(&image(), 0L);
    applyFilter(&inv);

    return (savefromDImg());
}

}  // namespace Digikam

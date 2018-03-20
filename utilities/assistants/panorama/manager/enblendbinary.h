/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : Autodetect enblend binary program and version
 *
 * Copyright (C) 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef ENBLEND_BINARY_H
#define ENBLEND_BINARY_H

// Local includes

#include "dbinaryiface.h"

namespace Digikam
{

class EnblendBinary : public DBinaryIface
{

public:

    explicit EnblendBinary()
        : DBinaryIface(QLatin1String("enblend"),
                       QLatin1String("4.0"),
                       QLatin1String("enblend "),
                       0,
                       QLatin1String("Hugin"),
                       QLatin1String("http://hugin.sourceforge.net/download/"),
                       QLatin1String("Panorama"),
                       QStringList(QLatin1String("-V"))
                      )
        {
            setup();
        }

    ~EnblendBinary()
    {
    }
};

} // namespace Digikam

#endif  // ENBLEND_BINARY_H

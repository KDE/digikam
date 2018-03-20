/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : Autodetect nona binary program and version
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

#ifndef NONA_BINARY_H
#define NONA_BINARY_H

// Local includes

#include "dbinaryiface.h"



namespace Digikam
{

class NonaBinary : public DBinaryIface
{

public:

    explicit NonaBinary()
        : DBinaryIface(QLatin1String("nona"),
                       QLatin1String("2010.4"),
                       QLatin1String("nona version "),
                       2,
                       QLatin1String("Hugin"),
                       QLatin1String("http://hugin.sourceforge.net/download/"),
                       QLatin1String("Panorama"),
                       QStringList(QLatin1String("-h"))
                      )
        {
            setup();
        }

    ~NonaBinary()
    {
    }
};

} // namespace Digikam

#endif  // NONA_BINARY_H

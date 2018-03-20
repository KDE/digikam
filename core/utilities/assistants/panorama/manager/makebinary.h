/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : Autodetect make binary program
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

#ifndef MAKE_BINARY_H
#define MAKE_BINARY_H

// Local includes

#include "dbinaryiface.h"

namespace Digikam
{

class MakeBinary : public DBinaryIface
{

public:

    explicit MakeBinary()
        : DBinaryIface(QLatin1String("make"),
                       QLatin1String("3.80"),
                       QLatin1String("GNU Make "),
                       0,
                       QLatin1String("GNU"),
#ifdef Q_OS_WIN
                       QLatin1String("http://gnuwin32.sourceforge.net/packages/make.htm"),
#else
                       QLatin1String("http://www.gnu.org/software/make/"),
#endif
                       QLatin1String("Panorama"),
                       QStringList(QLatin1String("-v"))
                      )
        {
            setup();
        }

    ~MakeBinary()
    {
    }
};

} // namespace Digikam

#endif  // MAKE_BINARY_H

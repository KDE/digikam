/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-05-16
 * Description : Autodetects pano_modify binary program and version
 *
 * Copyright (C) 2013-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#ifndef PANO_MODIFY_BINARY_H
#define PANO_MODIFY_BINARY_H

// Local includes

#include "dbinaryiface.h"

namespace Digikam
{

class PanoModifyBinary : public DBinaryIface
{

public:

    explicit PanoModifyBinary()
        : DBinaryIface(QLatin1String("pano_modify"),
                       QLatin1String("2012.0"),
                       QLatin1String("pano_modify version "),
                       1,
                       QLatin1String("Hugin"),
                       QLatin1String("http://hugin.sourceforge.net/download/"),
                       QLatin1String("Panorama"),
                       QStringList(QLatin1String("-h"))
                      )
        {
            setup();
        }
};

} // namespace Digikam

#endif // PANO_MODIFY_BINARY_H

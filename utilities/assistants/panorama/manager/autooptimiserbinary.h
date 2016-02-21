/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : Autodetect autooptimiser binary program and version
 *
 * Copyright (C) 2011-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#ifndef AUTOOPTIMISERBINARY_H
#define AUTOOPTIMISERBINARY_H

// Local includes

#include "dbinaryiface.h"



namespace Digikam
{

class AutoOptimiserBinary : public DBinaryIface
{

public:

    AutoOptimiserBinary()
        : DBinaryIface(QStringLiteral("autooptimiser"), 
                        QStringLiteral("2010.4"), 
                        QStringLiteral("autooptimiser version "),
                        1, 
                        QStringLiteral("Hugin"), 
                        QStringLiteral("http://hugin.sourceforge.net"), 
                        QStringLiteral("Panorama")
                       )
        { 
            setup();
        }

    ~AutoOptimiserBinary()
    {
    }
};

} // namespace Digikam

#endif  // AUTOOPTIMISERBINARY_H

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : Autodetect pto2mk binary program and version
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

#ifndef PTO2MKBINARY_H
#define PTO2MKBINARY_H

// Local includes

#include "dbinaryiface.h"



namespace Digikam
{

class Pto2MkBinary : public DBinaryIface
{

public:

    Pto2MkBinary()
        : DBinaryIface(QStringLiteral("pto2mk"),
                        QStringLiteral("2010.4"), 
                        QStringLiteral("pto2mk version "),
                        2, 
                        QStringLiteral("Hugin"),
                        QStringLiteral("http://hugin.sourceforge.net"),
                        QStringLiteral("Panorama"),
                        QStringList(QStringLiteral("-h"))
                       )
        { 
            setup(); 
        }

    ~Pto2MkBinary()
    {
    }
};

} // namespace Digikam

#endif  // PTO2MKBINARY_H

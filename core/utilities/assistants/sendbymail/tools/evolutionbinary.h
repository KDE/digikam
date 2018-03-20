/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect evolution binary program
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef EVOLUTION_BINARY_H
#define EVOLUTION_BINARY_H

// Local includes

#include "dbinaryiface.h"

namespace Digikam
{

class EvolutionBinary : public DBinaryIface
{
public:

    explicit EvolutionBinary()
        : DBinaryIface(
                       QLatin1String("evolution"),
                       QLatin1String("Evolution"),
                       QLatin1String("https://wiki.gnome.org/Apps/Evolution"),
                       QLatin1String("SendByMail"),
                       QStringList(QLatin1String("-v")),
                       i18n("Gnome Mail Client.")
                      )
        {
            setup();
        }

    ~EvolutionBinary()
    {
    }
};

} // namespace Digikam

#endif // EVOLUTION_BINARY_H

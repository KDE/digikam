/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect claws-mail binary program
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CLAWS_MAIL_BINARY_H
#define CLAWS_MAIL_BINARY_H

// Local includes

#include "dbinaryiface.h"

namespace Digikam
{

class ClawsMailBinary : public DBinaryIface
{
public:

    explicit ClawsMailBinary()
        : DBinaryIface(
                       QLatin1String("claws-mail"),
                       QLatin1String("Claws Mail"),
                       QLatin1String("claws-mail"),
                       QLatin1String("SendByMail"),
                       QStringList(QLatin1String("-v")),
                       i18n("GTK based Mail Client.")
                      )
        {
            setup();
        }

    ~ClawsMailBinary()
    {
    }
};

} // namespace Digikam

#endif // CLAWS_MAIL_BINARY_H

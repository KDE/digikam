/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2019-07-06
 * Description : Autodetect outlook binary program
 *
 * Copyright (C) 2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2019 by Maik Qualmann <metzpinguin at gmail dot com>
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

#ifndef OUTLOOK_BINARY_H
#define OUTLOOK_BINARY_H

// Local includes

#include "dbinaryiface.h"

namespace Digikam
{

class OutlookBinary : public DBinaryIface
{
public:

    OutlookBinary()
        : DBinaryIface(
                       QLatin1String("outlook"),
                       QLatin1String("Outlook"),
                       QLatin1String("https://www.microsoft.com/"),
                       QLatin1String("SendByMail"),
                       QStringList(),
                       i18n("Outlook Mail Client.")
                      )
        {
            setup();
        }

    ~OutlookBinary()
    {
    }
};

} // namespace Digikam

#endif // OUTLOOK_BINARY_H

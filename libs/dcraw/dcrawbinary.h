/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date  : 2006-04-13
 * Description : Autodetect dcraw binary
 *
 * Copyright 2006 by Marcel Wiesweg
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

#ifndef DCRAWBINARY_H
#define DCRAWBINARY_H

// Qt includes.

#include <qstring.h>

// Digikam Includes.

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DcrawBinary
{

public:

    static DcrawBinary *instance();
    static void cleanUp();

    bool checkSystem();

    const char *path();
    bool isAvailable();

private:

    DcrawBinary();
    ~DcrawBinary();

    static DcrawBinary *m_instance;

    bool m_available;
};

} // namespace Digikam

#endif  // DCRAWBINARY_H

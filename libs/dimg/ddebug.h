/* ============================================================
 * Authors: Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2006-06-11
 * Description : thread safe debugging.
 * See B.K.O #133026: because kdDebug() is not thread-safe
 * we need to use a dedicaced debug statements in threaded 
 * implementation to prevent crash.
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

#ifndef _DDEBUG_H_
#define _DDEBUG_H_

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT Ddbgstream : public kdbgstream
{

public:

    Ddbgstream(kdbgstream stream);
    ~Ddbgstream();
};

class DIGIKAM_EXPORT Dndbgstream : public kndbgstream
{

public:

    Dndbgstream(kndbgstream stream);
    ~Dndbgstream();
};

} // namespace Digikam

DIGIKAM_EXPORT Digikam::Ddbgstream DDebug(int area = 0);
DIGIKAM_EXPORT Digikam::Ddbgstream DWarning(int area = 0);
DIGIKAM_EXPORT Digikam::Ddbgstream DError(int area = 0);

DIGIKAM_EXPORT Digikam::Dndbgstream DnDebug(int area = 0);

#ifdef NDEBUG
#define DDebug DnDebug
#endif

#endif //  _DDEBUG_H_


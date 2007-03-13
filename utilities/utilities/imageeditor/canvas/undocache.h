/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Joern Ahrens <joern.ahrens@kdemail.net>
 * Date   : 2005-02-05
 * Description :
 *
 * Copyright 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>, Joern Ahrens <joern.ahrens@kdemail.net>
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

#ifndef UNDOCACHE_H
#define UNDOCACHE_H

// QT includes.

#include <qglobal.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class UndoCachePriv;

class DIGIKAM_EXPORT UndoCache
{

public:

    UndoCache();
    ~UndoCache();

    void   clear();
    bool   putData(int level, int w, int h, int bytesDepth, uchar* data);
    uchar *getData(int level, int& w, int& h, int& bytesDepth, bool del=true);

    void   erase(int level);

private:

    UndoCachePriv *d;
};

}  // namespace Digikam

#endif /* UNDOCACHE_H */

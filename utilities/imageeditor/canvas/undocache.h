/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-05
 * Description : undo cache manager for image editor.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005 by Joern Ahrens <joern.ahrens@kdemail.net>
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

// Local includes

#include "digikam_export.h"

namespace Digikam
{


class DIGIKAM_EXPORT UndoCache
{

public:

    UndoCache();
    ~UndoCache();

    /**
     * Delete all cache files
     */
    void   clear();
    /**
     * Delete all cache files starting from the given level upwards
     */
    void   clearFrom(int level);
    /**
    * Write the data into a cache file
    */
    bool   putData(int level, int w, int h, bool sixteenBit, bool hasAlpha, uchar* data);
    /**
    * Get the data from a cache file
    */
    uchar* getData(int level, int& w, int& h, bool& sixteenBit, bool& hasAlpha) const;

private:

    class UndoCachePriv;
    UndoCachePriv* const d;
};

}  // namespace Digikam

#endif /* UNDOCACHE_H */

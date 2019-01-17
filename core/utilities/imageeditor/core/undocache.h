/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-02-05
 * Description : undo cache manager for image editor.
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005      by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_UNDO_CACHE_H
#define DIGIKAM_UNDO_CACHE_H

// Local includes

#include "dimg.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT UndoCache
{

public:

    explicit UndoCache();
    ~UndoCache();

    /**
     * Delete all cache files
     */
    void clear();

    /**
     * Delete all cache files starting from the given level upwards
     */
    void clearFrom(int level);

    /**
     * Write the image data into a cache file
     */
    bool putData(int level, const DImg& img) const;

    /**
     * Get the image data from a cache file
     */
    DImg getData(int level) const;

private:

    UndoCache(const UndoCache&); // Disable

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_UNDO_CACHE_H

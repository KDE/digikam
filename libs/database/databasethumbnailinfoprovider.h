/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-06-08
 * Description : Album database <-> thumbnail database interface
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DATABASETHUMBNAILINFOPROVIDER_H
#define DATABASETHUMBNAILINFOPROVIDER_H

// Local includes

#include "digikam_export.h"
#include "thumbnailinfo.h"
#include "loadsavethread.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT DatabaseThumbnailInfoProvider : public ThumbnailInfoProvider
{
public:

    ThumbnailInfo thumbnailInfo(const ThumbnailIdentifier& identifier);
};

class DIGIKAM_DATABASE_EXPORT DatabaseLoadSaveFileInfoProvider : public LoadSaveFileInfoProvider
{
public:

    virtual int orientationHint(const QString& path);
};


}  // namespace Digikam

#endif /* DATABASETHUMBNAILINFOPROVIDER_H */

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-22-01
 * Description : batch sync pictures metadata with digiKam database
 *
 * Copyright (C) 2007-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef BATCHALBUMSSYNCMETADATA_H
#define BATCHALBUMSSYNCMETADATA_H

// Local includes

#include "imageinfo.h"
#include "maintenanceimginfjobtool.h"

namespace Digikam
{

class BatchAlbumsSyncMetadata : public MaintenanceImgInfJobTool
{
    Q_OBJECT

public:

    BatchAlbumsSyncMetadata();
    ~BatchAlbumsSyncMetadata();

private:

    void gotNewImageInfoList(const ImageInfoList&);
};

}  // namespace Digikam

#endif /* BATCHALBUMSSYNCMETADATA_H */

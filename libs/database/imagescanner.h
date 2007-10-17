/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning of a single image
 *
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGESCANNER_H
#define IMAGESCANNER_H

// Qt includes

#include <QFileInfo>

// Local includes

#include "dimg.h"
#include "dmetadata.h"
#include "albuminfo.h"

namespace Digikam
{

// no export for now
class ImageScanner
{
public:

    ImageScanner(const QFileInfo &info, const ItemScanInfo &info);
    ImageScanner(const QFileInfo &info);
    ImageScanner(qlonglong imageid);

    void fileModified();
    void rescanToDatabase();
    void newFile(int albumId);

protected:

    void scanFile();
    void scanHardInfos();

    void addImage(int albumId);
    void scanImageInformation();
    void scanImageMetadata();
    void scanImagePosition();
    void scanImageComments();
    void scanImageCopyright();
    void scanIPTCCore();
    void scanTags();

    void loadFromDisk();

    QFileInfo m_fileInfo;
    DMetadata m_metadata;
    DImg m_img;
    ItemScanInfo m_scanInfo;

    bool m_hasImage;
    bool m_hasMetadata;
};


}

#endif


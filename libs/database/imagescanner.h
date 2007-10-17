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

class DIGIKAM_EXPORT ImageScanner
{
public:

    /**
     * Construct an ImageScanner object from an existing QFileInfo
     * and ItemScanInfo object.
     * This constructor shall be used with fileModified() or fullScan().
     */
    ImageScanner(const QFileInfo &info, const ItemScanInfo &info);
    /**
     * Construct an ImageScanner from an existing QFileInfo object.
     * Use this constructor if you intend to call newFile().
     */
    ImageScanner(const QFileInfo &info);
    /**
     * Construct an ImageScanner for an image in the database.
     * File info, Scan info and the category will be retrieved from the database.
     */
    ImageScanner(qlonglong imageid);

    /**
     * Inform the scanner about the category of the file.
     * Required at least for newFile() calls, recommended for calls with the
     * first constructor above as well.
     */
    void setCategory(DatabaseItem::Category category);

    /**
     * Call this when you have detected that a file in the database has been
     * modified on disk. Only two groups of fields will be updated in the database:
     * - filesystem specific properties (those that signalled you that the file has been modified
     *   because their state on disk differed from the state in the database)
     * - image specific properties, for which a difference in the database independent from
     *   the actual file does not make sense (width/height, bit depth, color model)
     */
    void fileModified();

    /**
     * Call this to take an existing image in the database, but re-read
     * all information from the file into the database, possibly overwriting
     * information there.
     */
    void fullScan();

    /**
     * Call this when you want ImageScanner to add a new file to the database
     * and read all information into the database.
     */
    void newFile(int albumId);

protected:

    void scanFile();
    void updateHardInfos();

    void addImage(int albumId);
    void updateImage();
    void scanImageInformation();
    void updateImageInformation();
    void scanImageMetadata();
    void scanImagePosition();
    void scanImageComments();
    void scanImageCopyright();
    void scanIPTCCore();
    void scanTags();

    void loadFromDisk();
    QString detectFormat();

    QFileInfo m_fileInfo;
    DMetadata m_metadata;
    DImg m_img;
    ItemScanInfo m_scanInfo;

    bool m_hasImage;
    bool m_hasMetadata;
};


}

#endif


/* ============================================================
 * Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-23
 * Description : JPEG file metadata loader
 *
 * Copyright 2006 by Gilles Caulier
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
 
#ifndef JPEGMETALOADER_H
#define JPEGMETALOADER_H

// Local includes.

#include "dmetaloader.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT JPEGMetaLoader : public DMetaLoader
{

public:

    JPEGMetaLoader(DMetadata* metadata);

    bool load(const QString& filePath);
    bool save(const QString& filePath);

    bool isReadOnly() const { return false; };
};

}  // NameSpace Digikam

#endif /* JPEGLOADER_H */

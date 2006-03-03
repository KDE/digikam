/* ============================================================
 * Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-23
 * Description : PNG file metadata loader
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
 
#ifndef PNGMETALOADER_H
#define PNGMETALOADER_H

// LibPNG includes.

extern "C"
{
#include <png.h>
}

// Local includes.

#include "dmetaloader.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT PNGMetaLoader : public DMetaLoader
{

public:

    PNGMetaLoader(DMetadata* metadata);

    bool load(const QString& filePath);
    bool save(const QString& filePath);

    bool isReadOnly() const { return true; };

private :
    
    uchar* readRawProfile(png_textp text, png_uint_32 *length, int ii);
    void   png_skip_till_end(png_structp png_ptr, png_infop info_ptr);
};

}  // NameSpace Digikam

#endif /* PNGMETALOADER_H */

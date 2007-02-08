/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2005-11-01
 * Description : a PNG image loader for DImg framework.
 * 
 * Copyright 2005-2006 by Gilles Caulier
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

#ifndef PNGLOADER_H
#define PNGLOADER_H

extern "C"
{
#include <stdarg.h>
#include <png.h>
}

// Local includes.

#include "dimgloader.h"
#include "digikam_export.h"

namespace Digikam
{
class DImg;

class DIGIKAM_EXPORT PNGLoader : public DImgLoader
{
public:

    PNGLoader(DImg* image);

    bool load(const QString& filePath, DImgLoaderObserver *observer);
    bool save(const QString& filePath, DImgLoaderObserver *observer);

    virtual bool hasAlpha()   const;
    virtual bool sixteenBit() const;
    virtual bool isReadOnly() const { return false; }; 

private:
    
    void   writeRawProfile(png_struct *ping, png_info *ping_info, char *profile_type, 
                           char *profile_data, png_uint_32 length);

    size_t concatenateString(char *destination, const char *source, const size_t length);
    size_t copyString(char *destination, const char *source, const size_t length);
    long   formatString(char *string, const size_t length, const char *format,...);
    long   formatStringList(char *string, const size_t length, const char *format, va_list operands);

private:

    bool m_sixteenBit;
    bool m_hasAlpha;
};
    
}  // NameSpace Digikam
    
#endif /* PNGLOADER_H */

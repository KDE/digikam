/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date  : 2005-06-17
 * Description : A TIFF IO file for DImg framework
 * 
 * Copyright 2005 by Renchi Raju, Gilles Caulier
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

#ifndef TIFFLOADER_H
#define TIFFLOADER_H

// C ansi includes.

extern "C" 
{
#include <tiffio.h>
#include <tiff.h>
}

// Local includes.

#include "dimgloader.h"
#include "digikam_export.h"

namespace Digikam
{
class DImg;

class DIGIKAM_EXPORT TIFFLoader : public DImgLoader
{
public:

    TIFFLoader(DImg* image);

    bool load(const QString& filePath, DImgLoaderObserver *observer);
    bool save(const QString& filePath, DImgLoaderObserver *observer);

    virtual bool hasAlpha()   const;
    virtual bool sixteenBit() const;
    virtual bool isReadOnly() const { return false; };

private:

    bool m_sixteenBit;
    bool m_hasAlpha;
    
private:

    void getTiffTextTag(TIFF* tif, int tag);
    void setTiffTextTag(TIFF* tif, int tag);
    
    static void dimg_tiff_warning(const char* module, const char* fmt, va_list ap);
    static void dimg_tiff_error(const char* module, const char* fmt, va_list ap);

};

}  // NameSpace Digikam
    
#endif /* TIFFLOADER_H */

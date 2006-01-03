/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2005-11-01
 * Description : A digital camera RAW files loader for DImg 
 *               framework using dcraw program.
 * 
 * Copyright 2005 by Gilles Caulier
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
 * ============================================================ */

#ifndef RAWLOADER_H
#define RAWLOADER_H

// C ansi includes.

extern "C" 
{
#include <setjmp.h>
#include <jpeglib.h>
}

// Local includes.

#include "dimgloader.h"
#include "digikam_export.h"

namespace Digikam
{
class DImg;

class DIGIKAM_EXPORT RAWLoader : public DImgLoader
{
public:

    RAWLoader(DImg* image, bool enableRAWQuality=false, int RAWquality=0, bool RGBInterpolate4Colors=false);

    bool load(const QString& filePath, DImgLoaderObserver *observer);
    bool save(const QString& filePath, DImgLoaderObserver *observer);

    virtual bool hasAlpha()   const;
    virtual bool sixteenBit() const;
    virtual bool isReadOnly() const { return true; };

private:

    bool m_sixteenBit;
    bool m_hasAlpha;

    bool m_RGBInterpolate4Colors;
    bool m_enableRAWQuality;
    int  m_RAWquality;
    
private:

    // To manage Errors/Warnings handling provide by libjpeg
    
    struct dimg_jpeg_error_mgr : public jpeg_error_mgr
    {
        jmp_buf setjmp_buffer;
    };

    // Methods to load RAW image using external dcraw instance.

    bool load8bits(const QString& filePath, DImgLoaderObserver *observer);
    bool load16bits(const QString& filePath, DImgLoaderObserver *observer);

    // Get ICC profiles from RAW files. Any RAW file formats are JPEG like,
    // anothers are TIFF like.

    bool getICCProfileFromJPEG(const QString& filePath);
    bool getICCProfileFromTIFF(const QString& filePath);
};
    
}  // NameSpace Digikam
    
#endif /* RAWLOADER_H */

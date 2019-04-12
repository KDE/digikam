/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-17
 * Description : A TIFF IO file for DImg framework
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
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

#ifndef DIGIKAM_TIFF_LOADER_H
#define DIGIKAM_TIFF_LOADER_H

// C ANSI includes

extern "C"
{
#include <tiffio.h>
#include <tiff.h>
}

// Local includes

#include "dimgloader.h"
#include "digikam_export.h"

namespace Digikam
{

class DImg;
class DMetadata;

class DIGIKAM_EXPORT TIFFLoader : public DImgLoader
{
public:

    explicit TIFFLoader(DImg* const image);

    bool load(const QString& filePath, DImgLoaderObserver* const observer) override;
    bool save(const QString& filePath, DImgLoaderObserver* const observer) override;

    virtual bool hasAlpha()   const override;
    virtual bool sixteenBit() const override;
    virtual bool isReadOnly() const override;

private:

    void tiffSetExifAsciiTag(TIFF* const tif, ttag_t tiffTag, const DMetadata& metaData, const char* const exifTagName);

    // cppcheck-suppress unusedPrivateFunction
    void tiffSetExifDataTag(TIFF* const tif, ttag_t tiffTag, const DMetadata& metaData, const char* const exifTagName);

    static void dimg_tiff_warning(const char* module, const char* format, va_list warnings);
    static void dimg_tiff_error(const char* module, const char* format, va_list errors);

private:

    bool m_sixteenBit;
    bool m_hasAlpha;
};

} // namespace Digikam

#endif // DIGIKAM_TIFF_LOADER_H

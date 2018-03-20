/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : A JPEG IO file for DImg framework
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>, Gilles Caulier
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef JPEGLOADER_H
#define JPEGLOADER_H

// C++ includes

#include <cstdio>
#include <cstdlib>

// C ANSI includes

extern "C"
{
#include <setjmp.h>

// to avoid warnings under win32
#undef HAVE_STDLIB_H
#undef HAVE_STDDEF_H

#include <jpeglib.h>
}

// Local includes

#include "dimgloader.h"
#include "digikam_export.h"

namespace Digikam
{
class DImg;

class DIGIKAM_EXPORT JPEGLoader : public DImgLoader
{

public:

    explicit JPEGLoader(DImg* const image);

    bool load(const QString& filePath, DImgLoaderObserver* const observer);
    bool save(const QString& filePath, DImgLoaderObserver* const observer);

    virtual bool hasAlpha()   const;
    virtual bool sixteenBit() const;
    virtual bool isReadOnly() const;

private:

    // To manage Errors/Warnings handling provide by libjpeg

    struct dimg_jpeg_error_mgr : public jpeg_error_mgr
    {
        jmp_buf setjmp_buffer;
    };

    static void dimg_jpeg_error_exit(j_common_ptr cinfo);
    static void dimg_jpeg_emit_message(j_common_ptr cinfo, int msg_level);
    static void dimg_jpeg_output_message(j_common_ptr cinfo);
};

}  // namespace Digikam

#endif /* JPEGLOADER_H */

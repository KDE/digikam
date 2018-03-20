/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-07-20
 * Description : Loader for thumbnails
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2003-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// C++ includes

#include <cstdlib>
#include <cstdio>
#include <cstring>

// Qt includes

#include <QImage>
#include <QDir>
#include <QCryptographicHash>
#include <QUrl>

// C ANSI includes

extern "C"
{
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef _WIN32
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#include <sys/time.h>
#include <png.h>
}

// Local includes

#include "thumbnailcreator.h"
#include "thumbnailcreator_p.h"

// Definitions

#define PNG_BYTES_TO_CHECK 4

namespace Digikam
{

// --- Static methods: Generate the thumbnail path according to FreeDesktop spec ---

QString ThumbnailCreator::normalThumbnailDir()
{
    return  QDir::homePath() + QLatin1String("/.thumbnails/normal/");
}

QString ThumbnailCreator::largeThumbnailDir()
{
    return  QDir::homePath() + QLatin1String("/.thumbnails/large/");
}

QString ThumbnailCreator::thumbnailPath(const QString& filePath, const QString& basePath)
{
    return thumbnailPathFromUri(thumbnailUri(filePath), basePath);
}

QString ThumbnailCreator::thumbnailUri(const QString& filePath)
{
    return QUrl::fromLocalFile(filePath).url();
}

QString ThumbnailCreator::thumbnailPathFromUri(const QString& uri, const QString& basePath)
{
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData( QFile::encodeName(uri).constData() );
    return ( basePath + QString::fromUtf8(QFile::encodeName(QString::fromUtf8(md5.result().toHex()))) + QLatin1String(".png") );
}

// --- non-static methods ---

void ThumbnailCreator::initThumbnailDirs()
{
    d->smallThumbPath = normalThumbnailDir();
    d->bigThumbPath   = largeThumbnailDir();

    if (!QDir(d->smallThumbPath).exists())
    {
        if (QDir().mkpath(d->smallThumbPath))
        {
            QFile f(d->smallThumbPath);
            f.setPermissions(QFile::ReadUser | QFile::WriteUser | QFile::ExeUser); // 0700
        }
    }

    if (!QDir(d->bigThumbPath).exists())
    {
        if (QDir().mkpath(d->bigThumbPath))
        {
            QFile f(d->bigThumbPath);
            f.setPermissions(QFile::ReadUser | QFile::WriteUser | QFile::ExeUser); // 0700
        }
    }
}

QString ThumbnailCreator::thumbnailPath(const QString& filePath) const
{
    QString basePath = (d->storageSize() == 128) ? d->smallThumbPath : d->bigThumbPath;
    return thumbnailPath(filePath, basePath);
}

// --- Basic PNG loading ---

QImage ThumbnailCreator::loadPNG(const QString& path) const
{
    png_uint_32  w32, h32;
    int          w, h;
    bool         has_alpha = 0;
    png_structp  png_ptr   = NULL;
    png_infop    info_ptr  = NULL;
    int          bit_depth, color_type, interlace_type;
    QImage       qimage;

    FILE* const f = fopen(path.toLatin1().constData(), "rb");

    if (!f)
    {
        return qimage;
    }

    unsigned char buf[PNG_BYTES_TO_CHECK];

    size_t itemsRead = fread(buf, 1, PNG_BYTES_TO_CHECK, f);
#if PNG_LIBPNG_VER >= 10400

    if (itemsRead != 1 || png_sig_cmp(buf, 0, PNG_BYTES_TO_CHECK))
#else
    if (itemsRead != 1 || !png_check_sig(buf, PNG_BYTES_TO_CHECK))
#endif
    {
        fclose(f);
        return qimage;
    }

    rewind(f);

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
    {
        fclose(f);
        return qimage;
    }

    info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(f);
        return qimage;
    }

#if PNG_LIBPNG_VER >= 10400

    if (setjmp(png_jmpbuf(png_ptr)))
#else
    if (setjmp(png_ptr->jmpbuf))
#endif
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(f);
        return qimage;
    }

    png_init_io(png_ptr, f);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, (png_uint_32*) (&w32),
                 (png_uint_32*) (&h32), &bit_depth, &color_type,
                 &interlace_type, NULL, NULL);

    bool  has_grey = 0;
    w              = w32;
    h              = h32;
    qimage         = QImage(w, h, QImage::Format_ARGB32);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_expand(png_ptr);
    }

#if PNG_LIBPNG_VER >= 10400
    png_byte info_color_type = png_get_color_type(png_ptr, info_ptr);
#else
    png_byte info_color_type = info_ptr->color_type;
#endif

    if (info_color_type == PNG_COLOR_TYPE_RGB_ALPHA)
    {
        has_alpha = 1;
    }

    if (info_color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        has_alpha = 1;
        has_grey  = 1;
    }

    if (info_color_type == PNG_COLOR_TYPE_GRAY)
    {
        has_grey = 1;
    }

    unsigned char** lines = 0;
    int             i;

    if (has_alpha)
    {
        png_set_expand(png_ptr);
    }

    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian)           // Intel
    {
        png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
        png_set_bgr(png_ptr);
    }
    else                                                         // PPC
    {
        png_set_swap_alpha(png_ptr);
        png_set_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
    }

    /* 16bit color -> 8bit color */
    if ( bit_depth == 16 )
    {
        png_set_strip_16(png_ptr);
    }

    /* pack all pixels to byte boundaries */

    png_set_packing(png_ptr);

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    {
        png_set_expand(png_ptr);
    }

    lines = (unsigned char**)malloc(h * sizeof(unsigned char*));

    if (!lines)
    {
        png_read_end(png_ptr, info_ptr);
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        fclose(f);
        return qimage;
    }

    if (has_grey)
    {
        png_set_gray_to_rgb(png_ptr);

        if (png_get_bit_depth(png_ptr, info_ptr) < 8)
#if PNG_LIBPNG_VER >= 10400
            png_set_expand_gray_1_2_4_to_8(png_ptr);

#else
            png_set_gray_1_2_4_to_8(png_ptr);
#endif
    }

    int sizeOfUint = sizeof(unsigned int);

    for (i = 0 ; i < h ; ++i)
    {
        lines[i] = ((unsigned char*)(qimage.bits())) + (i * w * sizeOfUint);
    }

    png_read_image(png_ptr, lines);
    free(lines);

    png_textp text_ptr;
    int num_text = 0;
    png_get_text(png_ptr,info_ptr, &text_ptr, &num_text);

    while (num_text--)
    {
        qimage.setText(QString::fromUtf8(text_ptr->key), QString::fromUtf8(text_ptr->text));
        ++text_ptr;
    }

    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
    fclose(f);

    return qimage;
}

}  // namespace Digikam

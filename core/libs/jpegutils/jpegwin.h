/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-22
 * Description : some workaround functions to read jpeg files without relying on libjpeg
 *
 * Copyright (C) 2008 Patrick Spendrin <ps_ml@gmx.de>
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

#ifndef JPEGWIN_H
#define JPEGWIN_H

namespace Digikam
{

namespace JPEGUtils
{

typedef struct
{
    struct jpeg_source_mgr pub;
    JOCTET eoi[2];
} digikam_source_mgr;

void    init_source (j_decompress_ptr cinfo);
boolean fill_input_buffer (j_decompress_ptr cinfo);
void    skip_input_data (j_decompress_ptr cinfo, long nbytes);
void    term_source (j_decompress_ptr cinfo);
void    jpeg_memory_src (j_decompress_ptr cinfo, const JOCTET* buffer, size_t bufsize);

} // namespace JPEGUtils

} // namespace Digikam

#endif // JPEGWIN_H

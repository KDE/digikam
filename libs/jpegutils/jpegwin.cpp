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

extern "C"
{
#include "iccjpeg.h"
}

// C+ includes

#include <cstdio>
#include <cstdlib>

// Qt includes

#include <QFile>
#include <QByteArray>

// Local includes

#include "jpegwin.h"

namespace Digikam
{

namespace JPEGUtils
{

void init_source (j_decompress_ptr /*cinfo*/)
{
}

boolean fill_input_buffer (j_decompress_ptr cinfo)
{
    digikam_source_mgr* src  = (digikam_source_mgr*) cinfo->src;

    /* Create a fake EOI marker */
    src->eoi[0]              = (JOCTET) 0xFF;
    src->eoi[1]              = (JOCTET) JPEG_EOI;
    src->pub.next_input_byte = src->eoi;
    src->pub.bytes_in_buffer = 2;

    return true;
}

void skip_input_data (j_decompress_ptr cinfo, long nbytes)
{
    digikam_source_mgr* src = (digikam_source_mgr*) cinfo->src;

    if (nbytes > 0)
    {
        while (nbytes > (long) src->pub.bytes_in_buffer)
        {
            nbytes -= (long) src->pub.bytes_in_buffer;
            (void) fill_input_buffer(cinfo);
        }

        src->pub.next_input_byte += (size_t) nbytes;
        src->pub.bytes_in_buffer -= (size_t) nbytes;
    }
}

void term_source (j_decompress_ptr /*cinfo*/)
{
}

void jpeg_memory_src (j_decompress_ptr cinfo, const JOCTET* buffer, size_t bufsize)
{
    digikam_source_mgr* src = 0;

    if (cinfo->src == NULL)
    {
        cinfo->src = (struct jpeg_source_mgr*) (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                     sizeof(digikam_source_mgr));
    }

    src                        = (digikam_source_mgr*) cinfo->src;
    src->pub.init_source       = init_source;
    src->pub.fill_input_buffer = fill_input_buffer;
    src->pub.skip_input_data   = skip_input_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart;    // default
    src->pub.term_source       = term_source;
    src->pub.next_input_byte   = buffer;
    src->pub.bytes_in_buffer   = bufsize;
}

} // namespace JPEGUtils

} // namespace Digikam

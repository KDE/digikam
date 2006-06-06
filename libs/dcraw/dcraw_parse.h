/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-12-20
 * Description : Raw Photo Parser
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

#ifndef DCRAW_PARSE_H
#define DCRAW_PARSE_H

// Digikam Includes.

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DcrawParse
{
public:

    DcrawParse();
    ~DcrawParse();

    int getThumbnail(const char* infile, const char* outfile);
    int getCameraModel(const char* infile, char* cameraConstructor, char* cameraModel);

private:

    typedef unsigned char uchar;
    typedef unsigned short ushort;
    typedef long long INT64;

    struct decode
    {
        struct decode *branch[2];
        int leaf;
    } first_decode[640], *free_decode;

    FILE  *ifp;
    short  order;
    char   make[128], model[128], model2[128], thumb_head[128];
    int    width, height, offset, length, bps, is_dng;
    int    thumb_offset, thumb_length, thumb_layers;

private:

    ushort  sget2 (uchar *s);
    ushort  get2();
    int     sget4 (uchar *s);
    int     get4();
    float   int_to_float (int i);
    void    tiff_dump(int base, int tag, int type, int count, int level);
    void    parse_nikon_capture_note (int length);
    void    nikon_decrypt (uchar ci, uchar cj, int tag, int i, int size, uchar *buf);
    void    nef_parse_makernote (int base);
    void    nef_parse_exif(int base);
    void    sony_decrypt (unsigned *data, int len, int start, int key);
    int     parse_tiff_ifd (int base, int level);
    void    parse_tiff (int base);
    void    parse_minolta();
    void    parse_ciff (int offset, int length, int level);
    int     parse_jpeg (int offset);
    void    parse_riff (int level);
    void    parse_mos(int level);
    void    parse_rollei();
    void    rollei_decode (FILE *tfp);
    void    get_utf8 (int offset, char *buf, int len);
    void    parse_foveon();
    void    foveon_tree (unsigned huff[1024], unsigned code);
    void    foveon_decode (FILE *tfp);
    void    kodak_yuv_decode (FILE *tfp);
    void    parse_fuji (int offset);
    void    parse_phase_one (int base);
    char   *memmem (char *haystack, size_t haystacklen, char *needle, size_t needlelen);

};

}  // NameSpace Digikam

#endif /* DCRAW_PARSE_H */

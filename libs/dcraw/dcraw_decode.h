/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-12-20
 * Description :
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
 * 
 * ============================================================ */

#ifndef DCRAW_DECODE_H
#define DCRAW_DECODE_H


// C++ includes

#include <cstdio>

extern "C" 
{
#include <time.h>
#include <setjmp.h>
#include <jpeglib.h>
}

// Digikam Includes.

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DcrawDecode
{
public:

    DcrawDecode();
    ~DcrawDecode();

private:

    typedef long long INT64;
    typedef unsigned long long UINT64;

    typedef unsigned char uchar;
    typedef unsigned short ushort;

    struct decode
    {
        struct decode *branch[2];
        int leaf;
    } first_decode[2048], *second_decode, *free_decode;

    /*
    Not a full implementation of Lossless JPEG, just
    enough to decode Canon, Kodak and Adobe DNG images.
    */
    struct jhead {
        int bits, high, wide, clrs, restart, vpred[4];
        struct decode *huff[4];
        ushort *row;
    };
    
    FILE        *ifp;
    short        order;
    char        *ifname, make[64], model[70], model2[64], *meta_data;
    float        flash_used, canon_ev, iso_speed, shutter, aperture, focal_len;
    time_t       timestamp;
    unsigned     shot_order, kodak_cbpp;
    int          data_offset, meta_offset, meta_length, nikon_curve_offset;
    int          tiff_bps, tiff_data_compression, kodak_data_compression;
    int          raw_height, raw_width, top_margin, left_margin;
    int          height, width, fuji_width, colors, tiff_samples;
    int          black, maximum, clip_max, clip_color;
    int          iheight, iwidth, shrink;
    int          dng_version, is_foveon, raw_color, use_gamma;
    int          flip, xmag, ymag;
    int          zero_after_ff;
    unsigned     filters;
    ushort       (*image)[4], white[8][8], curve[0x1000], cr2_slice[3];
    float        bright, red_scale, blue_scale, sigma_d, sigma_r;
    int          four_color_rgb, document_mode;
    int          verbose, use_auto_wb, use_camera_wb, use_camera_rgb;
    int          fuji_layout, fuji_secondary, use_secondary;
    float        cam_mul[4], pre_mul[4], rgb_cam[3][4];    /* RGB from camera color */

    int          histogram[3][0x2000];

    jmp_buf      failure;
    
    int          profile_offset, profile_length;

private:

    void     merror (void *ptr, char *where);
    ushort   sget2 (uchar *s);
    ushort   get2();
    int      sget4 (uchar *s);
    int      get4();
    double   getrat();
    float    int_to_float (int i);
    void     read_shorts (ushort *pixel, int count);
    
    void     canon_600_fixed_wb (int temp);
    int      canon_600_color (int ratio[2], int mar);
    void     canon_600_auto_wb();
    void     canon_600_coeff();
    void     canon_600_load_raw();
    void     canon_a5_load_raw();

    unsigned getbits (int nbits);
    void     init_decoder();
    void     crw_init_tables (unsigned table);
    int      canon_has_lowbits();
    void     canon_compressed_load_raw();

    int      ljpeg_start (struct jhead *jh);
    int      ljpeg_diff (struct decode *dindex);
    void     ljpeg_row (int jrow, struct jhead *jh);
    void     lossless_jpeg_load_raw();

    void     adobe_copy_pixel (int row, int col, ushort **rp);
    void     adobe_dng_load_raw_lj();
    void     adobe_dng_load_raw_nc();

    void     nikon_compressed_load_raw();
    void     nikon_load_raw();
    int      nikon_is_compressed();
    int      nikon_e995();
    int      nikon_e2100();
    int      nikon_3700();
    void     nikon_e900_load_raw();
    void     nikon_e2100_load_raw();

    void     fuji_load_raw();
    void     rollei_load_raw();
    void     phase_one_load_raw();
    unsigned ph1_bits (int nbits);
    void     phase_one_load_raw_c();
    void     leaf_load_raw();
    void     packed_12_load_raw();
    void     unpacked_load_raw();
    
    void     olympus_e300_load_raw();
    void     olympus_cseries_load_raw();

    int      minolta_z2();
    void     minolta_rd175_load_raw();

    void     eight_bit_load_raw();
    void     casio_qv5700_load_raw();
    void     nucore_load_raw();
    int      radc_token (int tree);

    static   boolean  fill_input_buffer (j_decompress_ptr cinfo);
    void     kodak_jpeg_load_raw();
    void     kodak_radc_load_raw();
    void     kodak_dc120_load_raw();
    void     kodak_easy_load_raw();
    void     kodak_compressed_load_raw();
    void     kodak_yuv_load_raw();

    void     sony_decrypt (unsigned *data, int len, int start, int key);
    void     sony_load_raw();
    void     smal_decode_segment (unsigned seg[2][2], int holes);
    void     smal_v6_load_raw();
    int      median4 (int *p);
    void     fill_holes (int holes);
    void     smal_v9_load_raw();

    void     foveon_decoder (unsigned huff[1024], unsigned code);
    void     foveon_load_camf();
    void     foveon_load_raw();
    char*    foveon_camf_param (char *block, char *param);
    void*    foveon_camf_matrix (int dim[3], char *name);
    int      foveon_fixed (void *ptr, int size, char *name);
    float    foveon_avg (short *pix, int range[2], float cfilt);
    short*   foveon_make_curve (double max, double mul, double filt);
    void     foveon_make_curves(short **curvep, float dq[3], float div[3], float filt);
    int      foveon_apply_curve (short *curve, int i);
    void     foveon_interpolate();

    void     bad_pixels();
    void     pseudoinverse (const double (*in)[3], double (*out)[3], int size);
    void     cam_xyz_coeff (double cam_xyz[4][3]);
    void     colorcheck();
    void     scale_colors();
    void     border_interpolate (int border);
    void     lin_interpolate();
    void     vng_interpolate();
    void     cam_to_cielab (ushort cam[4], float lab[3]);
    void     ahd_interpolate();
    void     bilateral_filter();

    void     parse_makernote();
    void     get_timestamp (int reversed);
    void     parse_exif (int base);
    int      parse_tiff_ifd (int base, int level);
    void     parse_tiff (int base);
    void     parse_minolta();
    void     parse_external_jpeg();
    void     ciff_block_1030();
    void     parse_ciff (int offset, int length);
    void     parse_rollei();
    void     parse_mos (int offset);
    void     parse_phase_one (int base);
    void     parse_fuji (int offset);
    int      parse_jpeg (int offset);
    void     parse_riff();
    void     parse_smal (int offset, int fsize);
    char*    foveon_gets (int offset, char *str, int len);
    void     parse_foveon();
    void     adobe_coeff();
    void     simple_coeff (int index);
    short    guess_byte_order (int words);
    
    int      identify (int no_decode);
    void     apply_profile (char *pfname);
    void     convert_to_rgb();
    void     fuji_rotate();
    void     flip_image();

    void     write_ppm (FILE *ofp);
    void     write_psd (FILE *ofp);
    void     write_ppm16 (FILE *ofp);

    void (DcrawDecode::*load_raw)();
	
    const int*     make_decoder_int (const int *source, int level);
    unsigned char* make_decoder (const unsigned char *source, int level);
    
};

}  // NameSpace Digikam

#endif /* DCRAW_DECODE_H */

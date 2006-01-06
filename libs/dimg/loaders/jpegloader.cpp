/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2005-06-14
 * Description : A JPEG IO file for DImg framework
 *
 * Copyright 2005 by Renchi Raju, Gilles Caulier
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

// JPEG_COM
#define M_COM    0xFE
// JPEG_APP0+1  ==> EXIF marker
#define M_EXIF   0xE1
// JPEG_APP0+13 ==> IPTC marker
#define M_IPTC   0xED

#define XMD_H 

// This line must be commented to prevent any latency time
// when we use threaded image loader interface for each image
// files io. Uncomment this line only for debugging.
//#define ENABLE_DEBUG_MESSAGES 

// C ansi includes.

extern "C" 
{
#include <stdio.h>
#include <stdlib.h>
#include "iccjpeg.h"
}

// QT includes.

#include <qfile.h>
#include <qcstring.h>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "dimg.h"
#include "jpegloader.h"

namespace Digikam
{

// To manage Errors/Warnings handling provide by libjpeg

void JPEGLoader::dimg_jpeg_error_exit(j_common_ptr cinfo)
{
    dimg_jpeg_error_mgr* myerr = (dimg_jpeg_error_mgr*) cinfo->err;

    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);

#ifdef ENABLE_DEBUG_MESSAGES
    kdDebug() << k_funcinfo << buffer << endl;
#endif

    longjmp(myerr->setjmp_buffer, 1);
}

void JPEGLoader::dimg_jpeg_emit_message(j_common_ptr cinfo, int msg_level)
{
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);

#ifdef ENABLE_DEBUG_MESSAGES
    kdDebug() << k_funcinfo << buffer << " (" << msg_level << ")" << endl;
#endif
}

void JPEGLoader::dimg_jpeg_output_message(j_common_ptr cinfo)
{
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);

#ifdef ENABLE_DEBUG_MESSAGES
    kdDebug() << k_funcinfo << buffer << endl;
#endif
}

JPEGLoader::JPEGLoader(DImg* image)
          : DImgLoader(image)
{
}

bool JPEGLoader::load(const QString& filePath, DImgLoaderObserver *observer, bool loadImageData)
{
    FILE *file = fopen(QFile::encodeName(filePath), "rb");
    if (!file)
        return false;

    unsigned short header;

    if (fread(&header, 2, 1, file) != 1)
    {
        fclose(file);
        return false;
    }

    if (header != 0xd8ff)
    {
        // not a jpeg file
        fclose(file);
        return false;
    }

    fseek(file, 0L, SEEK_SET);

    struct jpeg_decompress_struct cinfo;
    struct dimg_jpeg_error_mgr    jerr;

    // -------------------------------------------------------------------
    // JPEG error handling.

    cinfo.err                 = jpeg_std_error(&jerr);
    cinfo.err->error_exit     = dimg_jpeg_error_exit;
    cinfo.err->emit_message   = dimg_jpeg_emit_message;
    cinfo.err->output_message = dimg_jpeg_output_message;

    // If an error occurs during reading, libjpeg will jump here

    if (setjmp(jerr.setjmp_buffer)) 
    {
        jpeg_destroy_decompress(&cinfo);
        fclose(file);
        return false;
    }

    // -------------------------------------------------------------------
    // Set JPEG decompressor instance

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, file);

    jpeg_save_markers(&cinfo, M_COM,  0xFFFF);
    jpeg_save_markers(&cinfo, M_EXIF, 0xFFFF);
    jpeg_save_markers(&cinfo, M_IPTC, 0xFFFF);

    // Recording ICC profile marker (from iccjpeg.c)
    setup_read_icc_profile(&cinfo);

    jpeg_read_header(&cinfo, true);
    cinfo.do_fancy_upsampling = false;
    cinfo.do_block_smoothing  = false;
    cinfo.out_color_space     = JCS_RGB;
    jpeg_start_decompress(&cinfo);

    // some pseudo-progress
    if (observer)
        observer->progressInfo(m_image, 0.1);

    // -------------------------------------------------------------------
    // Get image data.

    int w = cinfo.output_width;
    int h = cinfo.output_height;
    uchar *dest;
    
    if (loadImageData)
    {
        uchar *ptr, *line[16], *data=0;
        uchar *ptr2=0;
        int    x, y, l, i, scans, count, prevy;
    
        if (cinfo.rec_outbuf_height > 16)
        {
            jpeg_destroy_decompress(&cinfo);
            fclose(file);
            kdDebug() << k_funcinfo << "Height of JPEG scanline buffer out of range!" << endl;
            return false;
        }
    
        if (cinfo.output_components != 3 && cinfo.output_components != 1)
        {
            jpeg_destroy_decompress(&cinfo);
            fclose(file);
            kdDebug() << k_funcinfo << "Number of JPEG color components unsupported!" << endl;
            return false;
        }
    
        data = new uchar[w * 16 * 3];
    
        if (!data)
        {
            jpeg_destroy_decompress(&cinfo);
            fclose(file);
            kdDebug() << k_funcinfo << "Cannot allocate memory!" << endl;
            return false;
        }
    
        ptr2 = new uchar[w * h * 4];
    
        if (!ptr2)
        {
            delete [] data;
            jpeg_destroy_decompress(&cinfo);
            fclose(file);
            kdDebug() << k_funcinfo << "Cannot allocate memory!" << endl;
            return false;
        }
    
        dest  = ptr2;
        count = 0;
        prevy = 0;
    
        if (cinfo.output_components == 3)
        {
            for (i = 0; i < cinfo.rec_outbuf_height; i++)
                line[i] = data + (i * w * 3);
    
            int checkPoint = 0;
            for (l = 0; l < h; l += cinfo.rec_outbuf_height)
            {
                // use 0-10% and 90-100% for pseudo-progress
                if (observer && l >= checkPoint)
                {
                    checkPoint += granularity(observer, h, 0.8);
                    if (!observer->continueQuery(m_image))
                    {
                        delete [] data;
                        delete [] ptr2;
                        jpeg_destroy_decompress(&cinfo);
                        fclose(file);
                        return false;
                    }
                    observer->progressInfo(m_image, 0.1 + (0.8 * ( ((float)l)/((float)h) )));
                }
    
                jpeg_read_scanlines(&cinfo, &line[0], cinfo.rec_outbuf_height);
                scans = cinfo.rec_outbuf_height;
    
                if ((h - l) < scans)
                    scans = h - l;
    
                ptr = data;
    
                for (y = 0; y < scans; y++)
                {
                    for (x = 0; x < w; x++)
                    {
                        ptr2[3] = 0xFF;
                        ptr2[2] = ptr[0];
                        ptr2[1] = ptr[1];
                        ptr2[0] = ptr[2];
    
                        ptr  += 3;
                        ptr2 += 4;
                    }
                }
            }
        }
        else if (cinfo.output_components == 1)
        {
            for (i = 0; i < cinfo.rec_outbuf_height; i++)
                line[i] = data + (i * w);
    
            int checkPoint = 0;
            for (l = 0; l < h; l += cinfo.rec_outbuf_height)
            {
                if (observer && l >= checkPoint)
                {
                    checkPoint += granularity(observer, h, 0.8);
                    if (!observer->continueQuery(m_image))
                    {
                        delete [] data;
                        delete [] ptr2;
                        jpeg_destroy_decompress(&cinfo);
                        fclose(file);
                        return false;
                    }
                    observer->progressInfo(m_image, 0.1 + (0.8 * ( ((float)l)/((float)h) )));
                }
    
                jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
                scans = cinfo.rec_outbuf_height;
    
                if ((h - l) < scans)
                    scans = h - l;
    
                ptr = data;
    
                for (y = 0; y < scans; y++)
                {
                    for (x = 0; x < w; x++)
                    {
                        ptr2[3] = 0xFF;
                        ptr2[2] = ptr[0];
                        ptr2[1] = ptr[0];
                        ptr2[0] = ptr[0];
    
                        ptr  ++;
                        ptr2 += 4;
                    }
                }
            }
        }
    
        delete [] data;
    }

    // -------------------------------------------------------------------
    // Get meta-data markers contents.

    QMap<int, QByteArray>& metaData = imageMetaData();
    metaData.clear();

    jpeg_saved_marker_ptr marker = cinfo.marker_list;

    while (marker)
    {
        QByteArray ba(marker->data_length);
        memcpy(ba.data(), marker->data, marker->data_length);

        if (marker->marker == M_COM)
        {
            metaData.insert(DImg::JPG_COM, ba);
        }
        else if (marker->marker == M_EXIF)
        {
            metaData.insert(DImg::JPG_EXIF, ba);
        }
        else if (marker->marker == M_IPTC)
        {
            metaData.insert(DImg::JPG_IPTC, ba);
        }

        marker = marker->next;
    }

    // -------------------------------------------------------------------
    // Read image ICC profile

    JOCTET *profile_data=NULL;
    uint    profile_size;

    read_icc_profile (&cinfo, &profile_data, &profile_size);

    if (profile_data != NULL) 
    {
        QByteArray profile_rawdata = imageICCProfil();
        profile_rawdata.resize(profile_size);
        memcpy(profile_rawdata.data(), profile_data, profile_size);
        free (profile_data);
    }

    // -------------------------------------------------------------------

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    // -------------------------------------------------------------------

    fclose(file);

    if (observer)
        observer->progressInfo(m_image, 1.0);

    imageWidth()  = w;
    imageHeight() = h;
    imageSetAttribute("format", "JPEG");
    
    if (loadImageData)
        imageData() = dest;
    
    return true;
}

bool JPEGLoader::save(const QString& filePath, DImgLoaderObserver *observer)
{
    FILE *file = fopen(QFile::encodeName(filePath), "wb");
    if (!file)
        return false;

    struct jpeg_compress_struct  cinfo;
    struct dimg_jpeg_error_mgr jerr;    

    // -------------------------------------------------------------------
    // JPEG error handling. 
    
    cinfo.err                 = jpeg_std_error(&jerr);
    cinfo.err->error_exit     = dimg_jpeg_error_exit;
    cinfo.err->emit_message   = dimg_jpeg_emit_message;
    cinfo.err->output_message = dimg_jpeg_output_message;
    
    // If an error occurs during writing, libjpeg will jump here
    
    if (setjmp(jerr.setjmp_buffer)) 
    {
        jpeg_destroy_compress(&cinfo);
        fclose(file);
        return false;
    }
    
    // -------------------------------------------------------------------
    // Set JPEG compressor instance
    
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, file);

    uint&              w   = imageWidth();
    uint&              h   = imageHeight();
    unsigned char*& data   = imageData();
    
    cinfo.image_width      = w;
    cinfo.image_height     = h;
    cinfo.input_components = 3;
    cinfo.in_color_space   = JCS_RGB;    
    
    QVariant qualityAttr = imageGetAttribute("quality");
    int quality = qualityAttr.isValid() ? qualityAttr.toInt() : 90;
    
    if (quality < 0)
        quality = 90;
    if (quality > 100)
        quality = 100;
    
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, true);
    jpeg_start_compress(&cinfo, true);

    if (observer)
        observer->progressInfo(m_image, 0.1);

    // -------------------------------------------------------------------
    // Write the markers that we have.
    
    typedef QMap<int, QByteArray> MetaDataMap;
    MetaDataMap map = imageMetaData();
    
    for (MetaDataMap::iterator it = map.begin(); it != map.end(); ++it)
    {
        QByteArray ba = it.data();
        
        switch (it.key())
        {
            case(DImg::JPG_COM):
            {
                jpeg_write_marker(&cinfo, M_COM, (const JOCTET*)ba.data(), ba.size());
                break;
            }
            case(DImg::JPG_EXIF):
            {
                jpeg_write_marker(&cinfo, M_EXIF, (const JOCTET*)ba.data(), ba.size());
                break;
            }
            case(DImg::JPG_IPTC):
            {
                jpeg_write_marker(&cinfo, M_IPTC, (const JOCTET*)ba.data(), ba.size());
                break;
            }
            default:
                break;
        }
    }

    // -------------------------------------------------------------------
    // Write ICC profil.
    
    QByteArray profile_rawdata = imageICCProfil();
    
    if (profile_rawdata.data() != 0)
    {
        write_icc_profile (&cinfo, (JOCTET *)profile_rawdata.data(), profile_rawdata.size());
    }    
        
    if (observer)
        observer->progressInfo(m_image, 0.2);
    
    // -------------------------------------------------------------------
    // Write Image data.
    
    uchar* line   = new uchar[w*3];
    uchar* dstPtr = 0;
    uint   checkPoint = 0;

    if (!imageSixteenBit())     // 8 bits image.
    {

        uchar* srcPtr = data;

        for (uint j=0; j<h; j++)
        {

            if (observer && j == checkPoint)
            {
                checkPoint += granularity(observer, h, 0.8);
                if (!observer->continueQuery(m_image))
                {
                    delete [] line;
                    jpeg_destroy_compress(&cinfo);
                    fclose(file);
                    return false;
                }
                // use 0-20% for pseudo-progress, now fill 20-100%
                observer->progressInfo(m_image, 0.2 + (0.8 * ( ((float)j)/((float)h) )));
            }

            dstPtr = line;
            
            for (uint i = 0; i < w; i++)
            {
                dstPtr[2] = srcPtr[0];
                dstPtr[1] = srcPtr[1];
                dstPtr[0] = srcPtr[2];
        
                srcPtr += 4;
                dstPtr += 3;
            }

            jpeg_write_scanlines(&cinfo, &line, 1);
        }
    }
    else
    {
        unsigned short* srcPtr = (unsigned short*)data;

        for (uint j=0; j<h; j++)
        {

            if (observer && j == checkPoint)
            {
                checkPoint += granularity(observer, h, 0.8);
                if (!observer->continueQuery(m_image))
                {
                    delete [] line;
                    jpeg_destroy_compress(&cinfo);
                    fclose(file);
                    return false;
                }
                // use 0-20% for pseudo-progress, now fill 20-100%
                observer->progressInfo(m_image, 0.2 + (0.8 * ( ((float)j)/((float)h) )));
            }

            dstPtr = line;
            
            for (uint i = 0; i < w; i++)
            {
                dstPtr[2] = (srcPtr[0] * 255UL)/65535UL;
                dstPtr[1] = (srcPtr[1] * 255UL)/65535UL;
                dstPtr[0] = (srcPtr[2] * 255UL)/65535UL;
        
                srcPtr += 4;
                dstPtr += 3;
            }
    
            jpeg_write_scanlines(&cinfo, &line, 1);
        }
    }

    delete [] line;

    // -------------------------------------------------------------------
    
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(file);

    return true;
}

}  // NameSpace Digikam

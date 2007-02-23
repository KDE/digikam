/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at gmail dot com>
 * Date   : 2004-09-29
 * Description : perform lossless rotation/flip to JPEG file
 * 
 * Copyright 2004-2005 by Renchi Raju
 * Copyright      2006 by Gilles Caulier
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

#define XMD_H

// C++ Includes.

#include <cstdio>
#include <cstdlib>

// C Ansi includes.

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include <setjmp.h>
#include <jpeglib.h>
}

// Qt includes.

#include <qcstring.h> 
#include <qfile.h>
#include <qfileinfo.h>
#include <qimage.h>

// KDE includes.

#include <kfilemetainfo.h>

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
#include "transupp.h"
#include "jpegutils.h"

namespace Digikam
{

// To manage Errors/Warnings handling provide by libjpeg

//#define ENABLE_DEBUG_MESSAGES

struct jpegutils_jpeg_error_mgr : public jpeg_error_mgr
{
    jmp_buf setjmp_buffer;
};

static void jpegutils_jpeg_error_exit(j_common_ptr cinfo);
static void jpegutils_jpeg_emit_message(j_common_ptr cinfo, int msg_level);
static void jpegutils_jpeg_output_message(j_common_ptr cinfo);

static void jpegutils_jpeg_error_exit(j_common_ptr cinfo)
{
    jpegutils_jpeg_error_mgr* myerr = (jpegutils_jpeg_error_mgr*) cinfo->err;

    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);

#ifdef ENABLE_DEBUG_MESSAGES
    DDebug() << k_funcinfo << buffer << endl;
#endif

    longjmp(myerr->setjmp_buffer, 1);
}

static void jpegutils_jpeg_emit_message(j_common_ptr cinfo, int msg_level)
{
    Q_UNUSED(msg_level)
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);

#ifdef ENABLE_DEBUG_MESSAGES
    DDebug() << k_funcinfo << buffer << " (" << msg_level << ")" << endl;
#endif
}

static void jpegutils_jpeg_output_message(j_common_ptr cinfo)
{
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);

#ifdef ENABLE_DEBUG_MESSAGES
    DDebug() << k_funcinfo << buffer << endl;
#endif
}

bool loadJPEGScaled(QImage& image, const QString& path, int maximumSize)
{
    QString format = QImageIO::imageFormat(path);
    if (format !="JPEG") return false;

    FILE* inputFile=fopen(QFile::encodeName(path), "rb");
    if(!inputFile)
        return false;

    struct jpeg_decompress_struct cinfo;
    struct jpegutils_jpeg_error_mgr       jerr;

    // JPEG error handling - thanks to Marcus Meissner
    cinfo.err                 = jpeg_std_error(&jerr);
    cinfo.err->error_exit     = jpegutils_jpeg_error_exit;
    cinfo.err->emit_message   = jpegutils_jpeg_emit_message;
    cinfo.err->output_message = jpegutils_jpeg_output_message;

    if (setjmp(jerr.setjmp_buffer)) 
    {
        jpeg_destroy_decompress(&cinfo);
        fclose(inputFile);
        return false;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, inputFile);
    jpeg_read_header(&cinfo, true);

    int imgSize = QMAX(cinfo.image_width, cinfo.image_height);

    // libjpeg supports 1/1, 1/2, 1/4, 1/8
    int scale=1;
    while(maximumSize*scale*2<=imgSize)
    {
        scale*=2;
    }
    if(scale>8) scale=8;

    cinfo.scale_num=1;
    cinfo.scale_denom=scale;

    switch (cinfo.jpeg_color_space)
    {
        case JCS_UNKNOWN:
            break;
        case JCS_GRAYSCALE:
        case JCS_RGB:
        case JCS_YCbCr:
            cinfo.out_color_space = JCS_RGB;
            break;
        case JCS_CMYK:
        case JCS_YCCK:
            cinfo.out_color_space = JCS_CMYK;
            break;
    }

    jpeg_start_decompress(&cinfo);

    QImage img;

    // We only take RGB with 1 or 3 components, or CMYK with 4 components
    if (!(
           (cinfo.out_color_space == JCS_RGB  && (cinfo.output_components == 3 || cinfo.output_components == 1))
        || (cinfo.out_color_space == JCS_CMYK &&  cinfo.output_components == 4)
        ))
    {
        jpeg_destroy_decompress(&cinfo);
        fclose(inputFile);
        return false;
    }

    switch(cinfo.output_components) 
    {
        case 3:
        case 4:
            img.create( cinfo.output_width, cinfo.output_height, 32 );
            break;
        case 1: // B&W image
            img.create( cinfo.output_width, cinfo.output_height, 8, 256 );
            for (int i = 0 ; i < 256 ; i++)
                img.setColor(i, qRgb(i, i, i));
            break;
    }

    uchar** lines = img.jumpTable();
    while (cinfo.output_scanline < cinfo.output_height)
        jpeg_read_scanlines(&cinfo, lines + cinfo.output_scanline, cinfo.output_height);

    jpeg_finish_decompress(&cinfo);

    // Expand 24->32 bpp
    if ( cinfo.output_components == 3 )
    {
        for (uint j=0; j<cinfo.output_height; j++) 
        {
            uchar *in = img.scanLine(j) + cinfo.output_width*3;
            QRgb *out = (QRgb*)( img.scanLine(j) );

            for (uint i=cinfo.output_width; i--; ) 
            {
                in -= 3;
                out[i] = qRgb(in[0], in[1], in[2]);
            }
        }
    }
    else if ( cinfo.output_components == 4 )
    {
        // CMYK conversion
        for (uint j=0; j<cinfo.output_height; j++)
        {
            uchar *in = img.scanLine(j) + cinfo.output_width*4;
            QRgb *out = (QRgb*)( img.scanLine(j) );

            for (uint i=cinfo.output_width; i--; )
            {
                in -= 4;
                int k = in[3];
                out[i] = qRgb(k * in[0] / 255, k * in[1] / 255, k * in[2] / 255);
            }
        }
    }

    int newMax = QMAX(cinfo.output_width, cinfo.output_height);
    int newx = maximumSize*cinfo.output_width / newMax;
    int newy = maximumSize*cinfo.output_height / newMax;

    jpeg_destroy_decompress(&cinfo);
    fclose(inputFile);

    image = img.smoothScale(newx,newy);

    return true;
}

bool exifRotate(const QString& file, const QString& documentName)
{
    QFileInfo fi(file);
    if (!fi.exists())
    {
        DDebug() << "ExifRotate: file do not exist: " << file << endl;
        return false;
    }
        
    if (isJpegImage(file))
    {
        DMetadata metaData;
        if (!metaData.load(file))
        {
            DDebug() << "ExifRotate: no Exif data found: " << file << endl;
            return true;
        }

        QString temp(fi.dirPath(true) + "/.digikam-exifrotate-");
        temp += QString::number(getpid());
        
        QCString in  = QFile::encodeName(file);
        QCString out = QFile::encodeName(temp);
        
        JCOPY_OPTION copyoption = JCOPYOPT_ALL;
        jpeg_transform_info transformoption;
    
        transformoption.force_grayscale = false;
        transformoption.trim            = false;
        transformoption.transform       = JXFORM_NONE;
            
        // we have the exif info. check the orientation
    
        switch(metaData.getImageOrientation())
        {
            case(DMetadata::ORIENTATION_UNSPECIFIED):
            case(DMetadata::ORIENTATION_NORMAL):
                break;
            case(DMetadata::ORIENTATION_HFLIP):
            {
                transformoption.transform = JXFORM_FLIP_H;
                break;
            }
            case(DMetadata::ORIENTATION_ROT_180):
            {
                transformoption.transform = JXFORM_ROT_180;
                break;
            }
            case(DMetadata::ORIENTATION_VFLIP):
            {
                transformoption.transform = JXFORM_FLIP_V;
                break;
            }
            case(DMetadata::ORIENTATION_ROT_90_HFLIP):
            {
                transformoption.transform = JXFORM_TRANSPOSE;
                break;
            }
            case(DMetadata::ORIENTATION_ROT_90):
            {
                transformoption.transform = JXFORM_ROT_90;
                break;
            }
            case(DMetadata::ORIENTATION_ROT_90_VFLIP):
            {
                transformoption.transform = JXFORM_TRANSVERSE;
                break;
            }
            case(DMetadata::ORIENTATION_ROT_270):
            {
                transformoption.transform = JXFORM_ROT_270;
                break;
            }
        }            
    
        if (transformoption.transform == JXFORM_NONE)
        {
            DDebug() << "ExifRotate: no rotation to perform: " << file << endl;
            return true;
        }
    
        struct jpeg_decompress_struct srcinfo;
        struct jpeg_compress_struct   dstinfo;
        struct jpegutils_jpeg_error_mgr jsrcerr, jdsterr;
        jvirt_barray_ptr* src_coef_arrays;
        jvirt_barray_ptr* dst_coef_arrays;
    
        // Initialize the JPEG decompression object with default error handling
        srcinfo.err                 = jpeg_std_error(&jsrcerr);
        srcinfo.err->error_exit     = jpegutils_jpeg_error_exit;
        srcinfo.err->emit_message   = jpegutils_jpeg_emit_message;
        srcinfo.err->output_message = jpegutils_jpeg_output_message;
    
        // Initialize the JPEG compression object with default error handling
        dstinfo.err                 = jpeg_std_error(&jdsterr);
        dstinfo.err->error_exit     = jpegutils_jpeg_error_exit;
        dstinfo.err->emit_message   = jpegutils_jpeg_emit_message;
        dstinfo.err->output_message = jpegutils_jpeg_output_message;
    
        FILE *input_file;
        FILE *output_file;
        
        input_file = fopen(in, "rb");
        if (!input_file)
        {
            DWarning() << "ExifRotate: Error in opening input file: " << input_file << endl;
            return false;
        }
    
        output_file = fopen(out, "wb");
        if (!output_file)
        {
            fclose(input_file);
            DWarning() << "ExifRotate: Error in opening output file: " << output_file  << endl;
            return false;
        }
    
        if (setjmp(jsrcerr.setjmp_buffer) || setjmp(jdsterr.setjmp_buffer))
        {
            jpeg_destroy_decompress(&srcinfo);
            jpeg_destroy_compress(&dstinfo);
            fclose(input_file);
            fclose(output_file);
            return false;
        }

        jpeg_create_decompress(&srcinfo);
        jpeg_create_compress(&dstinfo);
        
        jpeg_stdio_src(&srcinfo, input_file);
        jcopy_markers_setup(&srcinfo, copyoption);
        
        (void) jpeg_read_header(&srcinfo, true);
    
        jtransform_request_workspace(&srcinfo, &transformoption);
    
        // Read source file as DCT coefficients
        src_coef_arrays = jpeg_read_coefficients(&srcinfo);
    
        // Initialize destination compression parameters from source values
        jpeg_copy_critical_parameters(&srcinfo, &dstinfo);
    
        dst_coef_arrays = jtransform_adjust_parameters(&srcinfo, &dstinfo,
                                                        src_coef_arrays, &transformoption);
    
        // Specify data destination for compression
        jpeg_stdio_dest(&dstinfo, output_file);
    
        // Start compressor (note no image data is actually written here)
        jpeg_write_coefficients(&dstinfo, dst_coef_arrays);
        
        // Copy to the output file any extra markers that we want to preserve
        jcopy_markers_execute(&srcinfo, &dstinfo, copyoption);
    
        jtransform_execute_transformation(&srcinfo, &dstinfo,
                                            src_coef_arrays, &transformoption);
    
        // Finish compression and release memory
        jpeg_finish_compress(&dstinfo);
        jpeg_destroy_compress(&dstinfo);
        (void) jpeg_finish_decompress(&srcinfo);
        jpeg_destroy_decompress(&srcinfo);
    
        fclose(input_file);
        fclose(output_file);
    
        // -- Metadata operations ------------------------------------------------------

        // Reset the Exif orientation tag of the temp image to normal
        DDebug() << "ExifRotate: set Orientation tag to normal: " << file << endl;

        metaData.load(temp);
        metaData.setImageOrientation(DMetadata::ORIENTATION_NORMAL);
        QImage img(temp);
        
        // Get the new image dimension of the temp image. Using a dummy QImage objet here 
        // has a sense because the Exif dimension information can be missing from original image.
        // Get new dimensions with QImage will always work...
        metaData.setImageDimensions(img.size());

        // Update the image preview.
        QImage preview = img.scale(800, 600, QImage::ScaleMin);
        
        // TODO: see B.K.O #130525. The a JPEG segment is limited to 64K. If IPTC byte array 
        // bigger than 64K duing of image preview tag size, the target JPEG image will be 
        // broken. Note that IPTC image preview tag is limited to 256K!!! 
        // Temp. solution to disable IPTC preview record in JPEG file until a right solution 
        // will be found into Exiv2. 
        // metaData.setImagePreview(preview);

        // Update the image thumbnail.
        QImage thumb = preview.scale(160, 120, QImage::ScaleMin);
        metaData.setExifThumbnail(thumb);

        // Update Exif Document Name tag (the orinal file name from camera for example).
        metaData.setExifTagString("Exif.Image.DocumentName", documentName);

        // We update all new metadata now...
        metaData.applyChanges();
    
        // -----------------------------------------------------------------------------
        // set the file modification time of the temp file to that
        // of the original file
        struct stat st;
        stat(in, &st);
    
        struct utimbuf ut;
        ut.modtime = st.st_mtime;
        ut.actime  = st.st_atime;
        
        utime(out, &ut);
                
        // now overwrite the original file
        if (rename(out, in) == 0)
        {
            return true;
        }
        else
        {
            // moving failed. unlink the temp file
            unlink(out);
            return false;
        }
    }
    
    // Not a jpeg image.
    DDebug() << "ExifRotate: not a JPEG file: " << file << endl;
    return false;
}

bool jpegConvert(const QString& src, const QString& dest, const QString& documentName, const QString& format)
{
    QFileInfo fi(src);
    if (!fi.exists())
    {
        DDebug() << "JpegConvert: file do not exist: " << src << endl;
        return false;
    }
        
    if (isJpegImage(src))
    {
        DImg image(src);
    
        // Get image Exif/Iptc data.
        DMetadata meta;
        meta.setExif(image.getExif());
        meta.setIptc(image.getIptc());
    
        // Update Iptc preview.
        QImage preview = image.smoothScale(800, 600, QSize::ScaleMin).copyQImage();
    
        // TODO: see B.K.O #130525. a JPEG segment is limited to 64K. If the IPTC byte array is
        // bigger than 64K duing of image preview tag size, the target JPEG image will be
        // broken. Note that IPTC image preview tag is limited to 256K!!!
        // Temp. solution to disable IPTC preview record in JPEG file until a right solution 
        // will be found into Exiv2.
        // Note : There is no limitation with TIFF and PNG about IPTC byte array size.
    
        if (format.upper() != QString("JPG") && format.upper() != QString("JPEG") && 
            format.upper() != QString("JPE")) 
            meta.setImagePreview(preview);
    
        // Update Exif thumbnail.
        QImage thumb = preview.smoothScale(160, 120, QImage::ScaleMin);
        meta.setExifThumbnail(thumb);
    
        // Update Exif Document Name tag (the orinal file name from camera for example).
        meta.setExifTagString("Exif.Image.DocumentName", documentName);
    
        // Store new Exif/Iptc data into image.
        image.setExif(meta.getExif());
        image.setIptc(meta.getIptc());
    
        // And now save the picture to a new file format.

        if ( format.upper() == QString("PNG") ) 
            image.setAttribute("quality", 9);
    
        if ( format.upper() == QString("TIFF") || format.upper() == QString("TIF") ) 
            image.setAttribute("compress", true);

        return (image.save(dest, format));
    }
    
    return false;
}

bool isJpegImage(const QString& file)
{
    // Check if the file is an JPEG image
    KFileMetaInfo metaInfo(file, "image/jpeg", KFileMetaInfo::Fastest);

    if (metaInfo.isValid())
    {
        if (metaInfo.mimeType() == "image/jpeg")
            return true;
    }

    return false;
}

} // Namespace Digikam

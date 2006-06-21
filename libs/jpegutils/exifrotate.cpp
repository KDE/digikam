/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
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
#include <jpeglib.h>
}

// Qt includes.

#include <qstring.h>
#include <qcstring.h> 
#include <qfile.h>
#include <qfileinfo.h>
#include <qimage.h>

// KDE includes.

#include <kdebug.h>
#include <kfilemetainfo.h>

// Local includes.

#include "dmetadata.h"
#include "transupp.h"

namespace Digikam
{

bool exifRotate(const QString& file)
{
    QFileInfo fi(file);
    if (!fi.exists())
    {
        kdDebug() << "ExifRotate: file do not exist: " << file << endl;
        return false;
    }
        
    // Check if the file is an JPEG image
    KFileMetaInfo metaInfo(file, "image/jpeg", KFileMetaInfo::Fastest);

    if (metaInfo.isValid())
    {
        if (metaInfo.mimeType() == "image/jpeg" &&
            metaInfo.containsGroup("Jpeg EXIF Data"))
        {   
            QString temp(fi.dirPath(true) + "/.digikam-exifrotate-");
            temp += QString::number(getpid());
            
            QCString in  = QFile::encodeName(file);
            QCString out = QFile::encodeName(temp);
            
            DMetadata metaData;
            if (!metaData.load(file))
            {
                kdDebug() << "ExifRotate: no Exif data found: " << file << endl;
                return true;
            }
        
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
                kdDebug() << "ExifRotate: no rotation to perform: " << file << endl;
                return true;
            }
        
            struct jpeg_decompress_struct srcinfo;
            struct jpeg_compress_struct   dstinfo;
            struct jpeg_error_mgr jsrcerr, jdsterr;
            jvirt_barray_ptr* src_coef_arrays;
            jvirt_barray_ptr* dst_coef_arrays;
        
            // Initialize the JPEG decompression object with default error handling
            srcinfo.err = jpeg_std_error(&jsrcerr);
            jpeg_create_decompress(&srcinfo);
        
            // Initialize the JPEG compression object with default error handling
            dstinfo.err = jpeg_std_error(&jdsterr);
            jpeg_create_compress(&dstinfo);
        
            FILE *input_file;
            FILE *output_file;
            
            input_file = fopen(in, "rb");
            if (!input_file)
            {
                kdWarning() << "ExifRotate: Error in opening input file: " << input_file << endl;
                return false;
            }
        
            output_file = fopen(out, "wb");
            if (!output_file)
            {
                fclose(input_file);
                kdWarning() << "ExifRotate: Error in opening output file: " << output_file  << endl;
                return false;
            }
        
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
            kdDebug() << "ExifRotate: set Orientation tag to normal: " << file << endl;
            metaData.load(temp);
            metaData.setImageOrientation(DMetadata::ORIENTATION_NORMAL);
            QImage img(temp);
            
            // Get the new image dimension of the temp image. Using a dummy QImage objet here 
            // has a sense because the Exif dimension informations can be missing from original image.
            // Get new dimensions with QImage will always work...
            metaData.setImageDimensions(img.size());

            // Update the image preview.
            QImage preview = img.scale(800, 600, QImage::ScaleMin);
            metaData.setImagePreview(preview);

            // Update the image thumbnail.
            QImage thumb = preview.scale(160, 120, QImage::ScaleMin);
            metaData.setExifThumbnail(thumb);

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
    }
    
    // Not a jpeg image.
    kdDebug() << "ExifRotate: not a JPEG file: " << file << endl;
    return false;
}

} // Namespace Digikam

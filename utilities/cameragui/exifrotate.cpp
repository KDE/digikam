/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-29
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <qstring.h>
#include <qcstring.h> 
#include <qfile.h>
#include <qfileinfo.h>

#include <kdebug.h>

#include <libkexif/kexifdata.h>
#include <libkexif/kexifutils.h>

#define XMD_H

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jpeglib.h>
#include <utime.h>
}

#include "transupp.h"


namespace Digikam
{

bool exifRotate(const QString& file)
{
    QFileInfo fi(file);
    if (!fi.exists())
        return false;
    
    QString temp(fi.dirPath(true) + "/.digikam-exifrotate-");
    temp += QString::number(getpid());
    
    QCString in  = QFile::encodeName(file);
    QCString out = QFile::encodeName(temp);
    
    KExifData exifData;
    if (!exifData.readFromFile(file))
    {
        // no exif data. return.
        return true;
    }

    JCOPY_OPTION copyoption = JCOPYOPT_ALL;
    jpeg_transform_info transformoption;

    transformoption.force_grayscale = false;
    transformoption.trim            = false;
    transformoption.transform       = JXFORM_NONE;
        
    // we have the exif info. check the orientation

    switch(exifData.getImageOrientation())
    {
    case(KExifData::UNSPECIFIED):
    case(KExifData::NORMAL):
        break;
    case(KExifData::HFLIP):
    {
        transformoption.transform = JXFORM_FLIP_H;
        break;
    }
    case(KExifData::ROT_180):
    {
        transformoption.transform = JXFORM_ROT_180;
        break;
    }
    case(KExifData::VFLIP):
    {
        transformoption.transform = JXFORM_FLIP_V;
        break;
    }
    case(KExifData::ROT_90_HFLIP):
    {
        transformoption.transform = JXFORM_TRANSPOSE;
        break;
    }
    case(KExifData::ROT_90):
    {
        transformoption.transform = JXFORM_ROT_90;
        break;
    }
    case(KExifData::ROT_90_VFLIP):
    {
        transformoption.transform = JXFORM_TRANSVERSE;
        break;
    }
    case(KExifData::ROT_270):
    {
        transformoption.transform = JXFORM_ROT_270;
        break;
    }
    }            

    if (transformoption.transform == JXFORM_NONE)
    {
        // nothing to do
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
        kdWarning() << "exifRotate: Error in opening input file" << endl;
        return false;
    }

    output_file = fopen(out, "wb");
    if (!output_file)
    {
        fclose(input_file);
        kdWarning() << "exifRotate: Error in opening output file" << endl;
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

    dst_coef_arrays = jtransform_adjust_parameters(&srcinfo,
                                                   &dstinfo,
                                                   src_coef_arrays,
                                                   &transformoption);

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

    // reset the orientation of the temp file to normal
    KExifUtils::writeOrientation(temp, KExifData::NORMAL);

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

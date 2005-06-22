/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-06-13
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju

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

#include <qfile.h>
#include <libkexif/kexifdata.h>

#include "jpegreader.h"

#define XMD_H 1
extern "C" {
#include <stdio.h>
#include <jpeglib.h>
}         

#define M_COM  0xFE
#define M_EXIF 0xE1

void readJPEGMetaData(const QString& filePath,
                      QString& comments,
                      QDateTime& datetime)
{
    comments = QString();
    datetime = QDateTime();

    struct jpeg_decompress_struct srcinfo;
    struct jpeg_error_mgr jsrcerr;

    srcinfo.err = jpeg_std_error(&jsrcerr);
    jpeg_create_decompress(&srcinfo);

    FILE *input_file;

    input_file = fopen(QFile::encodeName(filePath), "rb");
    if (!input_file)
        return;

    unsigned short header;
    
    if (fread(&header, 2, 1, input_file) != 1)
    {
        fclose(input_file);
        return;
    }

    if (header != 0xd8ff)
    {
        // not a jpeg file
        fclose(input_file);
        return;
    }
    
    fseek(input_file, 0L, SEEK_SET);

    jpeg_stdio_src(&srcinfo, input_file);

    jpeg_save_markers(&srcinfo, M_COM, 0xFFFF);
    jpeg_save_markers(&srcinfo, M_EXIF, 0xFFFF);

    (void) jpeg_read_header(&srcinfo, true);

    jpeg_saved_marker_ptr marker = srcinfo.marker_list;
    while (marker)
    {
        if (marker->marker == M_COM)
        {
            if (!marker->data || !marker->data_length)
                continue;

            comments = QString::fromAscii((const char*)marker->data,
                                          marker->data_length);
        }
        else if (marker->marker == M_EXIF)
        {
            KExifData exifData;
            if (!exifData.readFromData((char*)marker->data,
                                       marker->data_length))
                continue;

            datetime = exifData.getExifDateTime();
        }

        marker = marker->next;
    }
        
    jpeg_destroy_decompress(&srcinfo);

    fclose(input_file);                 
}


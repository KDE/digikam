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
#include <kdebug.h>
#include <libkexif/kexifdata.h>

#include "jpegmetadata.h"

#define XMD_H 1
extern "C" {
#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>
}         

struct readJPEGMetaData_error_mgr : public jpeg_error_mgr
{
    jmp_buf setjmp_buffer;
};

extern "C"
{
    static void readJPEGMetaData_error_exit(j_common_ptr cinfo)
    {
        readJPEGMetaData_error_mgr* myerr =
            (readJPEGMetaData_error_mgr*) cinfo->err;

        char buffer[JMSG_LENGTH_MAX];
        (*cinfo->err->format_message)(cinfo, buffer);
        kdWarning() << buffer << endl;
        longjmp(myerr->setjmp_buffer, 1);
    }
}

namespace Digikam
{

#define M_COM  0xFE
#define M_EXIF 0xE1

void readJPEGMetaData(const QString& filePath,
                      QString& comments,
                      QDateTime& datetime)
{
    comments = QString();
    datetime = QDateTime();

    FILE *input_file = fopen(QFile::encodeName(filePath), "rb");
    if (!input_file)
        return;
    
    struct jpeg_decompress_struct     srcinfo;
    struct readJPEGMetaData_error_mgr jerr;

    srcinfo.err             = jpeg_std_error(&jerr);
    srcinfo.err->error_exit = readJPEGMetaData_error_exit;

    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&srcinfo);
        fclose(input_file);
        return;
    }
    
    jpeg_create_decompress(&srcinfo);


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
            {
                marker=marker->next;
                continue;
            }

            comments = QString::fromAscii((const char*)marker->data,
                                          marker->data_length);
        }
        else if (marker->marker == M_EXIF)
        {
            KExifData exifData;
            if (!exifData.readFromData((char*)marker->data,
                                       marker->data_length))
            {
                marker=marker->next;
                continue;
            }

            datetime = exifData.getExifDateTime();
        }

        marker = marker->next;
    }

    jpeg_destroy_decompress(&srcinfo);

    fclose(input_file);
}

}

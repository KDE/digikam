/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-23
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju
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

#ifndef EXIFRESTORER_H
#define EXIFRESTORER_H

// Qt includes.

#include <qptrlist.h>
#include <qstring.h>

// Local includes.

#include "jpegsection.h"

//-- Jpeg Image Maker Types -----------------------------------------

#define M_SOF0  0xC0       // Start Of Frame N
#define M_SOF1  0xC1       // N indicates which compression process
#define M_SOF2  0xC2       // Only SOF0-SOF2 are now in common use
#define M_SOF3  0xC3
#define M_SOF5  0xC5       // NB: codes C4 and CC are NOT SOF markers
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_SOF9  0xC9
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_SOI   0xD8       // Start Of Image (beginning of datastream)
#define M_EOI   0xD9       // End Of Image (end of datastream)
#define M_SOS   0xDA       // Start Of Scan (begins compressed data)
#define M_JFIF  0xE0       // Jfif marker
#define M_EXIF  0xE1       // Exif marker
#define M_COM   0xFE       // COMment

//------------------------------------------------------------------


class ExifRestorer {

public:

    enum ReadMode {
        ExifOnly=0,
        EntireImage
    };

    ExifRestorer();
    ~ExifRestorer();

    void clear();
    int  readFile(const QString& filename, ReadMode mode);
    int  writeFile(const QString& filename);
    void insertExifData(JpegSection *exifData);

    bool hasExif () {
        return hasExif_;
    }

    JpegSection* exifData() {
        return exifData_;
    }

private:

    QPtrList<JpegSection> jpegSections_;
    JpegSection *exifData_;
    JpegSection *imageData_;
    bool        hasExif_;

};

#endif  // EXIFRESTORER_H

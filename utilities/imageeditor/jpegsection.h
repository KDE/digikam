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
 
#ifndef JPEGSECTION_H
#define JPEGSECTION_H

class JpegSection {

    friend class ExifRestorer;

public:

    JpegSection() {
        data = 0;
        size = 0;
    }

    ~JpegSection() {
        if (data)
            delete [] data;
    }

private:

    unsigned char* data;
    unsigned int   size;
    unsigned char  type;

};


#endif  // JPEGSECTION_H

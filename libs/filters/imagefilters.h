/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-24-01
 * Description : image filters. 
 * 
 * Copyright 2004-2005 by Gilles Caulier
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

#ifndef IMAGE_FILTERS_H
#define IMAGE_FILTERS_H

namespace Digikam
{

class ImageFilters
{

private:

struct double_packet
    {
    double red;
    double green;
    double blue;
    double alpha;
    };

struct short_packet
    {
    unsigned short int red;
    unsigned short int green;
    unsigned short int blue;
    unsigned short int alpha;
    };

struct NormalizeParam 
    {
    uchar  lut[256];
    double min;
    double max;
    };
    
public:

    static void equalizeImage(uint *data, int w, int h);
    static void stretchContrastImage(uint *data, int w, int h);
    static void normalizeImage(uint *data, int w, int h);
    static void autoLevelsCorrectionImage(uint *data, int w, int h);
    static void invertImage(uint *data, int w, int h);
    static void antiAliasImage(uint *data, int Width, int Height, int Sensibility=128);
};

}  // NameSpace Digikam

#endif /* IMAGE_FILTERS_H */

/* ============================================================
 * File  : imageeffect_colorsenhance.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-20
 * Description : image contrast enhancement techniques. 
 * 
 * Copyright 2004 by Gilles Caulier
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

#ifndef IMAGEEFFECT_COLORSENHANCE_H
#define IMAGEEFFECT_COLORSENHANCE_H

class ImageEffect_ColorsEnhance
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

public:

    static void equalizeImage();
    static void normalizeImage();
    static void autoLevelsCorrectionImage();
    static void testKImageEffect();

};

#endif /* IMAGEEFFECT_COLORSENHANCE_H */

/* ============================================================
 * File  : hslmodifier.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-03-06
 * Description : Hue/Saturation/Lightness modifier methods
 *               for DImg framework
 * 
 * Copyright 2005 by Gilles Caulier
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
 * ============================================================ */

#ifndef HSLMODIFIER_H
#define HSLMODIFIER_H

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT HSLModifier
{
public:

    HSLModifier();
    ~HSLModifier();

    void   reset();
    bool   modified() const;
    
    void   setHue(double val);
    void   setSaturation(double val);
    void   setLightness(double val);
    void   applyHSL(DImg& image);
    
private:

    bool           m_modified;
    
    // Used with HSL correction methods.
    
    int htransfer[256];
    int ltransfer[256];
    int stransfer[256];
    
    int htransfer16[65536];
    int ltransfer16[65536];
    int stransfer16[65536];
    
private:

    int  hsl_value (double n1, double n2, double hue);

    void rgb_to_hsl (int& r, int& g, int& b);
    void rgb_to_hsl16 (int& r, int& g, int& b);
    
    void hsl_to_rgb (int& hue, int& saturation, int& lightness);
    void hsl_to_rgb16 (int& hue, int& saturation, int& lightness);

};

}  // NameSpace Digikam
    
#endif /* HSLMODIFIER_H */

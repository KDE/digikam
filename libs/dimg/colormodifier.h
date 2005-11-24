/* ============================================================
 * File  : colormodifier.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2005-03-06
 * Description : a color modifier methods for DImg framework
 * 
 * Copyright 2005 by Renchi Raju, Gilles Caulier
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

#ifndef COLORMODIFIER_H
#define COLORMODIFIER_H

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT ColorModifier
{
public:

    ColorModifier();
    ~ColorModifier();

    void   reset();
    bool   modified() const;
    
    void   setGamma(double val);
    void   setBrightness(double val);
    void   setContrast(double val);
    void   applyBCG(DImg& image);
    
    void   setHue(double val);
    void   setSaturation(double val);
    void   setLightness(double val);
    void   applyHSL(DImg& image);
    
private:

    bool           m_modified;
    
    // Used with BCG correction methods.
    unsigned short m_map16[65536];
    unsigned char  m_map[256];
    
    // Used with HSL correction methods.
    
    int htransfer[256];
    int ltransfer[256];
    int stransfer[256];
    
    int htransfer16[65536];
    int ltransfer16[65536];
    int stransfer16[65536];
    
private:

    int  hsl_value (double n1, double n2, double hue);
    int  hsl_value16 (double n1, double n2, double hue);
    
    void rgb_to_hsl (int& r, int& g, int& b);
    
    void rgb_to_hsl16 (int& r, int& g, int& b);
    
    void hsl_to_rgb (int& hue, int& saturation, int& lightness);
    
    void hsl_to_rgb16 (int& hue, int& saturation, int& lightness);
    int  rgb_to_l (int red, int green, int blue);

};

}  // NameSpace Digikam
    
#endif /* COLORMODIFIER_H */

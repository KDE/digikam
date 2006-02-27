/* ============================================================
 * File  : hslmodifier.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-03-06
 * Description : Hue/Saturation/Lightness modifier methods
 *               for DImg framework
 * 
 * Copyright 2005-2006 by Gilles Caulier
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
class HSLModifierPriv;

class DIGIKAM_EXPORT HSLModifier
{
public:

    HSLModifier();
    ~HSLModifier();

    void reset();
    bool modified() const;

    void setOverIndicator(bool overIndicator);
    void setHue(double val);
    void setSaturation(double val);
    void setLightness(double val);
    void applyHSL(DImg& image);
    
private:

    HSLModifierPriv* d;    
};

}  // NameSpace Digikam
    
#endif /* HSLMODIFIER_H */

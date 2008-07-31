/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-06
 * Description : Hue/Saturation/Lightness image filter.
 * 
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

    void setHue(double val);
    void setSaturation(double val);
    void setLightness(double val);
    void applyHSL(DImg& image);
    
private:

    HSLModifierPriv* d;    
};

}  // NameSpace Digikam
    
#endif /* HSLMODIFIER_H */

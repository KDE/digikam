/* ============================================================
 * File  : bcgmodifier.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date  : 2005-03-06
 * Description : Brighness/Contrast/Gamma modifier methods
 *               for DImg framework
 * 
 * Copyright 2005 by Renchi Raju, Gilles Caulier
 * Copyright 2006 Gilles Caulier
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

#ifndef BCGMODIFIER_H
#define BCGMODIFIER_H

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DImg;
class BCGModifierPriv;

class DIGIKAM_EXPORT BCGModifier
{
public:

    BCGModifier();
    ~BCGModifier();

    void reset();
    bool modified() const;

    void setOverIndicator(bool overIndicator);
    void setGamma(double val);
    void setBrightness(double val);
    void setContrast(double val);
    void applyBCG(DImg& image);
    
private:

    BCGModifierPriv* d;
    
};

}  // NameSpace Digikam
    
#endif /* BCGMODIFIER_H */

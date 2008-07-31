/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-06
 * Description : a Brightness/Contrast/Gamma image filter.
 * 
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com> 
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

    enum CHANNEL
    {
        CHANNEL_ALL=0,
        CHANNEL_RED,
        CHANNEL_GREEN,
        CHANNEL_BLUE
    };

public:

    BCGModifier();
    ~BCGModifier();

    void reset();
    bool modified() const;

    void setChannel(int channel);
    void setGamma(double val);
    void setBrightness(double val);
    void setContrast(double val);
    void applyBCG(DImg& image);
    void applyBCG(uchar *bits, uint width, uint height, bool sixteenBits);

private:

    BCGModifierPriv* d;
};

}  // NameSpace Digikam

#endif /* BCGMODIFIER_H */
